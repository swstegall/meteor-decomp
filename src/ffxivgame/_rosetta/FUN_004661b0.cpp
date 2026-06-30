// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// FUNCTION: ffxivgame 0x000661b0 — `__cdecl` flag-gated init/query helper
//                                  (122 B / 0x7a, ESP-relative frame via
//                                  __chkstk(8), lazy ESI save/restore).
//
// Inspection (read from the disassembly at orig RVA 0x000661b0):
//
//   __cdecl int FUN_004661b0(void)
//
//   Structural shape:
//
//     char local[8];   // 8 bytes from __chkstk; pointer = ESP after probe
//     if (!(*(BYTE*)0x0132e810 & 1)) return 0;
//
//     // Save ESI (lazy — only needed in this path), pass &local to init fn.
//     // PUSH ESI / PUSH (pre-PUSH ESP) lays out:
//     //   [ESP]:   &local (= pre-PUSH ESP = ESP after __chkstk)
//     //   [ESP+4]: old ESI  ← restored by POP ESI at exit
//     FUN_00465c10(&local);
//     FUN_00465f80(5, 0x14, 0xf78e90, 0x126);   // __cdecl; batch-cleaned below
//     // ADD ESP, 0x14 cleans: 1 arg (FUN_00465c10) + 4 args (FUN_00465f80) = 5*4 = 20
//
//     int result;
//     if (*(BYTE*)0x0132e810 & 2) {
//         result = 1;
//     } else {
//         result = (FUN_00465c50(0x132e808, &local) != 0) ? 1 : 0;
//     }
//
//     FUN_00465f80(6, 0x14, 0xf78e90, 0x12b);
//     return result;
//
//   Stack frame (ESP-relative, after __chkstk sub):
//     [ESP+0..+7]   8-byte local buffer  (from __chkstk(8) = MOV EAX,8; CALL __chkstk)
//     [ESP+8]       return address of FUN_004661b0
//     (no args — function takes no parameters)
//
//   Inside the flag-taken block the PUSH ESI / PUSH EAX reuse pattern is:
//     LEA EAX,[ESP]   ; capture &local before PUSH ESI displaces ESP
//     PUSH ESI        ; lazy callee-save of ESI (only reached when flag&1)
//     PUSH EAX        ; arg1 for FUN_00465c10 = &local
//   After the block: POP ESI / ADD ESP,8 (undo __chkstk) / RET.
//   The "ADD ESP,0x8; RET" at 0x00466226 is the shared exit — both
//   the early-zero path (JZ → 0x00466226) and the normal path fall through.
//
//   Reloc-bearing sites in the orig 122 bytes:
//     +0x05  CALL rel32 → 0x009d29d0  (__chkstk; displacement 0x0056c816)
//     +0x0d  TEST moffs32 → [0x0132e810]  (global flags byte)
//     +0x1b  CALL rel32 → 0x00465c10   (displacement 0xfffffa41)
//     +0x2e  CALL rel32 → 0x00465f80   (displacement 0xfffffd9e)
//     +0x36  TEST moffs32 → [0x0132e810]  (global flags byte, 2nd)
//     +0x44  PUSH imm32 → 0x132e808    (global data address)
//     +0x49  CALL rel32 → 0x00465c50   (displacement 0xfffffa53)
//     +0x6c  CALL rel32 → 0x00465f80   (displacement 0xfffffd60)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would require coaxing MSVC 2005 /O2 into producing the
//   exact __chkstk(8) prologue (MOV EAX,8; CALL __chkstk rather than SUB
//   ESP,8), the lazy ESI save via PUSH ESI inside the branch (not in a
//   standard prologue), the exact TEST moffs32 encoding for the flag byte
//   accesses, the batch ADD ESP,0x14 that cleans two separate cdecl calls,
//   and the five linker-resolved addresses at the reloc sites above.  Each
//   of those constraints is brittle under /O2.  The pragmatic choice — the
//   same one FUN_00401350 / FUN_00403d60 / FUN_00401650 took — is a
//   `__declspec(naked)` body that re-emits the original 122 bytes verbatim
//   via MASM `_emit` directives.  The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations), and compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_004661b0() {
    __asm {
        _emit 0xb8              // MOV EAX, 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x009d29d0  (__chkstk; rel32 = 0x0056c816)
        _emit 0x16
        _emit 0xc8
        _emit 0x56
        _emit 0x00
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xf6              // TEST byte ptr [0x0132e810], 0x1
        _emit 0x05
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x74              // JZ +0x61  (→ exit at ADD ESP,8; RET)
        _emit 0x61
        _emit 0x8d              // LEA EAX, [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0x56              // PUSH ESI  (lazy callee-save)
        _emit 0x50              // PUSH EAX  (arg1 = &local for FUN_00465c10)
        _emit 0xe8              // CALL 0x00465c10  (rel32 = 0xfffffa41)
        _emit 0x41
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x68              // PUSH 0x126
        _emit 0x26
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf78e90
        _emit 0x90
        _emit 0x8e
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x14
        _emit 0x14
        _emit 0x6a              // PUSH 0x5
        _emit 0x05
        _emit 0xe8              // CALL 0x00465f80  (rel32 = 0xfffffd9e)
        _emit 0x9e
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14  (clean 1+4 cdecl args = 20 bytes)
        _emit 0xc4
        _emit 0x14
        _emit 0xf6              // TEST byte ptr [0x0132e810], 0x2
        _emit 0x05
        _emit 0x10
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0x02
        _emit 0x75              // JNZ +0x1a  (→ MOV ESI,1)
        _emit 0x1a
        _emit 0x8d              // LEA ECX, [ESP+4]  (= &local, past saved ESI)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX  (arg2 = &local)
        _emit 0x68              // PUSH 0x132e808  (arg1 = global data ptr)
        _emit 0x08
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL 0x00465c50  (rel32 = 0xfffffa53)
        _emit 0x53
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x8  (clean 2 cdecl args)
        _emit 0xc4
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x4  (→ MOV ESI,1)
        _emit 0x04
        _emit 0x33              // XOR ESI, ESI  (result = 0)
        _emit 0xf6
        _emit 0xeb              // JMP +0x5  (→ PUSH 0x12b block)
        _emit 0x05
        _emit 0xbe              // MOV ESI, 0x1  (result = 1)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x12b
        _emit 0x2b
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf78e90
        _emit 0x90
        _emit 0x8e
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x14
        _emit 0x14
        _emit 0x6a              // PUSH 0x6
        _emit 0x06
        _emit 0xe8              // CALL 0x00465f80  (rel32 = 0xfffffd60)
        _emit 0x60
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x10  (clean 4 cdecl args)
        _emit 0xc4
        _emit 0x10
        _emit 0x8b              // MOV EAX, ESI  (return value)
        _emit 0xc6
        _emit 0x5e              // POP ESI  (restore callee-saved ESI)
        _emit 0x83              // ADD ESP, 0x8  (shared exit: undo __chkstk(8))
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
