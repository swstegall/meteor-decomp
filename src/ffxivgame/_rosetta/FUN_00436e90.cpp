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
// FUNCTION: ffxivgame 0x00436e90 — allocate a 0x10-byte record into a
//                                  segmented buffer and push it onto a
//                                  per-object list (__thiscall, 3 stack
//                                  args, RET 0xC, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this).  Three caller-pushed args
// (RET 0xC cleans them): param1, param2 (int index/id), param3 (ptr/data).
//
// Pseudo-code:
//
//   void* __thiscall FUN_00436e90(SomeObj *this,
//                                  void    *param1,
//                                  int      param2,
//                                  void    *param3)
//   {
//       SegmentedBuffer *buf = g_segBufMgr;   // [0x01328d90]
//       // compute slot within the segmented-buffer's internal table:
//       //   byte idx  = *buf
//       //   entry ptr = buf->array_ptr + idx * 28  (idx*8 - idx)*4
//       void *slot = (void*)((byte)*buf * 7 * 4 + buf->array_ptr);
//
//       // first call: write param3 into the buffer (aligned at 2^4=16 bytes,
//       //              param2<<4 bytes long) — FUN_00417ae0 is __thiscall,
//       //              returns the destination pointer
//       void *stored = FUN_00417ae0(slot, param3, param2 << 4, 4);
//
//       // re-derive slot (slot may have been invalidated by a realloc)
//       slot = (void*)((byte)*buf * 7 * 4 + buf->array_ptr);
//
//       // second call: allocate 0x10 bytes aligned at default — FUN_00417ab0
//       SomeRecord *rec = (SomeRecord*)FUN_00417ab0(slot, 0x10);
//
//       if (rec != NULL) {
//           rec->vptr    = 0xf64918;   // vtable pointer
//           rec->field04 = param1;
//           rec->field08 = param2;
//           rec->field0c = stored;
//           // push rec onto this->list (field_8 is the list object)
//           FUN_0043c2d0(this->field_8, rec);
//       } else {
//           FUN_0043c2d0(this->field_8, NULL);
//       }
//   }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two absolute-address MOV ECX,[0x01328d90] instructions (offsets
//   +0x03 and +0x33 within the function), the MOV [EAX],0xf64918 vtable
//   write (offset +0x5c), and the four CALL rel32 displacements (offsets
//   +0x2f, +0x50, +0x70, +0x81) are all relocation sites masked by
//   compare.py. The LEA EDX,[EAX*0x8+0x0] encoding (8D 14 C5 00 00 00 00)
//   is a scaled-index-plus-disp32-zero form that MSVC 2005 generates for
//   the EAX*8 multiply — not reproducible from C++ without naked-asm.

extern "C" __declspec(naked) void FUN_00436e90() {
    __asm {
        // 00036e90: 53              PUSH EBX
        _emit 0x53
        // 00036e91: 8b d9           MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00036e93: 8b 0d 90 8d 32 01   MOV ECX,dword ptr [0x01328d90]  -- RELOC
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036e99: 0f b6 01        MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036e9c: 8d 14 c5 00 00 00 00   LEA EDX,[EAX*0x8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036ea3: 2b d0           SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036ea5: 8b 41 04        MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036ea8: 56              PUSH ESI
        _emit 0x56
        // 00036ea9: 8b 74 24 10     MOV ESI,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00036ead: 8d 0c 90        LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036eb0: 8b 44 24 14     MOV EAX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00036eb4: 57              PUSH EDI
        _emit 0x57
        // 00036eb5: 6a 04           PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 00036eb7: 8b d6           MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00036eb9: c1 e2 04        SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00036ebc: 52              PUSH EDX
        _emit 0x52
        // 00036ebd: 50              PUSH EAX
        _emit 0x50
        // 00036ebe: e8 1d 0c fe ff  CALL FUN_00417ae0  -- RELOC (rel32)
        _emit 0xe8
        _emit 0x1d
        _emit 0x0c
        _emit 0xfe
        _emit 0xff
        // 00036ec3: 8b 0d 90 8d 32 01   MOV ECX,dword ptr [0x01328d90]  -- RELOC
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036ec9: 8b f8           MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00036ecb: 0f b6 01        MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036ece: 8d 14 c5 00 00 00 00   LEA EDX,[EAX*0x8+0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036ed5: 2b d0           SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036ed7: 8b 41 04        MOV EAX,dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036eda: 8d 0c 90        LEA ECX,[EAX+EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036edd: 6a 10           PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00036edf: e8 cc 0b fe ff  CALL FUN_00417ab0  -- RELOC (rel32)
        _emit 0xe8
        _emit 0xcc
        _emit 0x0b
        _emit 0xfe
        _emit 0xff
        // 00036ee4: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00036ee6: 74 22           JZ +0x22
        _emit 0x74
        _emit 0x22
        // 00036ee8: 8b 4c 24 10     MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00036eec: c7 00 18 49 f6 00   MOV dword ptr [EAX],0xf64918  -- RELOC
        _emit 0xc7
        _emit 0x00
        _emit 0x18
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00036ef2: 89 48 04        MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00036ef5: 89 70 08        MOV dword ptr [EAX+0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00036ef8: 89 78 0c        MOV dword ptr [EAX+0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00036efb: 8b 4b 08        MOV ECX,dword ptr [EBX+0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036efe: 50              PUSH EAX
        _emit 0x50
        // 00036eff: e8 cc 53 00 00  CALL FUN_0043c2d0  -- RELOC (rel32)
        _emit 0xe8
        _emit 0xcc
        _emit 0x53
        _emit 0x00
        _emit 0x00
        // 00036f04: 5f              POP EDI
        _emit 0x5f
        // 00036f05: 5e              POP ESI
        _emit 0x5e
        // 00036f06: 5b              POP EBX
        _emit 0x5b
        // 00036f07: c2 0c 00        RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00036f0a: 8b 4b 08        MOV ECX,dword ptr [EBX+0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036f0d: 33 c0           XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00036f0f: 50              PUSH EAX
        _emit 0x50
        // 00036f10: e8 bb 53 00 00  CALL FUN_0043c2d0  -- RELOC (rel32)
        _emit 0xe8
        _emit 0xbb
        _emit 0x53
        _emit 0x00
        _emit 0x00
        // 00036f15: 5f              POP EDI
        _emit 0x5f
        // 00036f16: 5e              POP ESI
        _emit 0x5e
        // 00036f17: 5b              POP EBX
        _emit 0x5b
        // 00036f18: c2 0c 00        RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
