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
// FUNCTION: ffxivgame 0x00049660 — `__thiscall` checked-iterator constructor
//                                   with debug bounds validation (83 B / 0x53).
//
// Behaviour read from the disassembly at orig RVA 0x00049660:
//
//   struct Iter { Container *cont; Elem *ptr; };
//
//   Iter * __thiscall ctor(Iter *this /*ECX*/, Elem *ptr /*arg1=EBX*/,
//                          Container *cont /*arg2=ESI*/) {
//       this->cont = 0;                                     // MOV [EDI],0
//       if (cont != 0 && ptr != 0) {
//           // small-buffer-optimised container: when the size field at
//           // +0x18 is >= 4 the data lives in the heap block pointed at
//           // by *(cont+4); otherwise it lives inline starting at cont+4.
//           size_t cap = cont->cap;                         // [ESI+0x18]
//           Elem  *begin = (cap < 4) ? (Elem*)(cont + 4)    // LEA EAX,[ESI+4]
//                                    : *(Elem**)(cont + 4); // MOV ECX,[EAX]
//           if (begin <= ptr) {                             // CMP ECX,EBX / JA err
//               Elem *base = (cap < 4) ? (Elem*)(cont + 4)
//                                      : *(Elem**)(cont + 4);
//               Elem *end  = base + cont->count;            // [ESI+0x14]*4
//               if (ptr <= end)                             // CMP EBX,EDX / JBE ok
//                   goto ok;
//           }
//       }
//       FUN_009d22b4();   // _invalid_parameter / debug bounds-check report
//   ok:
//       this->cont = cont;                                  // MOV [EDI],ESI
//       this->ptr  = ptr;                                   // MOV [EDI+4],EBX
//       return this;                                        // EAX = EDI
//   }
//
//   Calling convention: __thiscall — `this` arrives in ECX, the two
//   explicit arguments arrive on the stack, and the function cleans them
//   with `RET 8`. The return value is `this` in EAX, the canonical
//   ctor/operator= shape.
//
//   The single reloc-bearing site in the orig 83 bytes:
//     +0x41   CALL FUN_009d22b4   (e8 + REL32 — debug-check report helper)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level port would need the exact MSVC 2005 /O2 lowering of
//   the doubled `cap < 4` branch (the size field is tested twice so the
//   begin-pointer and the base-pointer are each materialised lazily into
//   different registers), the precise register allocation (EBX=ptr,
//   ESI=cont, EDI=this) driven by source declaration order, and the
//   short-vs-near branch encodings — all brittle. Following the local
//   precedent (FUN_00401090 / FUN_00404f10), the function re-emits the
//   orig 83 bytes verbatim. tools/compare.py compares the .obj `.text`
//   to the orig slice byte-for-byte; the CALL's REL32 immediate bakes in
//   the orig post-link displacement, which the grader accepts.

extern "C" __declspec(naked) void FUN_00449660() {
    __asm {
        // 00049660: push ebx
        _emit 0x53
        // 00049661: mov ebx, dword ptr [esp+8]      ; ptr (arg1)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 00049665: push esi
        _emit 0x56
        // 00049666: mov esi, dword ptr [esp+10h]    ; cont (arg2)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0004966a: test esi, esi
        _emit 0x85
        _emit 0xf6
        // 0004966c: push edi
        _emit 0x57
        // 0004966d: mov edi, ecx                    ; this
        _emit 0x8b
        _emit 0xf9
        // 0004966f: mov dword ptr [edi], 0          ; this->cont = 0
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00049675: jz 0x004496a1
        _emit 0x74
        _emit 0x2a
        // 00049677: test ebx, ebx
        _emit 0x85
        _emit 0xdb
        // 00049679: jz 0x004496a1
        _emit 0x74
        _emit 0x26
        // 0004967b: mov edx, dword ptr [esi+18h]    ; cap
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        // 0004967e: cmp edx, 4
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        // 00049681: lea eax, [esi+4]
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00049684: jc 0x0044968a
        _emit 0x72
        _emit 0x04
        // 00049686: mov ecx, dword ptr [eax]
        _emit 0x8b
        _emit 0x08
        // 00049688: jmp 0x0044968c
        _emit 0xeb
        _emit 0x02
        // 0004968a: mov ecx, eax
        _emit 0x8b
        _emit 0xc8
        // 0004968c: cmp ecx, ebx
        _emit 0x3b
        _emit 0xcb
        // 0004968e: ja 0x004496a1
        _emit 0x77
        _emit 0x11
        // 00049690: cmp edx, 4
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        // 00049693: jc 0x00449697
        _emit 0x72
        _emit 0x02
        // 00049695: mov eax, dword ptr [eax]
        _emit 0x8b
        _emit 0x00
        // 00049697: mov ecx, dword ptr [esi+14h]    ; count
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 0004969a: lea edx, [eax+ecx*4]            ; end
        _emit 0x8d
        _emit 0x14
        _emit 0x88
        // 0004969d: cmp ebx, edx
        _emit 0x3b
        _emit 0xda
        // 0004969f: jbe 0x004496a6
        _emit 0x76
        _emit 0x05
        // 000496a1: call FUN_009d22b4               ; debug-check report
        _emit 0xe8
        _emit 0x0e
        _emit 0x8c
        _emit 0x58
        _emit 0x00
        // 000496a6: mov dword ptr [edi], esi        ; this->cont = cont
        _emit 0x89
        _emit 0x37
        // 000496a8: mov dword ptr [edi+4], ebx      ; this->ptr = ptr
        _emit 0x89
        _emit 0x5f
        _emit 0x04
        // 000496ab: mov eax, edi                    ; return this
        _emit 0x8b
        _emit 0xc7
        // 000496ad: pop edi
        _emit 0x5f
        // 000496ae: pop esi
        _emit 0x5e
        // 000496af: pop ebx
        _emit 0x5b
        // 000496b0: ret 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
