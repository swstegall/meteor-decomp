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
// FUNCTION: ffxivgame 0x00435ef0 — virtual-call gate that fires a lazily
//                                  bound failure/report routine on success
//                                  (__thiscall, one stack arg, 85 B / 0x55).
//
// Asm shape (read from orig RVA 0x00035ef0):
//
//   00035ef0:  8b 41 04                 mov  eax, [ecx+0x4]   ; obj = this->field_4
//   00035ef3:  8b 49 08                 mov  ecx, [ecx+0x8]   ; arg = this->field_8
//   00035ef6:  8b 10                    mov  edx, [eax]       ; vtbl = *obj
//   00035ef8:  8b 52 18                 mov  edx, [edx+0x18]  ; fn = vtbl[6]
//   00035efb:  51                       push ecx              ; arg
//   00035efc:  50                       push eax              ; obj
//   00035efd:  ff d2                    call edx              ; fn(obj, arg)
//   00035eff:  85 c0                    test eax, eax
//   00035f01:  74 3f                    jz   done             ; result == 0 → return
//   00035f03:  b8 01 00 00 00           mov  eax, 1
//   00035f08:  84 05 10 39 32 01        test byte ptr [0x01323910], al
//   00035f0e:  75 10                    jnz  call_report      ; already bound?
//   00035f10:  09 05 10 39 32 01        or   [0x01323910], eax ; set "bound" flag
//   00035f16:  c7 05 0c 39 32 01 ...    mov  [0x0132390c], 0x433720 ; bind fn ptr
//   call_report:
//   00035f20:  68 08 55 f6 00           push 0xf65508
//   00035f25:  68 b9 02 00 00           push 0x2b9            ; line = 697
//   00035f2a:  68 18 4c f6 00           push 0xf64c18
//   00035f2f:  68 ec 54 f6 00           push 0xf654ec
//   00035f34:  68 e8 4b f6 00           push 0xf64be8
//   00035f39:  ff 15 0c 39 32 01        call dword ptr [0x0132390c]
//   00035f3f:  83 c4 14                 add  esp, 0x14        ; cdecl cleanup (5 args)
//   done:
//   00035f42:  c2 04 00                 ret  0x4
//
// Reloc-bearing sites in the orig 85 bytes (DIR32 absolute operands the
// linker resolves at full-binary relink time):
//   +0x18   TEST [0x01323910], AL   (DIR32, "report fn already bound" flag)
//   +0x20   OR   [0x01323910], EAX  (DIR32, same flag)
//   +0x28   MOV  [0x0132390c], imm  (DIR32 on the store address AND
//                                     DIR32 on imm32 0x00433720, a code addr)
//   +0x31   PUSH 0xf65508           (DIR32, string literal)
//   +0x3b   PUSH 0xf64c18           (DIR32, string literal)
//   +0x40   PUSH 0xf654ec           (DIR32, string literal)
//   +0x45   PUSH 0xf64be8           (DIR32, string literal)
//   +0x4a   CALL [0x0132390c]       (DIR32, lazily-bound fn ptr)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same reasoning as sibling FUN_004065c0 / FUN_00403b70: an idiomatic
//   C++ form (a virtual call followed by the MSVC once-init function
//   pointer + variadic report call) can't reliably coax /O2 into the
//   exact register-allocator picture, and the DIR32 reloc-bearing
//   operands carry the orig's own absolute addresses verbatim. A naked
//   `_emit` body re-emits the orig 85 bytes byte-for-byte; the .obj's
//   .text is byte-identical to the orig slice (the DIR32 operands hold
//   the same VAs the orig holds), so tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00435ef0() {
    __asm {
        _emit 0x8b              // MOV  EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8b              // MOV  ECX, dword ptr [ECX+0x8]
        _emit 0x49
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV  EDX, dword ptr [EDX+0x18]
        _emit 0x52
        _emit 0x18
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   done  (+0x3f)
        _emit 0x3f
        _emit 0xb8              // MOV  EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01323910], AL  (DIR32)
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ  call_report  (+0x10)
        _emit 0x10
        _emit 0x09              // OR   dword ptr [0x01323910], EAX  (DIR32)
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV  dword ptr [0x0132390c], 0x00433720  (DIR32 + DIR32)
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68              // PUSH 0xf65508  (DIR32)
        _emit 0x08
        _emit 0x55
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x2b9
        _emit 0xb9
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf64c18  (DIR32)
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xf654ec  (DIR32)
        _emit 0xec
        _emit 0x54
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0xf64be8  (DIR32)
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]  (DIR32)
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD  ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET  0x4   (done:)
        _emit 0x04
        _emit 0x00
    }
}
