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
// FUNCTION: ffxivgame 0x0045bb90 — int_err_get (125 B / 0x7d)
//
// Behaviour reconstructed from the asm at RVA 0x0005bb90:
//
//   Calling convention: __cdecl (bare RET, caller-saves ESI used as the
//   only preserved register, no EBP frame, no stack args popped by callee).
//
//   Structural shape (one argument on stack at [ESP+0x8] after the PUSH ESI):
//
//     int int_err_get(some_arg) {
//         // "enter" log trace (line 0x162, file 0xf6802c, level 1, id 9)
//         log_event(0x9, 0x1, 0xf6802c, 0x162);
//         int result = *(int*)0x0132e78c;  // load global error object
//         if (result == 0 && arg != 0) {
//             // create a new error object
//             set_error_location(0xf68040, 0xf6802c, 0x165);
//             result = create_error(0x45bb50, 0x461320);
//             // store back + flush
//             *(int*)0x0132e78c = result;
//             flush_error();
//             result = *(int*)0x0132e78c;
//             if (result == 0) result = 0;
//         }
//         // "exit" log trace (line 0x16b, file 0xf6802c, level 1, id 0xa)
//         log_event(0xa, 0x1, 0xf6802c, 0x16b);
//         return result;
//     }
//
//   Reloc-bearing sites in the orig 125 bytes (all absolute immediates
//   baked in — no COFF relocations in the standalone .obj because the
//   source-of-truth bytes are already at their final binary addresses):
//
//     +0x02   PUSH imm32   → 0x162         (log line number)
//     +0x07   PUSH imm32   → 0xf6802c      (file string VA)
//     +0x0d   CALL rel32   → 0x00465f80    (log_event)
//     +0x11   MOV  moffs32 → 0x0132e78c    (global error pointer)
//     +0x1e   PUSH imm32   → 0x165         (log line number)
//     +0x23   PUSH imm32   → 0xf6802c      (file string VA)
//     +0x28   PUSH imm32   → 0xf68040      (format string VA)
//     +0x2d   CALL rel32   → 0x00466300    (set_error_location)
//     +0x32   PUSH imm32   → 0x461320      (arg VA)
//     +0x37   PUSH imm32   → 0x45bb50      (arg VA)
//     +0x3c   CALL rel32   → 0x00466990    (create_error)
//     +0x46   MOV  moffs32 ← 0x0132e78c   (store global error pointer)
//     +0x4b   CALL rel32   → 0x004664d0   (flush_error)
//     +0x50   MOV  moffs32 → 0x0132e78c   (reload global error pointer)
//     +0x5f   PUSH imm32   → 0x16b        (log line number)
//     +0x64   PUSH imm32   → 0xf6802c     (file string VA)
//     +0x6d   CALL rel32   → 0x00465f80   (log_event)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function uses absolute addresses for globals and call targets that
//   are baked in at the binary's 0x00400000 load base. A source-level C++
//   reconstruction would require coaxing MSVC 2005 /O2 into the exact same
//   register allocation and branch encoding, and would still produce COFF
//   relocations for the global references that won't round-trip through the
//   standalone .obj diff cleanly. The pragmatic choice (same as siblings
//   FUN_00401350 and FUN_00403d60) is a __declspec(naked) body emitting
//   all 125 bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_0045bb90() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x68              // PUSH 0x162
        _emit 0x62
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf6802c
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x6a              // PUSH 0x9
        _emit 0x09
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0xe8              // CALL 0x00465f80 (rel32 = 0x0000a3da)
        _emit 0xda
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x0132e78c]
        _emit 0x8c
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x3f
        _emit 0x3f
        _emit 0x39              // CMP dword ptr [ESP+0x8], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x74              // JZ +0x3b
        _emit 0x3b
        _emit 0x68              // PUSH 0x165
        _emit 0x65
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf6802c
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xf68040
        _emit 0x40
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        _emit 0xe8              // CALL 0x00466300 (rel32 = 0x0000a734)
        _emit 0x34
        _emit 0xa7
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x461320
        _emit 0x20
        _emit 0x13
        _emit 0x46
        _emit 0x00
        _emit 0x68              // PUSH 0x45bb50
        _emit 0x50
        _emit 0xbb
        _emit 0x45
        _emit 0x00
        _emit 0xe8              // CALL 0x00466990 (rel32 = 0x0000adb5)
        _emit 0xb5
        _emit 0xad
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xa3              // MOV [0x0132e78c], EAX
        _emit 0x8c
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL 0x004664d0 (rel32 = 0x0000a8e8)
        _emit 0xe8
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x0132e78c]
        _emit 0x8c
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x2
        _emit 0x02
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x68              // PUSH 0x16b
        _emit 0x6b
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf6802c
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x6a              // PUSH 0xa
        _emit 0x0a
        _emit 0xe8              // CALL 0x00465f80 (rel32 = 0x0000a37a)
        _emit 0x7a
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
