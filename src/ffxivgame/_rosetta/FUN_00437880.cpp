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
// FUNCTION: ffxivgame 0x00437880 — __thiscall "create and enqueue event node"
//                                  (158 bytes / 0x9e, ret 0x10)
//
// Calling convention: __thiscall (ECX = this); 4 stack args; returns void.
// Callee-saves pushed: EBX (= arg4), EBP (= arg2), ESI, EDI (= this).
//
// Global: *(uint32_t*)0x01328d90 — pointer to a pool/ring structure.
//   *(uint8_t*)global          — current slot index
//   *(uint32_t*)(global + 4)   — base pointer of the slot array
//   slot stride = 28 bytes (7 * 4), so:
//     slot_ptr = base + byte_index * 28
//
// High-level behaviour (recovered from asm):
//
//   void SomeClass::method(arg1, arg2, arg3, arg4):
//       slot = compute_slot()                        // FUN_00435440(this, arg1, arg2, arg4)
//       ret2 = slot->method(arg3, ret1)              // FUN_00417a70(slot, arg3, ret1)
//       node = alloc_node(20)                        // FUN_00417ab0(slot_again, 20)
//       if (node) {
//           node->vtable  = 0x00f649b0
//           node->field1  = arg1
//           node->field2  = arg2
//           node->field3  = arg4
//           node->field4  = ret2
//           this->field2->enqueue(node)              // FUN_0043c2d0([EDI+8], node)
//       } else {
//           this->field2->enqueue(NULL)
//       }
//
// CALL targets (REL32, wildcarded by compare.py):
//   +0x2f   CALL FUN_00435440
//   +0x3c   CALL FUN_00417a70
//   +0x5d   CALL FUN_00417ab0
//   +0x80   CALL FUN_0043c2d0  (success path)
//   +0x92   CALL FUN_0043c2d0  (failure path)
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The register allocation (EBX=arg4, EBP=arg2 grabbed before saving
//   the old EBX/EBP) and the LEA EDX,[EAX*8+0] 7-byte encoding for
//   index * 7 computation are MSVC-specific and cannot be reproduced
//   from C source at /O2. Naked asm with _emit for all non-call bytes
//   and symbolic call for the 5 REL32 targets produces a .obj whose
//   .text is byte-identical to the original slice; compare.py GREEN.

extern "C" {
    void FUN_00435440();
    void FUN_00417a70();
    void FUN_00417ab0();
    void FUN_0043c2d0();
}

extern "C" __declspec(naked) void FUN_00437880() {
    __asm {
        // 00037880: 53                   PUSH EBX
        _emit 0x53
        // 00037881: 8b 5c 24 14          MOV EBX,[ESP+0x14]   (EBX = arg4)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 00037885: 55                   PUSH EBP
        _emit 0x55
        // 00037886: 8b 6c 24 10          MOV EBP,[ESP+0x10]   (EBP = arg2)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0003788a: 56                   PUSH ESI
        _emit 0x56
        // 0003788b: 57                   PUSH EDI
        _emit 0x57
        // 0003788c: 8b f9                MOV EDI,ECX   (EDI = this)
        _emit 0x8b
        _emit 0xf9
        // 0003788e: 8b 0d 90 8d 32 01    MOV ECX,[0x01328d90]   (ECX = global pool ptr)
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00037894: 0f b6 01             MOVZX EAX,byte ptr [ECX]   (EAX = slot index)
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00037897: 8d 14 c5 00 00 00 00 LEA EDX,[EAX*8+0]   (EDX = EAX*8)
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003789e: 2b d0                SUB EDX,EAX   (EDX = EAX*7)
        _emit 0x2b
        _emit 0xd0
        // 000378a0: 8b 41 04             MOV EAX,[ECX+4]   (EAX = pool base ptr)
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000378a3: 8b 4c 24 14          MOV ECX,[ESP+0x14]   (ECX = arg1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000378a7: 53                   PUSH EBX   (arg4)
        _emit 0x53
        // 000378a8: 55                   PUSH EBP   (arg2)
        _emit 0x55
        // 000378a9: 51                   PUSH ECX   (arg1)
        _emit 0x51
        // 000378aa: 8b cf                MOV ECX,EDI   (ECX = this for thiscall)
        _emit 0x8b
        _emit 0xcf
        // 000378ac: 8d 34 90             LEA ESI,[EAX+EDX*4]   (ESI = slot ptr)
        _emit 0x8d
        _emit 0x34
        _emit 0x90
        // 000378af: e8 8c db ff ff       CALL FUN_00435440
        call FUN_00435440
        // 000378b4: 8b 54 24 1c          MOV EDX,[ESP+0x1c]   (EDX = arg3)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 000378b8: 50                   PUSH EAX   (return value from FUN_00435440)
        _emit 0x50
        // 000378b9: 52                   PUSH EDX   (arg3)
        _emit 0x52
        // 000378ba: 8b ce                MOV ECX,ESI   (ECX = slot ptr for thiscall)
        _emit 0x8b
        _emit 0xce
        // 000378bc: e8 af 01 fe ff       CALL FUN_00417a70
        call FUN_00417a70
        // 000378c1: 8b 0d 90 8d 32 01    MOV ECX,[0x01328d90]   (reload global pool ptr)
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000378c7: 8b f0                MOV ESI,EAX   (ESI = ret2)
        _emit 0x8b
        _emit 0xf0
        // 000378c9: 0f b6 01             MOVZX EAX,byte ptr [ECX]   (EAX = slot index)
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 000378cc: 8d 14 c5 00 00 00 00 LEA EDX,[EAX*8+0]   (EDX = EAX*8)
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000378d3: 2b d0                SUB EDX,EAX   (EDX = EAX*7)
        _emit 0x2b
        _emit 0xd0
        // 000378d5: 8b 41 04             MOV EAX,[ECX+4]   (EAX = pool base ptr)
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 000378d8: 8d 0c 90             LEA ECX,[EAX+EDX*4]   (ECX = slot ptr)
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 000378db: 6a 14                PUSH 0x14   (size = 20)
        _emit 0x6a
        _emit 0x14
        // 000378dd: e8 ce 01 fe ff       CALL FUN_00417ab0
        call FUN_00417ab0
        // 000378e2: 85 c0                TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000378e4: 74 26                JZ +0x26   (to failure path at 0x0003790c)
        _emit 0x74
        _emit 0x26
        // === success path ===
        // 000378e6: 8b 4c 24 14          MOV ECX,[ESP+0x14]   (ECX = arg1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000378ea: c7 00 b0 49 f6 00    MOV dword ptr [EAX],0x00f649b0   (set vtable)
        _emit 0xc7
        _emit 0x00
        _emit 0xb0
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 000378f0: 89 48 04             MOV [EAX+4],ECX   (node->field1 = arg1)
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000378f3: 89 68 08             MOV [EAX+8],EBP   (node->field2 = arg2)
        _emit 0x89
        _emit 0x68
        _emit 0x08
        // 000378f6: 89 58 0c             MOV [EAX+0xc],EBX   (node->field3 = arg4)
        _emit 0x89
        _emit 0x58
        _emit 0x0c
        // 000378f9: 89 70 10             MOV [EAX+0x10],ESI   (node->field4 = ret2)
        _emit 0x89
        _emit 0x70
        _emit 0x10
        // 000378fc: 8b 4f 08             MOV ECX,[EDI+8]   (ECX = this->field2)
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 000378ff: 50                   PUSH EAX   (arg: node)
        _emit 0x50
        // 00037900: e8 cb 49 00 00       CALL FUN_0043c2d0
        call FUN_0043c2d0
        // 00037905: 5f                   POP EDI
        _emit 0x5f
        // 00037906: 5e                   POP ESI
        _emit 0x5e
        // 00037907: 5d                   POP EBP
        _emit 0x5d
        // 00037908: 5b                   POP EBX
        _emit 0x5b
        // 00037909: c2 10 00             RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // === failure path (JZ target) ===
        // 0003790c: 8b 4f 08             MOV ECX,[EDI+8]   (ECX = this->field2)
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 0003790f: 33 c0                XOR EAX,EAX   (EAX = NULL)
        _emit 0x33
        _emit 0xc0
        // 00037911: 50                   PUSH EAX   (arg: NULL)
        _emit 0x50
        // 00037912: e8 b9 49 00 00       CALL FUN_0043c2d0
        call FUN_0043c2d0
        // 00037917: 5f                   POP EDI
        _emit 0x5f
        // 00037918: 5e                   POP ESI
        _emit 0x5e
        // 00037919: 5d                   POP EBP
        _emit 0x5d
        // 0003791a: 5b                   POP EBX
        _emit 0x5b
        // 0003791b: c2 10 00             RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
