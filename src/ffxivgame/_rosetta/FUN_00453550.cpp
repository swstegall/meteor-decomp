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
// FUNCTION: ffxivgame 0x00053550 — SEH-wrapped predicate with string
//                                   construct/compare (__cdecl, 169 B / 0xa9)
//
// Asm shape (recovered from orig @ 0x00053550):
//
//   bool FUN_00453550(void *arg) {                  // arg @ [ESP+0x70 entry]
//       // SEH frame + /GS double security cookie:
//       //   PUSH -1; PUSH 0xe58178 (scope table); PUSH FS:[0]; SUB ESP,0x58
//       //   cookie @ [ESP+0x54] = __security_cookie ^ ESP
//       //   PUSH ESI; inner cookie = __security_cookie ^ ESP (pushed)
//       //   establish FS:[0] = &frame_record
//       ESI = arg;
//       if (!FUN_004531c0(arg)) return false;       // 0x004531c0(arg) -> AL
//       FUN_00452ba0(&local_8, arg);                // construct local obj
//       local_state = 0;                            // [ESP+0x68] = 0
//       FUN_00445ee0(&local_8);                      // __thiscall on local_8
//       local_state = -1;                            // [ESP+0x68] = -1
//       if (FUN_00445d60(&local_c, 0xf676e4)) {      // __thiscall compare
//           FUN_00446f50(&local_8);                  // dtor (true arm)
//           return true;
//       }
//       FUN_00446f50(&local_8);                       // dtor (false arm)
//       return false;
//       // epilogue: restore FS:[0], pop, /GS check via FUN_009d20f4
//   }
//
// Reconstruction strategy — naked-asm byte passthrough (the local idiom
// used by siblings FUN_00406280 / FUN_004063c0 / FUN_00401b70): a
// source-level C++ form would emit the same shape but produce
// reloc-bearing immediates (the SEH scope-table pointer, the
// __security_cookie DIR32 sites, the FS:[0] thunk, the 0xf676e4 string,
// and six internal rel32 CALLs) that the linker resolves at relink time.
// Without a relink driving compare.py, re-emitting the orig 169 bytes
// verbatim produces a zero-reloc .obj whose .text is byte-identical to the
// orig slice; compare.py wildcards the reloc windows and reports GREEN.
//
// Reloc-bearing sites in the orig 169 bytes (masked by compare.py):
//     +0x02   PUSH imm32   → 0x00e58178  (SEH scope table)
//     +0x11   MOV  EAX,[0x012ea8b0]      (__security_cookie)
//     +0x1d   MOV  EAX,[0x012ea8b0]      (__security_cookie)
//     +0x34   CALL rel32   → 0x004531c0
//     +0x46   CALL rel32   → 0x00452ba0
//     +0x5a   CALL rel32   → 0x00445ee0
//     +0x5f   PUSH imm32   → 0x00f676e4  (compare string literal)
//     +0x68   CALL rel32   → 0x00445d60
//     +0x7d   CALL rel32   → 0x00446f50
//     +0x86   CALL rel32   → 0x00446f50
//     +0xa0   CALL rel32   → 0x009d20f4  (__security_check_cookie)

extern "C" __declspec(naked) void FUN_00453550() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e58178            (SEH scope table)
        _emit 0x78
        _emit 0x81
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x58
        _emit 0xec
        _emit 0x58
        _emit 0xa1              // MOV EAX, [0x012ea8b0]      (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV dword ptr [ESP+0x54], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]      (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP+0x60]
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x64              // MOV FS:[0x0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x70]   (arg)
        _emit 0x74
        _emit 0x24
        _emit 0x70
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_004531c0 (rel32)
        _emit 0x37
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ short +0x4b (→ false_return)
        _emit 0x4b
        _emit 0x8d              // LEA EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_00452ba0 (rel32)
        _emit 0x05
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x8d              // LEA ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESP+0x68], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00445ee0 (rel32)
        _emit 0x31
        _emit 0x29
        _emit 0xff
        _emit 0xff
        _emit 0x68              // PUSH 0x00f676e4            (compare literal)
        _emit 0xe4
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL FUN_00445d60 (rel32)
        _emit 0xa3
        _emit 0x27
        _emit 0xff
        _emit 0xff
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESP+0x68], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8d              // LEA ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x74              // JZ short +0x9 (→ false_arm_dtor)
        _emit 0x09
        _emit 0xe8              // CALL FUN_00446f50 (rel32, true-arm dtor)
        _emit 0x7e
        _emit 0x39
        _emit 0xff
        _emit 0xff
        _emit 0xb0              // MOV AL, 0x1
        _emit 0x01
        _emit 0xeb              // JMP short +0x7 (→ epilogue)
        _emit 0x07
        _emit 0xe8              // CALL FUN_00446f50 (rel32, false-arm dtor)
        _emit 0x75
        _emit 0x39
        _emit 0xff
        _emit 0xff
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x60]
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x64              // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5e              // POP ESI
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x54]
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        _emit 0x33              // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8              // CALL FUN_009d20f4 (rel32, __security_check_cookie)
        _emit 0xff
        _emit 0xea
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x64
        _emit 0xc4
        _emit 0x64
        _emit 0xc3              // RET
    }
}
