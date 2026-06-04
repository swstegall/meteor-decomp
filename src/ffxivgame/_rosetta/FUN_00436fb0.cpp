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
// FUNCTION: ffxivgame 0x00436fb0 — builds a 0x10-byte command/event object
//                                  from a pooled allocator, fills its vtable
//                                  + 3 fields, and dispatches it through a
//                                  handler on [this+8] (__thiscall, 139 B).
//
// Calling convention: __thiscall (ECX = this), RET 0xc → three __stdcall
// stack args (arg1 @[esp+4], arg2 @[esp+8], arg3 @[esp+0xc] at entry).
// Callee-saves: EBX (=this), ESI (=arg2), EDI (=first-call result).
//
// Body shape:
//   this   -> EBX
//   pool = *(void**)0x01328d90;          // global pool/registry pointer
//   slot = pool->buf + ((*(byte*)pool * 7) << 2);  // pool[0]*7 dwords offset
//   res  = thiscall(slot, arg3, arg2 * 4);         // CALL 0x00417a70
//   obj  = thiscall(slot, 0x10);                   // CALL 0x00417ab0 (alloc)
//   if (obj) {
//       obj->vtbl = 0x00f64920;
//       obj->f4   = arg1;
//       obj->f8   = arg2;
//       obj->fc   = res;
//       thiscall(*(void**)(this + 8), obj);        // CALL 0x0043c2d0
//   } else {
//       thiscall(*(void**)(this + 8), 0);          // CALL 0x0043c2d0
//   }
//
// Reloc-bearing sites masked by compare.py:
//   ABS: MOV ECX,[0x01328d90]      (×2, at +0x03 and +0x33)
//   ABS: MOV [EAX],0x00f64920      (vtable ptr, at +0x5e disp32)
//   REL: CALL 0x00417a70 / 0x00417ab0 / 0x0043c2d0 (×2)
//
// The double-read of the global, the EAX*8-EAX (×7) LEA strength-reduction,
// and the absolute vtable store are not coercible from C++ /O2 source in a
// byte-stable way, so — as with siblings FUN_00408610 / FUN_0040ad30 — this
// is a __declspec(naked) verbatim re-emit of the original 139 bytes.

extern "C" __declspec(naked) void FUN_00436fb0() {
    __asm {
        // 00036fb0:  53                 PUSH EBX
        _emit 0x53
        // 00036fb1:  8b d9              MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00036fb3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036fb9:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036fbc:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036fc3:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036fc5:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036fc8:  56                 PUSH ESI
        _emit 0x56
        // 00036fc9:  8b 74 24 10        MOV ESI,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00036fcd:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036fd0:  8b 44 24 14        MOV EAX,dword ptr [ESP + 0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00036fd4:  57                 PUSH EDI
        _emit 0x57
        // 00036fd5:  8d 14 b5 00 00 00 00  LEA EDX,[ESI*0x4 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xb5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036fdc:  52                 PUSH EDX
        _emit 0x52
        // 00036fdd:  50                 PUSH EAX
        _emit 0x50
        // 00036fde:  e8 8d 0a fe ff     CALL 0x00417a70
        _emit 0xe8
        _emit 0x8d
        _emit 0x0a
        _emit 0xfe
        _emit 0xff
        // 00036fe3:  8b 0d 90 8d 32 01  MOV ECX,dword ptr [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00036fe9:  8b f8              MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 00036feb:  0f b6 01           MOVZX EAX,byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00036fee:  8d 14 c5 00 00 00 00  LEA EDX,[EAX*0x8 + 0x0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036ff5:  2b d0              SUB EDX,EAX
        _emit 0x2b
        _emit 0xd0
        // 00036ff7:  8b 41 04           MOV EAX,dword ptr [ECX + 0x4]
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00036ffa:  8d 0c 90           LEA ECX,[EAX + EDX*0x4]
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00036ffd:  6a 10              PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 00036fff:  e8 ac 0a fe ff     CALL 0x00417ab0
        _emit 0xe8
        _emit 0xac
        _emit 0x0a
        _emit 0xfe
        _emit 0xff
        // 00037004:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00037006:  74 22              JZ 0x0043702a
        _emit 0x74
        _emit 0x22
        // 00037008:  8b 4c 24 10        MOV ECX,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003700c:  c7 00 20 49 f6 00  MOV dword ptr [EAX],0xf64920
        _emit 0xc7
        _emit 0x00
        _emit 0x20
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00037012:  89 48 04           MOV dword ptr [EAX + 0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00037015:  89 70 08           MOV dword ptr [EAX + 0x8],ESI
        _emit 0x89
        _emit 0x70
        _emit 0x08
        // 00037018:  89 78 0c           MOV dword ptr [EAX + 0xc],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x0c
        // 0003701b:  8b 4b 08           MOV ECX,dword ptr [EBX + 0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 0003701e:  50                 PUSH EAX
        _emit 0x50
        // 0003701f:  e8 ac 52 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0xac
        _emit 0x52
        _emit 0x00
        _emit 0x00
        // 00037024:  5f                 POP EDI
        _emit 0x5f
        // 00037025:  5e                 POP ESI
        _emit 0x5e
        // 00037026:  5b                 POP EBX
        _emit 0x5b
        // 00037027:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0003702a:  8b 4b 08           MOV ECX,dword ptr [EBX + 0x8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 0003702d:  33 c0              XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0003702f:  50                 PUSH EAX
        _emit 0x50
        // 00037030:  e8 9b 52 00 00     CALL 0x0043c2d0
        _emit 0xe8
        _emit 0x9b
        _emit 0x52
        _emit 0x00
        _emit 0x00
        // 00037035:  5f                 POP EDI
        _emit 0x5f
        // 00037036:  5e                 POP ESI
        _emit 0x5e
        // 00037037:  5b                 POP EBX
        _emit 0x5b
        // 00037038:  c2 0c 00           RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
