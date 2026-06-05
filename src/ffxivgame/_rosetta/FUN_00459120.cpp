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
// FUNCTION: ffxivgame 0x00059120 — __thiscall COM-style "open two interfaces"
//                                  bootstrap (153 B / 0x99, ret 4, returns bool).
//
// Calling convention: __thiscall (ECX = this); one stack arg (a source
//   object pointer); cleans 4 bytes on return (`ret 4`); result in AL (bool).
//
// Object layout (offsets touched on `this` = ESI):
//   [this + 0x04]   out-param A (filled by the second vcall)
//   [this + 0x14]   interface slot (queried/released; nulled on failure)
//   [this + 0x1c]   out-param B (set to -1 on the failure branch)
//
// Behaviour (recovered from asm @ 0x00059120):
//
//   bool Bootstrap(SourceObj *src) {
//       this->PreInit();                              // FUN_00458d70 (thiscall)
//       bool ok = false;
//       // src->vtbl[6](&this->iface) — open the first interface into +0x14
//       if (src->vtbl[6](&this->iface) >= 0) {
//           if (this->iface == nullptr) return true;  // nothing to do
//           IFace *q = nullptr;
//           // iface->QueryInterface(IID_A, &q)
//           if (this->iface->vtbl[0](&IID_A, &q) >= 0) {
//               // q->vtbl[3](&IID_B, &this->fieldB, &this->outC)
//               if (q->vtbl[3](&IID_B, &this->fieldB, &this->outC) >= 0)
//                   ok = true;
//               else
//                   this->outC = -1;
//               q->vtbl[2](q);                         // Release q
//               if (ok) goto done;
//           }
//       }
//       if (this->iface) {                             // failure cleanup
//           this->iface->vtbl[2](this->iface);         // Release iface
//           this->iface = nullptr;
//       }
//   done:
//       return ok;
//   }
//
// CALL targets:
//   +0x05   CALL FUN_00458d70   (REL32 — masked by compare.py)
//   all others are virtual-dispatch indirect calls (CALL EDX / CALL EAX).
//   The two PUSHes of absolute VAs (0x011088c0, 0x01108600) are IID GUID
//   pointers in .rdata; emitted as literal immediates (their .text bytes
//   are identical regardless of the separate base-reloc record).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The mixed COM vtable dispatch, the BL boolean carried across two
//   cleanup paths, and the EBP push/pop that only happens on the success
//   branch don't lower cleanly from /O2 C++ in an isolated TU (the
//   register allocator picks a different callee-saved set than the
//   full-binary build). Per the ffxivgame convention, re-emit the
//   original 153 bytes verbatim; the only reloc window is the single
//   REL32 direct call, which compare.py masks.

// Sibling called via direct CALL (e8 + REL32 COFF relocation).
extern "C" int FUN_00458d70();   // __thiscall void PreInit(this)

extern "C" __declspec(naked) void FUN_00459120() {
    __asm {
        // 00059120: 53                   PUSH EBX
        _emit 0x53
        // 00059121: 56                   PUSH ESI
        _emit 0x56
        // 00059122: 57                   PUSH EDI
        _emit 0x57
        // 00059123: 8b f1                MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00059125: e8 46 fc ff ff       CALL FUN_00458d70 (REL32 reloc — masked)
        call FUN_00458d70
        // 0005912a: 8b 44 24 10          MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0005912e: 8b 08                MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 00059130: 8b 51 18             MOV EDX,[ECX+0x18]
        _emit 0x8b
        _emit 0x51
        _emit 0x18
        // 00059133: 8d 7e 14             LEA EDI,[ESI+0x14]
        _emit 0x8d
        _emit 0x7e
        _emit 0x14
        // 00059136: 57                   PUSH EDI
        _emit 0x57
        // 00059137: 50                   PUSH EAX
        _emit 0x50
        // 00059138: 32 db                XOR BL,BL
        _emit 0x32
        _emit 0xdb
        // 0005913a: ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0005913c: 85 c0                TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005913e: 7c 55                JL +0x55 -> 0x00059195
        _emit 0x7c
        _emit 0x55
        // 00059140: 8b 07                MOV EAX,[EDI]
        _emit 0x8b
        _emit 0x07
        // 00059142: 85 c0                TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00059144: 74 6b                JZ +0x6b -> 0x000591b1
        _emit 0x74
        _emit 0x6b
        // 00059146: 8b 08                MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 00059148: 8d 54 24 10          LEA EDX,[ESP+0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0005914c: 52                   PUSH EDX
        _emit 0x52
        // 0005914d: 68 c0 88 10 01       PUSH 0x011088c0 (IID_A ptr)
        _emit 0x68
        _emit 0xc0
        _emit 0x88
        _emit 0x10
        _emit 0x01
        // 00059152: 50                   PUSH EAX
        _emit 0x50
        // 00059153: 8b 01                MOV EAX,[ECX]
        _emit 0x8b
        _emit 0x01
        // 00059155: ff d0                CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00059157: 85 c0                TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00059159: 7c 3a                JL +0x3a -> 0x00059195
        _emit 0x7c
        _emit 0x3a
        // 0005915b: 8b 44 24 10          MOV EAX,[ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0005915f: 8b 08                MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 00059161: 8b 51 0c             MOV EDX,[ECX+0xc]
        _emit 0x8b
        _emit 0x51
        _emit 0x0c
        // 00059164: 55                   PUSH EBP
        _emit 0x55
        // 00059165: 8d 6e 1c             LEA EBP,[ESI+0x1c]
        _emit 0x8d
        _emit 0x6e
        _emit 0x1c
        // 00059168: 55                   PUSH EBP
        _emit 0x55
        // 00059169: 83 c6 04             ADD ESI,0x4
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        // 0005916c: 56                   PUSH ESI
        _emit 0x56
        // 0005916d: 68 00 86 10 01       PUSH 0x01108600 (IID_B ptr)
        _emit 0x68
        _emit 0x00
        _emit 0x86
        _emit 0x10
        _emit 0x01
        // 00059172: 50                   PUSH EAX
        _emit 0x50
        // 00059173: ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00059175: 85 c0                TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00059177: 7c 04                JL +0x04 -> 0x0005917d
        _emit 0x7c
        _emit 0x04
        // 00059179: b3 01                MOV BL,0x1
        _emit 0xb3
        _emit 0x01
        // 0005917b: eb 07                JMP +0x07 -> 0x00059184
        _emit 0xeb
        _emit 0x07
        // 0005917d: c7 45 00 ff ff ff ff MOV [EBP],0xffffffff
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00059184: 8b 44 24 14          MOV EAX,[ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00059188: 8b 08                MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 0005918a: 8b 51 08             MOV EDX,[ECX+0x8]
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 0005918d: 50                   PUSH EAX
        _emit 0x50
        // 0005918e: ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00059190: 84 db                TEST BL,BL
        _emit 0x84
        _emit 0xdb
        // 00059192: 5d                   POP EBP
        _emit 0x5d
        // 00059193: 75 14                JNZ +0x14 -> 0x000591a9
        _emit 0x75
        _emit 0x14
        // 00059195: 8b 07                MOV EAX,[EDI]
        _emit 0x8b
        _emit 0x07
        // 00059197: 85 c0                TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00059199: 74 0e                JZ +0x0e -> 0x000591a9
        _emit 0x74
        _emit 0x0e
        // 0005919b: 8b 08                MOV ECX,[EAX]
        _emit 0x8b
        _emit 0x08
        // 0005919d: 8b 51 08             MOV EDX,[ECX+0x8]
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 000591a0: 50                   PUSH EAX
        _emit 0x50
        // 000591a1: ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000591a3: c7 07 00 00 00 00    MOV [EDI],0x0
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000591a9: 5f                   POP EDI
        _emit 0x5f
        // 000591aa: 5e                   POP ESI
        _emit 0x5e
        // 000591ab: 8a c3                MOV AL,BL
        _emit 0x8a
        _emit 0xc3
        // 000591ad: 5b                   POP EBX
        _emit 0x5b
        // 000591ae: c2 04 00             RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 000591b1: 5f                   POP EDI
        _emit 0x5f
        // 000591b2: 5e                   POP ESI
        _emit 0x5e
        // 000591b3: b0 01                MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 000591b5: 5b                   POP EBX
        _emit 0x5b
        // 000591b6: c2 04 00             RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
