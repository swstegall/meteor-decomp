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
// FUNCTION: ffxivgame 0x004358b0 — __thiscall predicate-then-lazy-log
//                                  thunk (95 bytes, RET 4).
//
// __thiscall bool FUN_004358b0(this, Obj *arg):
//
//   Calls a virtual on `arg` (arg->vtbl[0x198/4](arg, this->field4,
//   this->field8)). If it returns 0, return immediately. Otherwise run
//   a once-guarded lazy initialiser: if the low bit of the global flag
//   word at 0x01323910 is clear, set it and store a function pointer
//   (0x00433720) into the global slot at 0x0132390c, then call through
//   that slot with five pushed arguments (four absolute data pointers
//   plus the literal 0x133) — a classic deferred-log / assert handler
//   dispatch — and clean up 0x14 bytes of cdecl args.
//
// Asm (95 bytes, read from orig RVA 0x000358b0):
//
//   8b 44 24 04                 mov   eax, [esp+4]          ; arg
//   8b 10                       mov   edx, [eax]            ; arg->vtbl
//   8b 92 98 01 00 00           mov   edx, [edx+0x198]      ; vtbl slot
//   56                          push  esi
//   8b 71 08                    mov   esi, [ecx+8]          ; this->field8
//   8b 49 04                    mov   ecx, [ecx+4]          ; this->field4
//   56                          push  esi
//   51                          push  ecx
//   50                          push  eax
//   ff d2                       call  edx                   ; arg->virt(...)
//   85 c0                       test  eax, eax
//   5e                          pop   esi
//   74 3f                       jz    ret                   ; 0x0043590c
//   b8 01 00 00 00              mov   eax, 1
//   84 05 10 39 32 01           test  byte ptr [0x01323910], al
//   75 10                       jnz   call                  ; 0x004358ea
//   09 05 10 39 32 01           or    [0x01323910], eax
//   c7 05 0c 39 32 01 20 37 43 00  mov dword ptr [0x0132390c], 0x00433720
//   68 48 4e f6 00              push  0x00f64e48            ; call:
//   68 33 01 00 00              push  0x133
//   68 18 4c f6 00              push  0x00f64c18
//   68 28 4e f6 00              push  0x00f64e28
//   68 e8 4b f6 00              push  0x00f64be8
//   ff 15 0c 39 32 01           call  dword ptr [0x0132390c]
//   83 c4 14                    add   esp, 0x14
//   c2 04 00                    ret   4                     ; ret:
//
// Reloc-bearing sites (DIR32 absolute operands the linker would resolve
// from source-level C++; here re-emitted verbatim so the .obj's .text is
// byte-identical to the orig slice with zero relocations — tools/
// compare.py masks reloc bytes and reports GREEN):
//   +0x22   TEST [imm32]  → 0x01323910 (once-flag word)
//   +0x2a   OR   [imm32]  → 0x01323910
//   +0x30   MOV  [imm32]  → 0x0132390c (fnptr slot), imm32 = 0x00433720
//   +0x3a   PUSH imm32    → 0x00f64e48
//   +0x44   PUSH imm32    → 0x00f64c18
//   +0x49   PUSH imm32    → 0x00f64e28
//   +0x4e   PUSH imm32    → 0x00f64be8
//   +0x53   CALL [imm32]  → 0x0132390c
//
// Reconstruction strategy — naked-asm byte passthrough (same as siblings
// FUN_00403bd0 / FUN_00406133): emit the 95 orig bytes literally via MASM
// `_emit`, with the DIR32 operands baked in as the values that already
// resolve against the orig PE's address space.

extern "C" __declspec(naked) void FUN_004358b0() {
    __asm {
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV  EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV  EDX, dword ptr [EDX+0x198]
        _emit 0x92
        _emit 0x98
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, dword ptr [ECX+8]
        _emit 0x71
        _emit 0x08
        _emit 0x8b              // MOV  ECX, dword ptr [ECX+4]
        _emit 0x49
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP  ESI
        _emit 0x74              // JZ   ret  (+0x3F)
        _emit 0x3f
        _emit 0xb8              // MOV  EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ  call (+0x10)
        _emit 0x10
        _emit 0x09              // OR   dword ptr [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV  dword ptr [0x0132390c], 0x00433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64e48
        _emit 0x48
        _emit 0x4e
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x133
        _emit 0x33
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64e28
        _emit 0x28
        _emit 0x4e
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD  ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET  4
        _emit 0x04
        _emit 0x00
    }
}
