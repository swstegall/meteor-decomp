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
// FUNCTION: ffxivgame 0x00447010 — __thiscall growable-buffer "resize"
//                                  helper (153 B / 0x99, ret 8).
//
// __thiscall void Resize(this, unsigned int newSize, char clearFlag):
//   ECX = this; two stack args (newSize, clearFlag); cleans 8 bytes
//   on return (`ret 8`).
//
// Object layout (offsets touched):
//   [this + 0x00]  void *data;       // backing storage
//   [this + 0x04]  unsigned capacity;// allocated bytes (32-aligned)
//   [this + 0x08]  unsigned size;    // current element/byte count
//   [this + 0x10]  char  flagA;      // cleared when clearFlag != 0
//   [this + 0x11]  char  flagB;      // ownership flag (free old buf when 0)
//
// Behaviour (recovered from asm @ 0x00447010):
//
//   void Resize(unsigned int newSize, char clearFlag) {
//       if (clearFlag) this->flagA = 0;
//       if (newSize <= this->capacity) {        // JBE (unsigned)
//           this->size = newSize;
//           return;
//       }
//       unsigned oldSize = this->size;
//       void *oldData    = this->data;
//       unsigned newCap  = (newSize + 0x1f) & ~0x1fu;
//       bool owned       = (this->flagB == 0);  // SETZ
//       void *buf = FUN_0044d500(newCap, 0, 0xb);   // allocator (3 args)
//       if (oldData) {
//           FUN_009d5110(buf, oldData, oldSize);    // memcpy
//           if (owned)
//               FUN_0044d350(oldData, this->capacity, 0xb);  // free
//       }
//       this->data     = buf;
//       this->capacity = newCap;
//       this->flagB    = 0;
//       this->size     = newSize;
//   }
//
// CALL targets (all REL32, wildcarded by tools/compare.py):
//   +0x40   CALL FUN_0044d500   — 3-arg allocator(size, 0, 0xb)
//   +0x55   CALL FUN_009d5110   — memcpy(dst, src, n)
//   +0x6c   CALL FUN_0044d350   — 3-arg free(ptr, size, 0xb)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   MSVC 2005's allocator reuses the inbound argument stack slots
//   (newSize at [esp+0x14], clearFlag at [esp+0x10]) as scratch for the
//   SETZ temp and the size reload, and spills capacity / old size into
//   the two locals materialised by `sub esp,8`. Source-level C++ at /O2
//   won't reliably converge on the exact slot assignment in an isolated
//   TU. Per the established ffxivgame rosetta convention, emit the 153
//   original bytes verbatim; the three REL32 windows are masked by
//   compare.py.

extern "C" {
    // Internal direct-call targets within the binary (REL32 relocations).
    int FUN_0044d500();    // 3-arg allocator
    int FUN_009d5110();    // memcpy
    int FUN_0044d350();    // 3-arg free
}

extern "C" __declspec(naked) void FUN_00447010() {
    __asm {
        // 00447010: 83 ec 08              SUB ESP,8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00447013: 80 7c 24 10 00        CMP byte ptr [ESP+0x10],0  (clearFlag)
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x00
        // 00447018: 56                    PUSH ESI
        _emit 0x56
        // 00447019: 57                    PUSH EDI
        _emit 0x57
        // 0044701a: 8b f1                 MOV ESI,ECX   (this)
        _emit 0x8b
        _emit 0xf1
        // 0044701c: 74 04                 JZ 0x447022
        _emit 0x74
        _emit 0x04
        // 0044701e: c6 46 10 00           MOV byte ptr [ESI+0x10],0
        _emit 0xc6
        _emit 0x46
        _emit 0x10
        _emit 0x00
        // 00447022: 8b 46 04              MOV EAX,[ESI+4]   (capacity)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00447025: 8b 7c 24 14           MOV EDI,[ESP+0x14]  (newSize)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 00447029: 3b f8                 CMP EDI,EAX
        _emit 0x3b
        _emit 0xf8
        // 0044702b: 89 44 24 0c           MOV [ESP+0xc],EAX   (save capacity)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0044702f: 76 6d                 JBE 0x44709e
        _emit 0x76
        _emit 0x6d
        // 00447031: 8b 46 08              MOV EAX,[ESI+8]    (old size)
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00447034: 53                    PUSH EBX
        _emit 0x53
        // 00447035: 55                    PUSH EBP
        _emit 0x55
        // 00447036: 8b 2e                 MOV EBP,[ESI]      (old data)
        _emit 0x8b
        _emit 0x2e
        // 00447038: 83 c7 1f              ADD EDI,0x1f
        _emit 0x83
        _emit 0xc7
        _emit 0x1f
        // 0044703b: 6a 0b                 PUSH 0xb
        _emit 0x6a
        _emit 0x0b
        // 0044703d: 83 e7 e0              AND EDI,0xffffffe0
        _emit 0x83
        _emit 0xe7
        _emit 0xe0
        // 00447040: 80 7e 11 00           CMP byte ptr [ESI+0x11],0
        _emit 0x80
        _emit 0x7e
        _emit 0x11
        _emit 0x00
        // 00447044: 6a 00                 PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00447046: 57                    PUSH EDI            (newCap)
        _emit 0x57
        // 00447047: 0f 94 44 24 2c        SETZ byte ptr [ESP+0x2c]  (owned)
        _emit 0x0f
        _emit 0x94
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0044704c: 89 44 24 1c           MOV [ESP+0x1c],EAX  (save old size)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00447050: e8 ?? ?? ?? ??        CALL FUN_0044d500
        call FUN_0044d500
        // 00447055: 83 c4 0c              ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00447058: 85 ed                 TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 0044705a: 8b d8                 MOV EBX,EAX         (buf)
        _emit 0x8b
        _emit 0xd8
        // 0044705c: 74 26                 JZ 0x447084
        _emit 0x74
        _emit 0x26
        // 0044705e: 8b 4c 24 10           MOV ECX,[ESP+0x10]  (old size)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00447062: 51                    PUSH ECX
        _emit 0x51
        // 00447063: 55                    PUSH EBP
        _emit 0x55
        // 00447064: 53                    PUSH EBX
        _emit 0x53
        // 00447065: e8 ?? ?? ?? ??        CALL FUN_009d5110   (memcpy)
        call FUN_009d5110
        // 0044706a: 83 c4 0c              ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0044706d: 80 7c 24 20 00        CMP byte ptr [ESP+0x20],0  (owned)
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x00
        // 00447072: 74 10                 JZ 0x447084
        _emit 0x74
        _emit 0x10
        // 00447074: 8b 54 24 14           MOV EDX,[ESP+0x14]  (capacity)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00447078: 6a 0b                 PUSH 0xb
        _emit 0x6a
        _emit 0x0b
        // 0044707a: 52                    PUSH EDX
        _emit 0x52
        // 0044707b: 55                    PUSH EBP
        _emit 0x55
        // 0044707c: e8 ?? ?? ?? ??        CALL FUN_0044d350   (free)
        call FUN_0044d350
        // 00447081: 83 c4 0c              ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00447084: 8b 44 24 1c           MOV EAX,[ESP+0x1c]  (newSize)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00447088: 5d                    POP EBP
        _emit 0x5d
        // 00447089: 89 1e                 MOV [ESI],EBX
        _emit 0x89
        _emit 0x1e
        // 0044708b: 5b                    POP EBX
        _emit 0x5b
        // 0044708c: 89 7e 04              MOV [ESI+4],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        // 0044708f: 5f                    POP EDI
        _emit 0x5f
        // 00447090: c6 46 11 00           MOV byte ptr [ESI+0x11],0
        _emit 0xc6
        _emit 0x46
        _emit 0x11
        _emit 0x00
        // 00447094: 89 46 08              MOV [ESI+8],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 00447097: 5e                    POP ESI
        _emit 0x5e
        // 00447098: 83 c4 08              ADD ESP,8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0044709b: c2 08 00              RET 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // 0044709e: 89 7e 08              MOV [ESI+8],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x08
        // 004470a1: 5f                    POP EDI
        _emit 0x5f
        // 004470a2: 5e                    POP ESI
        _emit 0x5e
        // 004470a3: 83 c4 08              ADD ESP,8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 004470a6: c2 08 00              RET 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
