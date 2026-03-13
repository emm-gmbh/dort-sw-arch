param(
    [string]$RenodeExe = "C:\Program Files\Renode\bin\Renode.exe"
)

$ErrorActionPreference = "Stop"

$workspace = Split-Path -Parent $MyInvocation.MyCommand.Path
$elfSource = Join-Path $workspace "Master-Default\mpfs-rpmsg-master.elf"

if(!(Test-Path $RenodeExe)) {
    throw "Renode executable not found: $RenodeExe"
}

if(!(Test-Path $elfSource)) {
    throw "ELF not found: $elfSource"
}

$drive = "X:"
$elfAlias = "X:\Master-Default\app.elf"
$rescPath = "X:\renode-smoke.resc"

try {
    cmd /c ("subst {0} ""{1}""" -f $drive, $workspace)
    Copy-Item -Force "X:\Master-Default\mpfs-rpmsg-master.elf" $elfAlias

    @'
using sysbus
mach create
machine LoadPlatformDescription @platforms/boards/mpfs-icicle-kit.repl
sysbus LoadELF "X:\Master-Default\app.elf"
'@ | Set-Content $rescPath

    & $RenodeExe --console --disable-gui --plain $rescPath -e 'start; e51 PC; e51 IsHalted; u54_4 PC; u54_4 IsHalted; quit'
}
finally {
    cmd /c "subst $drive /D" | Out-Null
}
