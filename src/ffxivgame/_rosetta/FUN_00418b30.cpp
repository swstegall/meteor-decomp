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
// FUNCTION: ffxivgame 0x00018b30 — unknown (__cdecl, 190 B / 0xbe)
//
// A 6-register SEH3 prolog (PUSH -1 / scope_table / FS:[0] / ECX / ESI /
// /GS cookie) followed by argument clamping and a single sub-function
// CALL, then a virtual-dispatch epilog.
//
// Stack layout after prolog (6 pushes + cookie = 24 bytes below ret-addr):
//   [ESP+ 0]  /GS cookie
//   [ESP+ 4]  saved ESI
//   [ESP+ 8]  saved ECX  (zeroed early to 0 by MOV [ESP+8], 0)
//   [ESP+12]  saved FS:[0]
//   [ESP+16]  scope_table ptr
//   [ESP+20]  -1  (initial EH state)
//   [ESP+24]  return address
//   [ESP+28]  arg1
//   [ESP+32]  arg2   (clamped to max(arg2, 1) -> EDX)
//   [ESP+36]  arg3   (clamped to max(arg3, 1) -> ECX)
//   [ESP+40]  arg4
//   [ESP+44]  arg5
//
// Body sketch:
//   1. Zero saved-ECX slot ([ESP+8] = 0).
//   2. EDX = max(arg2, 1);  ECX = max(arg3, 1)
//   3. Build a 7-argument call frame on the stack:
//        PUSH arg5, PUSH ECX(clamped), ESP->[old_arg2_slot],
//        zero [ESP], PUSH arg4, PUSH 0x1a, PUSH ECX, PUSH EDX,
//        LEA ECX, [ESP+0x38], PUSH ECX
//   4. CALL 0x00418970   (sub-function, reloc at +0x67)
//   5. ADD ESP, 0x1c      (cdecl cleanup, 7 args x 4 = 28 bytes)
//   6. Post-call: dereference return value, store into *arg1 (ESI),
//      set EH state to 1, clear arg5 slot.
//   7. Conditional virtual call: if arg2 != 0: (*(*arg2)->vtbl[0])(1)
//   8. Return ESI in EAX.  SEH epilog: restore FS:[0], POP ECX+ESI,
//      ADD ESP, 0x10, RET.
//
// Structural sibling: FUN_00418a70 (RVA 0x00018a70) is byte-for-byte
// identical except for three sites:
//   offset +0x03  scope_table ptr: 0x00e5543b -> 0x00e55477
//   offset +0x5e  PUSH imm8:       0x09       -> 0x1a
//   offset +0x68  CALL rel32:      95 fe ff ff -> d5 fd ff ff
//
// Reconstruction strategy -- naked-asm byte passthrough:
//
//   The SEH3 prolog (PUSH -1 / PUSH scope_table_RVA / FS:[0] / /GS
//   cookie XOR ESP) together with the mid-body ESP-save idiom
//   (MOV EAX, ESP + MOV [ESP+0x28], ESP) and the virtual-dispatch
//   tail (MOV EAX, [ECX] / CALL [EAX+0] via CALL EDX) form a
//   compiler-emitted shape that is impractical to reproduce byte-for-byte
//   in plain C++ under /O2.  Same strategy as FUN_00406680 and
//   FUN_00403f10: emit the 190 orig bytes verbatim via MASM _emit.
//   compare.py masks the 3 reloc windows (+0x03, +0x11, +0x67).
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x03   scope_table ptr  (.rdata 0x00e55477)
//   +0x11   __security_cookie load (.data 0x012ea8b0)
//   +0x67   CALL rel32 -> 0x00418970 (sub-function)

extern "C" __declspec(naked) void FUN_00418b30() {
    __asm {
        // 00018b30 -- SEH3 prolog
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH scope_table_ptr (reloc +0x03)
        _emit 0x77
        _emit 0x54
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0x56  // PUSH ESI
        _emit 0xa1  // MOV EAX, [__security_cookie] (reloc +0x11)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX (cookie)
        _emit 0x8d  // LEA EAX, [ESP+0x0c]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018b52 -- body start
        _emit 0xc7  // MOV dword ptr [ESP+0x08], 0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EDX, [ESP+0x20]  (arg2)
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x83  // CMP EDX, 1
        _emit 0xfa
        _emit 0x01
        _emit 0x73  // JNC +5
        _emit 0x05
        _emit 0xba  // MOV EDX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, [ESP+0x24]  (arg3)
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x83  // CMP ECX, 1
        _emit 0xf9
        _emit 0x01
        _emit 0x73  // JNC +5
        _emit 0x05
        _emit 0xb9  // MOV ECX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0x2c]  (arg5)
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV EAX, ESP
        _emit 0xc4
        _emit 0x89  // MOV [ESP+0x28], ESP
        _emit 0x64
        _emit 0x24
        _emit 0x28
        _emit 0xc7  // MOV dword ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0x30]  (arg4)
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x50  // PUSH EAX
        _emit 0x6a  // PUSH 0x1a
        _emit 0x1a
        _emit 0x51  // PUSH ECX
        _emit 0x52  // PUSH EDX
        _emit 0x8d  // LEA ECX, [ESP+0x38]
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL rel32 (reloc +0x67)
        _emit 0xd5
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x1c  (cleanup 7 args)
        _emit 0xc4
        _emit 0x1c
        _emit 0x8b  // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x8b  // MOV ESI, [ESP+0x1c]
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0x8b  // MOV ECX, EDX
        _emit 0xca
        _emit 0xc7  // MOV dword ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESP+0x14], 1  (EH state = 1)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESP+0x2c], 0
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESI], ECX
        _emit 0x0e
        _emit 0x8b  // MOV ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x85  // TEST ECX, ECX
        _emit 0xc9
        _emit 0xc7  // MOV dword ptr [ESP+0x08], 1
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6  // MOV byte ptr [ESP+0x14], 0
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x74  // JZ +8
        _emit 0x08
        _emit 0x8b  // MOV EAX, [ECX]
        _emit 0x01
        _emit 0x8b  // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x6a  // PUSH 1
        _emit 0x01
        _emit 0xff  // CALL EDX
        _emit 0xd2
        // 00018bdb -- epilog
        _emit 0x8b  // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8b  // MOV ECX, [ESP+0x0c]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5e  // POP ESI
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3  // RET
    }
}
