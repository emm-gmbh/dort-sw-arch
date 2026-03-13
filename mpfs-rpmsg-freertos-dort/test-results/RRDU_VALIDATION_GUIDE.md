# RRDU SIL Quick Guide (Renode)

## 1) Goal
Validate RRDU behavior only, without relying on the full bringup result:
- Success: RRDU reaches READY
- Fault: RRDU reaches FAILED

This guide is for the standalone RRDU validation flow in:
- `src/application/src_user/core/rrdu_validation_demo.c`

It is separate from the full bringup guide in:
- `test-results/RENODE_TEST_GUIDE.md`

## 2) Validation Outputs
Standalone RRDU validation exposes:
- `g_rrdu_validation_result`
- `g_rrdu_validation_reason`

Meaning:
- `1` = RRDU validation PASS
- `-1` = RRDU validation FAIL
- `-2` = timeout / no terminal result

## 3) Expected RRDU Flow
Success path:
1. RRDU config succeeds
2. RRDU health check passes
3. Async measurement completes with `RRDU_HAL_OK`
4. RRDU posts `EVT_UNIT_READY`
5. `g_rrdu_validation_result = 1`

Fault path:
1. RRDU config succeeds
2. RRDU health check passes
3. Async measurement completes with `RRDU_HAL_ERR`
4. RRDU posts `EVT_UNIT_FAILED`
5. `g_rrdu_validation_result = -1`

## 4) Build Assumption
To use this guide, the image must be built so that the standalone RRDU validation demo runs and exports:
- `g_rrdu_validation_result`
- `g_rrdu_validation_reason`

If the current image only runs full bringup validation, use:
- `test-results/RENODE_TEST_GUIDE.md`

## 5) What To Check In Renode
Read these two exported symbols from the standalone RRDU validation image:
- `g_rrdu_validation_result`
- `g_rrdu_validation_reason`

Expected:
- Success build:
  - result = `0x00000001`
  - reason = `0x00000000`
- Fault build:
  - result = `0xFFFFFFFF`
  - reason = implementation-defined failure code from RRDU path

## 6) Notes
- This validates RRDU in isolation.
- It does not prove the whole bringup sequence.
- For supervisor evidence of system-level sequencing, use:
  - `test-results/RENODE_TEST_GUIDE.md`
