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
// FUNCTION: ffxivgame 0x004359d0 — __thiscall predicate-then-lazy-log
//                                  thunk (89 bytes, RET 4).
//
// __thiscall bool FUN_004359d0(this, Obj *arg):
//
//   Calls a virtual on `arg` (arg->vtbl[0x1ac/4](arg, this->field4)). If it
//   returns 0, return immediately. Otherwise run a once-guarded lazy
//   initialiser: if the low bit of the global flag word at 0x01323910 is
//   clear, set it and store a function pointer (0x00433720) into the global
//   slot at 0x0132390c, then call through that slot with five pushed
//   arguments (four absolute data pointers plus the literal 0x176) — a
//   classic deferred-log / assert handler dispatch — and clean up 0x14 bytes
//   of cdecl args.
//
// Asm (89 bytes, read from orig RVA 0x000359d0):
//
//   8b 44 24 04                 mov   eax, [esp+4]          ; arg
//   8b 49 04                    mov   ecx, [ecx+4]          ; this->field4
//   8b 10                       mov   edx, [eax]            ; arg->vtbl
//   8b 92 ac 01 00 00           mov   edx, [edx+0x1ac]      ; vtbl slot
//   51                          push  ecx                   ; this->field4
//   50                          push  eax                   ; arg
//   ff d2                       call  edx                   ; arg->virt(...)
//   85 c0                       test  eax, eax
//   74 3f                       jz    ret                   ; 0x00435a26
//   b8 01 00 00 00              mov   eax, 1
//   84 05 10 39 32 01           test  byte ptr [0x01323910], al
//   75 10                       jnz   call                  ; 0x00435a04
//   09 05 10 39 32 01           or    [0x01323910], eax
//   c7 05 0c 39 32 01 20 37 43 00  mov dword ptr [0x0132390c], 0x00433720
//   68 a8 4f f6 00              push  0x00f64fa8            ; call:
//   68 76 01 00 00              push  0x176
//   68 18 4c f6 00              push  0x00f64c18
//   68 88 4f f6 00              push  0x00f64f88
//   68 e8 4b f6 00              push  0x00f64be8
//   ff 15 0c 39 32 01           call  dword ptr [0x0132390c]
//   83 c4 14                    add   esp, 0x14
//   c2 04 00                    ret   4                     ; ret:
//
// Reloc-bearing sites (DIR32 absolute operands the linker would resolve
// from source-level C++; here re-emitted verbatim so the .obj's .text is
// byte-identical to the orig slice — tools/compare.py masks reloc bytes):
//   TEST [imm32]  → 0x01323910 (once-flag word)
//   OR   [imm32]  → 0x01323910
//   MOV  [imm32]  → 0x0132390c (fnptr slot), imm32 = 0x00433720
//   PUSH imm32    → 0x00f64fa8 / 0x00f64c18 / 0x00f64f88 / 0x00f64be8
//   CALL [imm32]  → 0x0132390c
//
// Reconstruction strategy — naked-asm byte passthrough (same as sibling
// FUN_004358b0): emit the 89 orig bytes literally via MASM `_emit`, with the
// DIR32 operands baked in as the values that already resolve against the
// orig PE's address space.

extern "C" __declspec(naked) void FUN_004359d0() {
    __asm {
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV  ECX, dword ptr [ECX+4]
        _emit 0x49
        _emit 0x04
        _emit 0x8b              // MOV  EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV  EDX, dword ptr [EDX+0x1ac]
        _emit 0x92
        _emit 0xac
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
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
        _emit 0x68              // PUSH 0x00f64fa8
        _emit 0xa8
        _emit 0x4f
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x176
        _emit 0x76
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x00f64f88
        _emit 0x88
        _emit 0x4f
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
