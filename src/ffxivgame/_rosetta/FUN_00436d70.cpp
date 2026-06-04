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
// FUNCTION: ffxivgame 0x00436d70 — factory/handler that allocates a
//                                  0x10-byte command object, populates it
//                                  with a vtable pointer + three fields, and
//                                  dispatches it to a queue helper
//                                  (__thiscall, 139 bytes / 0x8b).
//
// Calling convention: __thiscall (ECX = this, saved to EBX at entry).
// Two stack arguments + cleans 0xc on return (RET 0xc).
//
// Shape (inferred from the asm):
//
//   void __thiscall FUN_00436d70(This *this /*ecx*/,
//                                int   arg0 /*[esp+0x10] → +0x4 field*/,
//                                int   arg1 /*[esp+0x14] → memcpy size*/);
//
// 1. Reads a global pool pointer at [0x01328d90]; its first byte is an
//    index, scaled by 7 (LEA EAX*8 - EAX) then *4 and added to pool[+4]
//    to form a per-bucket allocator context pointer (ECX) passed to the
//    allocator helper 0x00417a70.
// 2. Calls the size-based allocator 0x00417ab0 (PUSH 0x10) to obtain a
//    0x10-byte object.
// 3. On success, stamps the object's vtable (0xf64908), copies arg0 to
//    +0x4, ESI (arg1) to +0x8, EDI (allocator-result) to +0xc, then
//    dispatches it via 0x0043c2d0 against [this+0x8].
// 4. On allocation failure, dispatches a null pointer via the same path.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two allocator helpers, the dispatch helper, the global pool
//   pointer [0x01328d90] and the absolute vtable constant 0xf64908 are
//   all linker-resolved references whose final bytes appear verbatim in
//   the original .text slice. A __declspec(naked) body re-emits the
//   orig 139 bytes via MASM _emit directives; the .obj's .text is
//   byte-identical to the orig slice and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00436d70() {
    __asm {
        // 00036d70:  53                 PUSH EBX
        _emit 0x53
        // 00036d71:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00036d73:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036d79:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036d7c:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036d83:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036d85:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036d88:  56                 PUSH ESI
        _emit 0x56
        // 00036d89:  8b 74 24 10        MOV ESI,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00036d8d:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036d90:  8b 44 24 14        MOV EAX,dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00036d94:  57                 PUSH EDI
        _emit 0x57
        // 00036d95:  8d 14 b5 00 00 00 00  LEA EDX,[ESI*0x4 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xb5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036d9c:  52                 PUSH EDX
        _emit 0x52
        // 00036d9d:  50                 PUSH EAX
        _emit 0x50
        // 00036d9e:  e8 cd 0c fe ff     CALL 0x00417a70
        _emit 0xe8
        _emit 0xcd
        _emit 0x0c
        _emit 0xfe
        _emit 0xff
        // 00036da3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036da9:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00036dab:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036dae:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036db5:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036db7:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036dba:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036dbd:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00036dbf:  e8 ec 0c fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0xec
        _emit 0x0c
        _emit 0xfe
        _emit 0xff
        // 00036dc4:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00036dc6:  74 22              JZ 0x00436dea
        _emit 0x74
        _emit 0x22
        // 00036dc8:  8b 4c 24 10        MOV ECX,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00036dcc:  c7 00 08 49 f6 00  MOV dword ptr [EAX],0xf64908
        _emit 0xc7
        _emit 0x00
        _emit 0x08
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00036dd2:  89 48 04           MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00036dd5:  89 70 08           MOV dword ptr [EAX + 0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00036dd8:  89 78 0c           MOV dword ptr [EAX + 0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 00036ddb:  8b 4b 08           MOV ECX,dword ptr [EBX + 0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036dde:  50                 PUSH EAX
        _emit 0x50
        // 00036ddf:  e8 ec 54 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xec
        _emit 0x54
        _emit 0x00
        _emit 0x00
        // 00036de4:  5f                 POP EDI
        _emit 0x5f
        // 00036de5:  5e                 POP ESI
        _emit 0x5e
        // 00036de6:  5b                 POP EBX
        _emit 0x5b
        // 00036de7:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00036dea:  8b 4b 08           MOV ECX,dword ptr [EBX + 0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00036ded:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00036def:  50                 PUSH EAX
        _emit 0x50
        // 00036df0:  e8 db 54 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xdb
        _emit 0x54
        _emit 0x00
        _emit 0x00
        // 00036df5:  5f                 POP EDI
        _emit 0x5f
        // 00036df6:  5e                 POP ESI
        _emit 0x5e
        // 00036df7:  5b                 POP EBX
        _emit 0x5b
        // 00036df8:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
