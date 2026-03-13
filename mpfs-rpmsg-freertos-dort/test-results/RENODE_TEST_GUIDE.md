# RRDU SIL Quick Guide (Renode)

## 1) Goal
Prove both paths without hardware:
- Success: bringup reaches DONE and RRDU reaches READY
- Fault: bringup fails when RRDU measurement fails

## 2) Start Renode
```powershell
subst X: "C:\Microchip\SoftConsole-v2022.2-RISC-V-747\extras\workspace.examples\mpfs-rpmsg-freertos - Copy"
& "C:\Program Files\Renode\bin\Renode.exe" --console --disable-gui --plain
```

## 3) Success Run
```text
mach create
machine LoadPlatformDescription @platforms/boards/mpfs-icicle-kit.repl
sysbus LoadELF "X:\test-results\mpfs-rpmsg-master-success.elf"
emulation RunFor "00:00:00.050"
```

Read summary:
```text
sysbus ReadDoubleWord 0x91809ea4   # bringup_result
sysbus ReadDoubleWord 0x91809ea8   # bringup_state
sysbus ReadDoubleWord 0x91809eac   # trace_len
sysbus ReadDoubleWord 0x91809ed8   # rrdu_state
sysbus ReadDoubleWord 0x91809ef0   # rrdu_fault
```

Expected (success):
- bringup_result = `0x00000001` (PASS)
- bringup_state  = `0x00000002` (BU_DONE)
- rrdu_state     = `0x00000006` (RRDU_READY)
- rrdu_fault     = `0x00000000` (NO_FAULT)

## 4) Fault Run
```text
mach create
machine LoadPlatformDescription @platforms/boards/mpfs-icicle-kit.repl
sysbus LoadELF "X:\test-results\mpfs-rpmsg-master-fault.elf"
emulation RunFor "00:00:00.050"
```

Read same summary addresses.

Expected (fault):
- bringup_result = `0xFFFFFFFF` (FAIL)
- bringup_state  = `0x00000003` (BU_FAILED)
- rrdu_state     = `0x00000007` (RRDU_FAILED)
- rrdu_fault     = `0x00000001` (FAULT)

## 5) Transition Trace (optional, evidence)
Trace base: `0x91809fd0`

Read first entries:
```text
sysbus ReadDoubleWord 0x91809fd0
sysbus ReadDoubleWord 0x91809fd4
sysbus ReadDoubleWord 0x91809fd8
sysbus ReadDoubleWord 0x91809fdc
```

Decode:
- `0x100` = bringup start
- `0x200 + unit_id` = unit ready
- `0x300 + unit_id` = unit failed
- `0x400` = RRDU measurement done
- `0x401` = RRDU measurement failed
- `0x500` = bringup done
- `0x501` = bringup failed

Typical sequences:
- Success: `0x100, 0x200, 0x400, 0x201, ... , 0x20A, 0x500`
- Fault:   `0x100, 0x200, 0x401, 0x301, 0x501`

## 6) Warnings
Renode warnings like `mmuart1 unsupported...` / `GPIO not registered` are expected here.
Pass/fail is decided by summary values above.

