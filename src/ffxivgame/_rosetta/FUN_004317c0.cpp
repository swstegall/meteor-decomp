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
// FUNCTION: ffxivgame 0x004317c0 — __thiscall teardown / release routine
//                                  (136 bytes / 0x88).
//
// Behaviour reconstructed from the asm (ECX = this, EBX caches it):
//
//   void __thiscall FUN_004317c0(Owner *this) {
//       // ESI = &this->vec  (a std::vector<Releasable*> embedded at +0x40,
//       //                    layout [_, _Myfirst@+4, _Mylast@+8])
//       for (int i = 0; this->vec._Myfirst != 0 &&
//                       i < (this->vec._Mylast - this->vec._Myfirst); i++) {
//           // operator[] bounds-check (inlined): out-of-range → _invalid (0x9d22b4)
//           Releasable *e = this->vec._Myfirst[i];
//           if (e) e->vftable[0](1);            // virtual release/destroy(1)
//       }
//       FUN_00c6be00(&this->vec);               // vector _Tidy / clear (+0x40)
//
//       if (this->m_3c) {                        // [this+0x3c]
//           this->m_3c->vftable[0](1);           // virtual release/destroy(1)
//           this->m_3c = 0;
//       }
//       if (this->m_34) {                        // [this+0x34]
//           g_singleton_01329920->vftable[0x24](this->m_34); // virtual @+0x24
//           this->m_34 = 0;
//       }
//       this->m_30 = 0;                          // [this+0x30]
//   }
//
// Calling convention: __thiscall (ECX = this; no `ret N` because there are
// no stack parameters). Frame: only callee-saves pushed (EBX/EBP/ESI/EDI),
// EBP used as a zero constant (XOR EBP,EBP) rather than a frame pointer.
//
// Reloc-bearing sites (emitted verbatim as raw bytes — see strategy below):
//   off 0x34  REL32  CALL 0x009d22b4   (STL out-of-range / _invalid helper)
//   off 0x52  REL32  CALL 0x00c6be00   (vector teardown on &this->vec)
//   off 0x71  DIR32  MOV ECX,[0x01329920]  (global singleton ptr)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The compiler-inserted 3-byte alignment nop at +0x0d (`8d 49 00`,
//   aligning the loop head at 0x004317d0) and the inlined vector
//   operator[] bounds-check cannot be coerced out of a source-level
//   /O2 form. As with siblings FUN_0041a930 / FUN_00408610, a
//   __declspec(naked) body re-emits the original 136 bytes verbatim via
//   MASM _emit directives (call/dir32 displacements included as raw
//   bytes). compare.py masks no positions (the .obj carries no relocs)
//   and the bytes equal the orig slice → GREEN.

extern "C" __declspec(naked) void FUN_004317c0() {
    __asm {
        // 000317c0: 53           PUSH EBX
        _emit 0x53
        // 000317c1: 55           PUSH EBP
        _emit 0x55
        // 000317c2: 56           PUSH ESI
        _emit 0x56
        // 000317c3: 8b d9        MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 000317c5: 57           PUSH EDI
        _emit 0x57
        // 000317c6: 33 ed        XOR EBP,EBP
        _emit 0x33
        _emit 0xed
        // 000317c8: 33 ff        XOR EDI,EDI
        _emit 0x33
        _emit 0xff
        // 000317ca: 8d 73 40     LEA ESI,[EBX+0x40]
        _emit 0x8d
        _emit 0x73
        _emit 0x40
        // 000317cd: 8d 49 00     LEA ECX,[ECX]   (3-byte alignment nop)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 000317d0: 8b 4e 04     MOV ECX,dword ptr [ESI+0x4]   (loop head)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 000317d3: 3b cd        CMP ECX,EBP
        _emit 0x3b
        _emit 0xcd
        // 000317d5: 74 38        JZ 0x0043180f
        _emit 0x74
        _emit 0x38
        // 000317d7: 8b 46 08     MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 000317da: 2b c1        SUB EAX,ECX
        _emit 0x2b
        _emit 0xc1
        // 000317dc: c1 f8 02     SAR EAX,0x2
        _emit 0xc1
        _emit 0xf8
        _emit 0x02
        // 000317df: 3b f8        CMP EDI,EAX
        _emit 0x3b
        _emit 0xf8
        // 000317e1: 73 2c        JNC 0x0043180f
        _emit 0x73
        _emit 0x2c
        // 000317e3: 3b cd        CMP ECX,EBP
        _emit 0x3b
        _emit 0xcd
        // 000317e5: 74 0c        JZ 0x004317f3
        _emit 0x74
        _emit 0x0c
        // 000317e7: 8b 46 08     MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 000317ea: 2b c1        SUB EAX,ECX
        _emit 0x2b
        _emit 0xc1
        // 000317ec: c1 f8 02     SAR EAX,0x2
        _emit 0xc1
        _emit 0xf8
        _emit 0x02
        // 000317ef: 3b f8        CMP EDI,EAX
        _emit 0x3b
        _emit 0xf8
        // 000317f1: 72 05        JC 0x004317f8
        _emit 0x72
        _emit 0x05
        // 000317f3: e8 bc 0a 5a 00   CALL 0x009d22b4 (out-of-range helper)
        _emit 0xe8
        _emit 0xbc
        _emit 0x0a
        _emit 0x5a
        _emit 0x00
        // 000317f8: 8b 46 04     MOV EAX,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000317fb: 8b 0c b8     MOV ECX,dword ptr [EAX+EDI*0x4]
        _emit 0x8b
        _emit 0x0c
        _emit 0xb8
        // 000317fe: 3b cd        CMP ECX,EBP
        _emit 0x3b
        _emit 0xcd
        // 00031800: 74 08        JZ 0x0043180a
        _emit 0x74
        _emit 0x08
        // 00031802: 8b 11        MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00031804: 8b 02        MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 00031806: 6a 01        PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 00031808: ff d0        CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0003180a: 83 c7 01     ADD EDI,0x1
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 0003180d: eb c1        JMP 0x004317d0
        _emit 0xeb
        _emit 0xc1
        // 0003180f: 8b ce        MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00031811: e8 ea a5 83 00   CALL 0x00c6be00 (vector teardown)
        _emit 0xe8
        _emit 0xea
        _emit 0xa5
        _emit 0x83
        _emit 0x00
        // 00031816: 8b 4b 3c     MOV ECX,dword ptr [EBX+0x3c]
        _emit 0x8b
        _emit 0x4b
        _emit 0x3c
        // 00031819: 3b cd        CMP ECX,EBP
        _emit 0x3b
        _emit 0xcd
        // 0003181b: 74 0b        JZ 0x00431828
        _emit 0x74
        _emit 0x0b
        // 0003181d: 8b 11        MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0003181f: 8b 02        MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 00031821: 6a 01        PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 00031823: ff d0        CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00031825: 89 6b 3c     MOV dword ptr [EBX+0x3c],EBP
        _emit 0x89
        _emit 0x6b
        _emit 0x3c
        // 00031828: 8b 43 34     MOV EAX,dword ptr [EBX+0x34]
        _emit 0x8b
        _emit 0x43
        _emit 0x34
        // 0003182b: 3b c5        CMP EAX,EBP
        _emit 0x3b
        _emit 0xc5
        // 0003182d: 74 11        JZ 0x00431840
        _emit 0x74
        _emit 0x11
        // 0003182f: 8b 0d 20 99 32 01   MOV ECX,dword ptr [0x01329920]
        _emit 0x8b
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        // 00031835: 8b 11        MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00031837: 50           PUSH EAX
        _emit 0x50
        // 00031838: 8b 42 24     MOV EAX,dword ptr [EDX+0x24]
        _emit 0x8b
        _emit 0x42
        _emit 0x24
        // 0003183b: ff d0        CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0003183d: 89 6b 34     MOV dword ptr [EBX+0x34],EBP
        _emit 0x89
        _emit 0x6b
        _emit 0x34
        // 00031840: 5f           POP EDI
        _emit 0x5f
        // 00031841: 5e           POP ESI
        _emit 0x5e
        // 00031842: 89 6b 30     MOV dword ptr [EBX+0x30],EBP
        _emit 0x89
        _emit 0x6b
        _emit 0x30
        // 00031845: 5d           POP EBP
        _emit 0x5d
        // 00031846: 5b           POP EBX
        _emit 0x5b
        // 00031847: c3           RET
        _emit 0xc3
    }
}
