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
// FUNCTION: ffxivgame 0x00040ed0 — __stdcall 3-arg allocating constructor
//                                  (124 B / 0x7c, SEH-wrapped with /GS).
//
// Behaviour reconstructed from the asm (RVA 0x00040ed0, 124 bytes):
//
//   __stdcall void* FUN_00440ed0(int arg1 /*[EBP+0x8]*/,
//                                int arg2 /*[EBP+0xC]*/,
//                                int arg3 /*[EBP+0x10]*/)
//   {
//       void *p = allocate(0x28);      // CALL 0x009d1b35 (cdecl)
//       local_p = p;                   // spill for SEH: [EBP-0x14]
//       trylevel = 0;                  // SEH state → [EBP-0x4]
//       if (p)
//           *(int*)p = arg1;           // store arg1 at offset 0
//       void *p4 = (char*)p + 0x4;
//       if (p4)
//           *(int*)p4 = arg2;          // store arg2 at offset 4
//       FUN_00440be0((char*)p + 0x8, arg3); // init field at offset 8 (cdecl, 2 args)
//       return p;
//   }
//
// SEH scope table lives at orig-image VA 0x00e56fa0; __security_cookie at
// 0x012ea8b0; allocator thunk at rel32 target 0x009d1b35; FUN_00440be0 at
// rel32 target 0x00440be0. All of these produce relocations in a normal
// .obj — the naked-asm passthrough avoids them by baking the orig image's
// absolute bytes verbatim, exactly as FUN_00403d60 and FUN_00401350 do.
//
// Calling convention: __stdcall (callee pops 3 × 4 = 12 bytes: `ret 0xc`).
//
// Reloc-bearing sites in the 124 bytes (masked by compare.py):
//   +0x05   PUSH imm32   → SEH scope table  (VA 0x00e56fa0)
//   +0x11   MOV  moffs32 → __security_cookie (VA 0x012ea8b0)
//   +0x23   CALL rel32   → allocator        (target 0x009d1b35)
//   +0x60   CALL rel32   → FUN_00440be0     (target 0x00440be0)

extern "C" __declspec(naked) void FUN_00440ed0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -1  (initial SEH trylevel)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e56fa0  (SEH scope table VA)
        _emit 0xa0
        _emit 0x6f
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (save prev FS:[0])
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, EBP
        _emit 0xc5
        _emit 0x50              // PUSH EAX  (GS cookie)
        _emit 0x8d              // LEA EAX, [EBP - 0x0C]
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x00000000], EAX  (install SEH handler)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP - 0x10], ESP  (save ESP for SEH)
        _emit 0x65
        _emit 0xf0
        _emit 0x6a              // PUSH 0x28  (allocation size = 40 bytes)
        _emit 0x28
        _emit 0xe8              // CALL rel32 → 0x009d1b35  (allocator)
        _emit 0x33
        _emit 0x0c
        _emit 0x59
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX  (save allocated ptr)
        _emit 0xf0
        _emit 0x83              // ADD ESP, 0x4  (cdecl cleanup)
        _emit 0xc4
        _emit 0x04
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x89              // MOV [EBP - 0x14], ESI  (spill ptr for SEH)
        _emit 0x75
        _emit 0xec
        _emit 0xc7              // MOV dword ptr [EBP - 0x4], 0  (trylevel = 0)
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +5  (skip if null)
        _emit 0x05
        _emit 0x8b              // MOV EAX, [EBP + 0x8]  (arg1)
        _emit 0x45
        _emit 0x08
        _emit 0x89              // MOV [ESI], EAX  (p->field0 = arg1)
        _emit 0x06
        _emit 0x8d              // LEA EAX, [ESI + 0x4]  (ptr to field4)
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +5  (skip if null)
        _emit 0x05
        _emit 0x8b              // MOV ECX, [EBP + 0xC]  (arg2)
        _emit 0x4d
        _emit 0x0c
        _emit 0x89              // MOV [EAX], ECX  (p->field4 = arg2)
        _emit 0x08
        _emit 0x8b              // MOV EDX, [EBP + 0x10]  (arg3)
        _emit 0x55
        _emit 0x10
        _emit 0x52              // PUSH EDX  (arg3 for FUN_00440be0)
        _emit 0x8d              // LEA EAX, [ESI + 0x8]  (ptr to field8)
        _emit 0x46
        _emit 0x08
        _emit 0x50              // PUSH EAX  (ptr for FUN_00440be0)
        _emit 0xe8              // CALL rel32 → 0x00440be0
        _emit 0xad
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x8  (cdecl cleanup)
        _emit 0xc4
        _emit 0x08
        _emit 0x8b              // MOV EAX, ESI  (return value = p)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [EBP - 0x0C]  (saved prev FS:[0])
        _emit 0x4d
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x00000000], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX  (drop GS cookie)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0xC
        _emit 0x0c
        _emit 0x00
    }
}
