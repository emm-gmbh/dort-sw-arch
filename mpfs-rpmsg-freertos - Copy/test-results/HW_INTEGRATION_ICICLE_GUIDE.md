# RRDU HW Integration Guide (Icicle Kit)

## 1) What Is Already Integrated
The project now supports two RRDU HAL backends:

- Fake backend (default):
  - `src/application/src_user/drivers/rrdu/rrdu_hal_fake.c`
- MMIO backend (new):
  - `src/application/src_user/drivers/rrdu/rrdu_hal_mmio.c`

Backend selection is done in:
- `src/application/Makefile`

Rule:
- if `-DRRDU_HAL_BACKEND_MMIO=1` is present in `EXT_CFLAGS`, MMIO backend is compiled
- otherwise fake backend is compiled

## 2) Build Commands
From project root:

MMIO (hardware path):
```powershell
make MASTER=1 all EXT_CFLAGS='-DRRDU_HAL_BACKEND_MMIO=1 -DRRDU_INJECT_FAULT=0'
```

Fake (existing path):
```powershell
make MASTER=1 all EXT_CFLAGS='-DRRDU_INJECT_FAULT=0'
```

Output artifact:
- `Master-Default/mpfs-rpmsg-master.elf`

## 3) MMIO Backend Behavior
The MMIO backend uses colleague RRDU flow and design document mapping:

- Base address: `0x65000000`
- Offsets:
  - `0x0`  `meas_coarse`
  - `0x4`  `ref_coarse`
  - `0x8`  `meas_fine`
  - `0xC`  `ref_fine`

GPIO2 usage:
- `GPIO2_0`: start pulse
- `GPIO2_1`: reset pulse
- `GPIO2_2`: fine done input
- `GPIO2_3`: ready input
- `GPIO2_4`: coarse done input

Current implementation runs measurement in `rrdu_hal_meas_start_async()` and then calls callback, so existing RRDU service state machine continues to work unchanged.

## 4) First Hardware Bring-Up Sequence
1. Program/load your firmware to Icicle.
2. Open UART terminal for board output.
3. Trigger normal bringup flow.
4. Check RRDU transitions:
   - health check passes
   - measurement done event arrives
   - RRDU reaches READY
5. If failing, inspect RRDU fault/status values first.

## 5) Common Early Failure Causes
- FPGA image does not match expected RRDU map (`0x65000000` not valid)
- GPIO2 pin routing does not match expected done/ready/start/reset lines
- external RRDU signal chain not connected or not stable
- build accidentally used fake backend instead of MMIO

## 6) Quick Verification Hints
- Confirm build used MMIO by checking linker/compile line includes `rrdu_hal_mmio.o`
- Confirm no fake path was linked in MMIO build
- Keep fake backend available for fallback debugging

## 7) Current Scope Status
- Software integration: done
- MMIO backend compile path: done
- Real hardware execution on Icicle: pending (requires board access)
