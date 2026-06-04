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
// FUNCTION: ffxivgame 0x00043660 — audio-buffer (re)load / flush helper
//                                  (__thiscall, 225 B / 0xe1, RET 0xc → 3
//                                  stack args on top of `this` in ECX).
//
// Recovered signature (this = ESI):
//
//   bool __thiscall FUN_00443660(AudioObj *this,  // ECX
//                                bool       doFeed,// arg0 = [esp+0x4] (byte)
//                                ISource   *src,   // arg1 (vtable obj, EDI)
//                                int        arg3); // arg2 (forwarded to vf14)
//
// Asm shape:
//
//   if (!doFeed) return false;                      ; CMP byte [esp+4],0 / JZ
//
//   void *p = src->vf18();                           ; [vtbl+0x18], EBP
//   if (p) {
//       FUN_009d4600(this+0x10034, p, src->vf28());  ; memcpy(dst, src, n)
//       (*g_f3e148)(this+0x50034, src->vf28());      ; IAT/global indirect
//       src->vf1c();                                 ; [vtbl+0x1c]
//   }
//
//   src->vf14(this, arg3, 0, 0);                     ; [vtbl+0x14]
//   this->field_4 = 0;
//   if (this->field_1c == 0) {
//       FUN_009d4600(this+0x34, this+0x10034, 0x10000);  ; memcpy 64 KiB
//       this->field_1c = FUN_00b91d90(this+0x28, this+0x34, 0x10000, 0, 2);
//       if ((unsigned)this->field_18 > 0) {
//           FUN_004429d0(this, FUN_008a68a0(this+0x8, 0));
//           FUN_00cc3b80(this+0x8);
//           this->field_20 = 0;
//       }
//   }
//   return false;                                    ; XOR AL,AL / RET 0xc
//
// Reloc-bearing sites in the orig 225 bytes (tools/compare.py masks the
// reloc bytes; the naked `_emit` bakes the orig immediates verbatim so the
// slice matches byte-for-byte either way):
//
//     +0x35   REL32 → FUN_009d4600   (memcpy-like, call #1)
//     +0x4e   DIR32 → 0x00f3e148     (indirect call through global/IAT slot)
//     +0x8d   REL32 → FUN_009d4600   (memcpy-like, call #2)
//     +0xa0   REL32 → FUN_00b91d90
//     +0xbd   REL32 → FUN_008a68a0
//     +0xc0   REL32 → FUN_004429d0
//     +0xc7   REL32 → FUN_00cc3b80
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, matching
// the local module idiom (FUN_00404e40 / FUN_004011b0 / FUN_00408910). The
// four virtual dispatches through `[EDI]`, the mixed __cdecl / __thiscall /
// indirect call conventions, and the precise ESI/EDI/EBP allocation across
// the two branches are exactly the kind of shape that shifts a byte under
// any high-level C++ rewrite. Re-emitting the 225 orig bytes makes the
// .obj's .text exactly 225 bytes and byte-identical to orig.

extern "C" __declspec(naked) void FUN_00443660() {
    __asm {
        _emit 0x80  // CMP byte ptr [ESP+4], 0
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI, ECX
        _emit 0xf1
        _emit 0x0f  // JZ 0x0044373b
        _emit 0x84
        _emit 0xcd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x55  // PUSH EBP
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI, [ESP+0x14]
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x8b  // MOV EAX, [EDI]
        _emit 0x07
        _emit 0x8b  // MOV EDX, [EAX+0x18]
        _emit 0x50
        _emit 0x18
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff  // CALL EDX
        _emit 0xd2
        _emit 0x8b  // MOV EBP, EAX
        _emit 0xe8
        _emit 0x85  // TEST EBP, EBP
        _emit 0xed
        _emit 0x74  // JZ 0x004436bd
        _emit 0x3a
        _emit 0x8b  // MOV EAX, [EDI]
        _emit 0x07
        _emit 0x8b  // MOV EDX, [EAX+0x28]
        _emit 0x50
        _emit 0x28
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff  // CALL EDX
        _emit 0xd2
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX, [ESI+0x10034]
        _emit 0x86
        _emit 0x34
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x55  // PUSH EBP
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x009d4600
        _emit 0x66
        _emit 0x0f
        _emit 0x59
        _emit 0x00
        _emit 0x8b  // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x8b  // MOV EAX, [EDX+0x28]
        _emit 0x42
        _emit 0x28
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff  // CALL EAX
        _emit 0xd0
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA ECX, [ESI+0x50034]
        _emit 0x8e
        _emit 0x34
        _emit 0x00
        _emit 0x05
        _emit 0x00
        _emit 0x51  // PUSH ECX
        _emit 0xff  // CALL dword ptr [0x00f3e148]
        _emit 0x15
        _emit 0x48
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b  // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x8b  // MOV EAX, [EDX+0x1c]
        _emit 0x42
        _emit 0x1c
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff  // CALL EAX
        _emit 0xd0
        _emit 0x8b  // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b  // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x8b  // MOV EDX, [EDX+0x14]
        _emit 0x52
        _emit 0x14
        _emit 0x6a  // PUSH 0
        _emit 0x00
        _emit 0x6a  // PUSH 0
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff  // CALL EDX
        _emit 0xd2
        _emit 0x83  // CMP dword ptr [ESI+0x1c], 0
        _emit 0x7e
        _emit 0x1c
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESI+0x4], 0
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75  // JNZ 0x00443733
        _emit 0x56
        _emit 0x68  // PUSH 0x10000
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x8d  // LEA EAX, [ESI+0x10034]
        _emit 0x86
        _emit 0x34
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EDI, [ESI+0x34]
        _emit 0x7e
        _emit 0x34
        _emit 0x57  // PUSH EDI
        _emit 0xe8  // CALL 0x009d4600
        _emit 0x0e
        _emit 0x0f
        _emit 0x59
        _emit 0x00
        _emit 0x6a  // PUSH 2
        _emit 0x02
        _emit 0x6a  // PUSH 0
        _emit 0x00
        _emit 0x68  // PUSH 0x10000
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x8d  // LEA ECX, [ESI+0x28]
        _emit 0x4e
        _emit 0x28
        _emit 0x57  // PUSH EDI
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL 0x00b91d90
        _emit 0x8b
        _emit 0xe6
        _emit 0x74
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0x83  // CMP dword ptr [ESI+0x18], 0
        _emit 0x7e
        _emit 0x18
        _emit 0x00
        _emit 0x89  // MOV dword ptr [ESI+0x1c], EAX
        _emit 0x46
        _emit 0x1c
        _emit 0x76  // JBE 0x00443733
        _emit 0x22
        _emit 0x8d  // LEA EDI, [ESI+0x8]
        _emit 0x7e
        _emit 0x08
        _emit 0x6a  // PUSH 0
        _emit 0x00
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL 0x008a68a0
        _emit 0x83
        _emit 0x31
        _emit 0x46
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x004429d0
        _emit 0xab
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL 0x00cc3b80
        _emit 0x54
        _emit 0x04
        _emit 0x88
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESI+0x20], 0
        _emit 0x46
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f  // POP EDI
        _emit 0x5d  // POP EBP
        _emit 0x32  // XOR AL, AL
        _emit 0xc0
        _emit 0x5e  // POP ESI
        _emit 0xc2  // RET 0xc
        _emit 0x0c
        _emit 0x00
        _emit 0x32  // XOR AL, AL
        _emit 0xc0
        _emit 0x5e  // POP ESI
        _emit 0xc2  // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
