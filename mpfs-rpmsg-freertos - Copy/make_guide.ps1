Set-Content -LiteralPath 'RENODE_RUN_PLACEHOLDER_GUIDE.txt' -Value @'
Renode Quick Run Checklist

1. Build and locate your ELF
   - Run your usual build (SoftConsole or make) for the subsystem you care about.
   - Locate the resulting ELF (SoftConsole output like Master-Default\mpfs-rpmsg-master.elf, Remote-Default\..., or another build folder). Swap the placeholder [PATH-TO-ELF] below with that exact path when you load it.

2. (Optional) Map a short drive
   - Shortcut: cmd /c subst X: \ FULL-WORKSPACE-PATH\
   - Replace FULL-WORKSPACE-PATH with C:\Microchip\SoftConsole-v2022.2-RISC-V-747\extras\workspace.examples\mpfs-rpmsg-freertos - Copy (or your working folder) to simplify later commands.
   - If you prefer not to map, keep using the full Windows path instead of X:\.

3. Start Renode monitor
   - Launch Renode: C:\Program Files\Renode\bin\Renode.exe --console --disable-gui --plain
   - Inside Renode run:
     `
     mach create
     machine LoadPlatformDescription @platforms/boards/BOARD_NAME.repl
     `
   - Replace BOARD_NAME.repl with the actual board description that matches your target hardware (for the Icicle kit build use mpfs-icicle-kit.repl).

4. Load the ELF
   - Command: sysbus LoadELF \ PATH-TO-ELF\
   - Swap PATH-TO-ELF with the ELF path from step 1. Use X:\... if you created the drive alias, otherwise keep the full path.

5. Run the simulation
   - Execute: emulation RunFor \" "00:00:00.050\ (or another duration that lets your firmware complete initialization).
   - You can also use start/continue if you prefer stepping through interactively.

6. Read exported values
   - Copy the register reads you need, for example:
     `
    sysbus ReadDoubleWord 0x91809ea4   # bringup_result
    sysbus ReadDoubleWord 0x91809ea8   # bringup_state
    sysbus ReadDoubleWord 0x91809ed8   # rrdu_state
    sysbus ReadDoubleWord 0x91809ef0   # rrdu_fault
     `
   - Replace the addresses/comments with the registers or symbols your subsystem uses; these are only examples from the RRDU bringup guide.
   - Compare the returned values against the expected constants for your test case (success/failure or other metrics).

7. Optional: Capture transition trace

8. Wrap up
   - Type quit in Renode to exit.
   - If you mapped X:, unmap it: cmd /c subst X: /D.
   - Note which ELF you used, the board .repl, and the values you observed so you can reproduce the run or share results with the team.

Replace every placeholder (PATH-TO-ELF, BOARD_NAME, FULL-WORKSPACE-PATH) with the actual path, board description, or register before handing this guide over. This single document lets him adapt the flow to whichever ELF he builds while keeping the Renode process consistent.
'@ -Encoding ascii
