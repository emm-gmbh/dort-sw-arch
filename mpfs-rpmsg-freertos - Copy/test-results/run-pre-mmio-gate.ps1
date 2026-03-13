param(
    [string]$RenodeExe = "C:\Program Files\Renode\bin\Renode.exe",
    [int]$Iterations = 10,
    [string]$RunFor = "00:00:00.050",
    [string]$DriveAlias = "X:"
)

$ErrorActionPreference = "Stop"

if ($Iterations -lt 1) {
    throw "Iterations must be >= 1"
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$workspace = Split-Path -Parent $scriptDir

$addrBringupResult = "0x91809ea4"
$addrBringupState  = "0x91809ea8"
$addrTraceLen      = "0x91809eac"
$addrRrduState     = "0x91809ed8"
$addrRrduFault     = "0x91809ef0"
$traceBase = 0x91809fd0
$traceReadCount = 16

$successElfReal = Join-Path $workspace "test-results\mpfs-rpmsg-master-success.elf"
$faultElfReal   = Join-Path $workspace "test-results\mpfs-rpmsg-master-fault.elf"

function Ensure-Exists([string]$path) {
    if (!(Test-Path $path)) {
        throw "Missing required file: $path"
    }
}

function HexToUInt32([string]$hexWord) {
    return [uint32]([convert]::ToUInt32($hexWord.Substring(2), 16))
}

function To-RenodePath([string]$path) {
    return ($path -replace "\\", "/")
}

function New-Resc([string]$elfPathForRenode, [string]$rescPathReal) {
    $lines = @(
        "using sysbus",
        "mach create",
        "machine LoadPlatformDescription @platforms/boards/mpfs-icicle-kit.repl",
        "sysbus LoadELF `"$elfPathForRenode`"",
        "emulation RunFor `"$RunFor`"",
        "sysbus ReadDoubleWord $addrBringupResult",
        "sysbus ReadDoubleWord $addrBringupState",
        "sysbus ReadDoubleWord $addrTraceLen",
        "sysbus ReadDoubleWord $addrRrduState",
        "sysbus ReadDoubleWord $addrRrduFault"
    )

    for ($i = 0; $i -lt $traceReadCount; $i++) {
        $addr = $traceBase + (4 * $i)
        $lines += ("sysbus ReadDoubleWord 0x{0:x8}" -f $addr)
    }

    $lines += "quit"
    Set-Content -Path $rescPathReal -Value ($lines -join "`r`n")
}

function Invoke-RenodeRaw([string]$rescPathForRenode) {
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $RenodeExe
    $psi.Arguments = "--console --disable-gui --plain `"$rescPathForRenode`""
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true

    $p = New-Object System.Diagnostics.Process
    $p.StartInfo = $psi
    [void]$p.Start()

    $stdout = $p.StandardOutput.ReadToEnd()
    $stderr = $p.StandardError.ReadToEnd()
    $p.WaitForExit()

    return [pscustomobject]@{
        ExitCode = $p.ExitCode
        Combined = ($stdout + "`n" + $stderr)
    }
}

function Invoke-RenodeAndParse([string]$rescPathForRenode) {
    $run = Invoke-RenodeRaw -rescPathForRenode $rescPathForRenode
    if ($run.ExitCode -ne 0) {
        throw "Renode exited with code $($run.ExitCode)`n$($run.Combined)"
    }

    $hexLines = @(
        ($run.Combined -split "`r?`n") |
        Where-Object { $_ -match '^\s*0x[0-9a-fA-F]{8}\s*$' } |
        ForEach-Object { $_.Trim() }
    )

    $expectedWords = 5 + $traceReadCount
    if ($hexLines.Count -lt $expectedWords) {
        throw "Renode output parse failure: expected at least $expectedWords values, got $($hexLines.Count)."
    }

    $vals = $hexLines[0..($expectedWords - 1)] | ForEach-Object { HexToUInt32 $_ }

    return [pscustomobject]@{
        BringupResult = $vals[0]
        BringupState  = $vals[1]
        TraceLen      = $vals[2]
        RrduState     = $vals[3]
        RrduFault     = $vals[4]
        TraceWords    = @($vals[5..($vals.Count - 1)])
    }
}

function Get-TraceEvents($traceWords, [uint32]$traceLen) {
    $count = [Math]::Min($traceWords.Count, $traceLen)
    if ($count -le 0) { return @() }
    return @($traceWords[0..($count - 1)])
}

function Test-Scenario([string]$name, [string]$elfPathForRenode, [string]$rescPathForRenode, [string]$rescPathReal, [hashtable]$expect) {
    New-Resc -elfPathForRenode $elfPathForRenode -rescPathReal $rescPathReal

    $passes = 0
    $fails = 0
    $details = New-Object System.Collections.Generic.List[string]

    for ($i = 1; $i -le $Iterations; $i++) {
        $r = Invoke-RenodeAndParse -rescPathForRenode $rescPathForRenode
        $trace = Get-TraceEvents -traceWords $r.TraceWords -traceLen $r.TraceLen

        $ok = $true
        if ($r.BringupResult -ne [uint32]$expect.BringupResult) { $ok = $false }
        if ($r.BringupState  -ne [uint32]$expect.BringupState)  { $ok = $false }
        if ($r.RrduState     -ne [uint32]$expect.RrduState)     { $ok = $false }
        if ($r.RrduFault     -ne [uint32]$expect.RrduFault)     { $ok = $false }
        if ($trace.Count -eq 0 -or $trace[0] -ne 0x100) { $ok = $false }
        if ($trace -notcontains [uint32]$expect.MustContain) { $ok = $false }
        if ($trace.Count -eq 0 -or $trace[$trace.Count - 1] -ne [uint32]$expect.TerminalEvent) { $ok = $false }

        if ($ok) {
            $passes++
        }
        else {
            $fails++
            $details.Add(
                ("iter={0} result=0x{1:x8} state=0x{2:x8} rrdu_state=0x{3:x8} rrdu_fault=0x{4:x8} trace={5}" -f
                    $i, $r.BringupResult, $r.BringupState, $r.RrduState, $r.RrduFault,
                    (($trace | ForEach-Object { "0x{0:x3}" -f $_ }) -join ","))
            ) | Out-Null
        }
    }

    return [pscustomobject]@{
        Scenario = $name
        Passes = $passes
        Fails = $fails
        Details = $details
    }
}

function Test-StaleAddresses {
    $oldAddrs = @("0x91809de4", "0x91809de8", "0x91809dec", "0x91809f1c", "0x91809f20")
    $rescFiles = Get-ChildItem -Path $workspace -Filter *.resc -File
    $hits = New-Object System.Collections.Generic.List[string]
    foreach ($f in $rescFiles) {
        $content = Get-Content -Path $f.FullName -Raw
        foreach ($addr in $oldAddrs) {
            if ($content -match [regex]::Escape($addr)) {
                $hits.Add("$($f.Name):$addr") | Out-Null
            }
        }
    }
    return $hits
}

Ensure-Exists $RenodeExe
Ensure-Exists $successElfReal
Ensure-Exists $faultElfReal

$successExpect = @{
    BringupResult = 0x00000001
    BringupState = 0x00000002
    RrduState = 0x00000006
    RrduFault = 0x00000000
    MustContain = 0x400
    TerminalEvent = 0x500
}

$faultExpect = @{
    BringupResult = 4294967295
    BringupState = 0x00000003
    RrduState = 0x00000007
    RrduFault = 0x00000001
    MustContain = 0x401
    TerminalEvent = 0x501
}

$staleHits = Test-StaleAddresses
$allOk = $true

$null = cmd /c "subst $DriveAlias /D >nul 2>&1"
$quotedWorkspace = '"' + $workspace + '"'
$null = cmd /c ("subst $DriveAlias $quotedWorkspace >nul 2>&1")
if (!(Test-Path "$DriveAlias\")) {
    throw "Failed to map $DriveAlias to workspace path"
}

try {
    $successElfRenode = To-RenodePath (Join-Path "$DriveAlias\" "test-results\mpfs-rpmsg-master-success.elf")
    $faultElfRenode   = To-RenodePath (Join-Path "$DriveAlias\" "test-results\mpfs-rpmsg-master-fault.elf")

    $successRescReal = Join-Path $workspace "test-results\gate-success.resc"
    $faultRescReal   = Join-Path $workspace "test-results\gate-fault.resc"

    $successRescRenode = To-RenodePath (Join-Path "$DriveAlias\" "test-results\gate-success.resc")
    $faultRescRenode   = To-RenodePath (Join-Path "$DriveAlias\" "test-results\gate-fault.resc")

    $success = Test-Scenario -name "success" -elfPathForRenode $successElfRenode -rescPathForRenode $successRescRenode -rescPathReal $successRescReal -expect $successExpect
    $fault = Test-Scenario -name "fault" -elfPathForRenode $faultElfRenode -rescPathForRenode $faultRescRenode -rescPathReal $faultRescReal -expect $faultExpect

    Write-Host "==== PRE-MMIO GATE (FAKE HAL SIL) ===="
    Write-Host ("Iterations per scenario: {0}" -f $Iterations)
    Write-Host ""

    foreach ($r in @($success, $fault)) {
        $status = if ($r.Fails -eq 0) { "PASS" } else { "FAIL" }
        Write-Host ("{0}: {1} (pass={2}, fail={3})" -f $r.Scenario.ToUpperInvariant(), $status, $r.Passes, $r.Fails)
        if ($r.Fails -gt 0) {
            $allOk = $false
            $r.Details | ForEach-Object { Write-Host ("  " + $_) }
        }
    }

    Write-Host ""
    if ($staleHits.Count -gt 0) {
        Write-Host "Address script hygiene: FAIL (stale addresses detected)"
        $staleHits | ForEach-Object { Write-Host ("  " + $_) }
        $allOk = $false
    } else {
        Write-Host "Address script hygiene: PASS"
    }

    Write-Host ""
    if ($allOk) {
        Write-Host "OVERALL: PASS"
        exit 0
    } else {
        Write-Host "OVERALL: FAIL"
        exit 1
    }
}
finally {
    $null = cmd /c "subst $DriveAlias /D"
}


