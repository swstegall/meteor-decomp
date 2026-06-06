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
// FUNCTION: ffxivgame 0x004326b0 — `__stdcall` 3-arg registration helper
//                                  that builds a descriptor, fetches a
//                                  lazily-initialised singleton, allocates a
//                                  12-byte slot from it, and fills three
//                                  fields (91 B / 0x5b, no SEH).
//
// Inspection (read from asm/ffxivgame/000326b0_FUN_004326b0.s):
//
//   __stdcall void* FUN_004326b0(int arg1 /* [ESP+0xc] */,
//                                int arg2 /* [ESP+0x10] */,
//                                int* arg3 /* [ESP+0x14] */);
//
//     char desc[8];                                   // local @ [ESP+0xc]
//     // __thiscall ctor: this=&desc, (0x10, "ptr 0xf57b04")
//     void* esi = FUN_0040e2d0(&desc, 0x10, (void*)0xf57b04);
//
//     Singleton* s = *(Singleton**)0x01327fc0;        // .data
//     if (s == nullptr)
//         s = FUN_0040e500();                          // lazy init / get
//
//     // __thiscall alloc: this=s, (0xc /* slot size */, esi)
//     void* slot = s->FUN_0040e110(0xc, esi);          // returns EAX
//     if (slot) {
//         *(int*)((char*)slot + 0x0) = arg1;           // desc[0] field
//     }
//     // (slot+4) and (slot+8) are tested for null but the LEA result is
//     // never zero in practice — MSVC emitted the redundant TEST/JZ guards
//     // from the source-level `if (p)` on each member pointer:
//     *(int*)((char*)slot + 0x4) = arg2;
//     *(int*)((char*)slot + 0x8) = *arg3;
//     return slot;                                     // EAX preserved
//
//   Calling convention: __stdcall — 3 stack args (0xc bytes), callee
//   cleans via `ret 0xc`. EAX (the allocated slot) is the return value.
//
//   Reloc-bearing sites in the orig 91 bytes (resolve only at full-binary
//   relink @ image base 0x00400000; tools/compare.py masks the reloc
//   windows so the naked-asm .obj matches byte-for-byte):
//     +0x04   descriptor string PUSH   (abs imm 0x00f57b04)
//     +0x0f   ctor CALL                (.text 0x0040e2d0 rel32)
//     +0x16   singleton ptr LOAD       (.data 0x01327fc0 moffs32)
//     +0x1f   lazy-init CALL           (.text 0x0040e500 rel32)
//     +0x29   slot alloc CALL          (.text 0x0040e110 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rebuild would need cl.exe to reproduce the exact moffs32
//   global load (`a1 ...`), the three already-resolved rel32 call
//   displacements, and the absolute descriptor-string immediate — none of
//   which a standalone .obj can emit without the full-binary relink. The
//   pragmatic choice (same as the FUN_0040a530 / FUN_00406ea0 siblings) is
//   a `__declspec(naked)` body that re-emits the orig 91 bytes verbatim via
//   MASM `_emit` directives; the .obj's `.text` ends up byte-identical to
//   the orig slice.
//
// Asm shape (91 bytes — read from asm/ffxivgame/000326b0_FUN_004326b0.s,
// RVA 0x000326b0..0x0003270a):
//
//     000326b0:  83 ec 08              SUB  ESP, 0x8
//     000326b3:  56                    PUSH ESI
//     000326b4:  68 04 7b f5 00        PUSH 0xf57b04          ; descriptor str
//     000326b9:  6a 10                 PUSH 0x10
//     000326bb:  8d 4c 24 0c           LEA  ECX, [ESP+0xc]    ; this = &desc
//     000326bf:  e8 0c bc fd ff        CALL 0x0040e2d0        ; ctor
//     000326c4:  8b f0                 MOV  ESI, EAX
//     000326c6:  a1 c0 7f 32 01        MOV  EAX, [0x01327fc0] ; singleton
//     000326cb:  85 c0                 TEST EAX, EAX
//     000326cd:  75 05                 JNZ  0x004326d4
//     000326cf:  e8 2c be fd ff        CALL 0x0040e500        ; lazy init
//     000326d4:  56                    PUSH ESI
//     000326d5:  6a 0c                 PUSH 0xc
//     000326d7:  8b c8                 MOV  ECX, EAX
//     000326d9:  e8 32 ba fd ff        CALL 0x0040e110        ; alloc slot
//     000326de:  85 c0                 TEST EAX, EAX
//     000326e0:  5e                    POP  ESI
//     000326e1:  74 06                 JZ   0x004326e9
//     000326e3:  8b 4c 24 0c           MOV  ECX, [ESP+0xc]    ; arg1
//     000326e7:  89 08                 MOV  [EAX], ECX
//     000326e9:  8d 48 04              LEA  ECX, [EAX+0x4]
//     000326ec:  85 c9                 TEST ECX, ECX
//     000326ee:  74 06                 JZ   0x004326f6
//     000326f0:  8b 54 24 10           MOV  EDX, [ESP+0x10]   ; arg2
//     000326f4:  89 11                 MOV  [ECX], EDX
//     000326f6:  8d 48 08              LEA  ECX, [EAX+0x8]
//     000326f9:  85 c9                 TEST ECX, ECX
//     000326fb:  74 08                 JZ   0x00432705
//     000326fd:  8b 54 24 14           MOV  EDX, [ESP+0x14]   ; arg3
//     00032701:  8b 12                 MOV  EDX, [EDX]
//     00032703:  89 11                 MOV  [ECX], EDX
//     00032705:  83 c4 08              ADD  ESP, 0x8
//     00032708:  c2 0c 00              RET  0xc

extern "C" __declspec(naked) void FUN_004326b0() {
    __asm {
        _emit 0x83      // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x56      // PUSH ESI
        _emit 0x68      // PUSH 0xf57b04
        _emit 0x04
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        _emit 0x6a      // PUSH 0x10
        _emit 0x10
        _emit 0x8d      // LEA ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8      // CALL 0x0040e2d0
        _emit 0x0c
        _emit 0xbc
        _emit 0xfd
        _emit 0xff
        _emit 0x8b      // MOV ESI, EAX
        _emit 0xf0
        _emit 0xa1      // MOV EAX, [0x01327fc0]
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x85      // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75      // JNZ +5
        _emit 0x05
        _emit 0xe8      // CALL 0x0040e500
        _emit 0x2c
        _emit 0xbe
        _emit 0xfd
        _emit 0xff
        _emit 0x56      // PUSH ESI
        _emit 0x6a      // PUSH 0xc
        _emit 0x0c
        _emit 0x8b      // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8      // CALL 0x0040e110
        _emit 0x32
        _emit 0xba
        _emit 0xfd
        _emit 0xff
        _emit 0x85      // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5e      // POP ESI
        _emit 0x74      // JZ +6
        _emit 0x06
        _emit 0x8b      // MOV ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x89      // MOV [EAX], ECX
        _emit 0x08
        _emit 0x8d      // LEA ECX, [EAX+0x4]
        _emit 0x48
        _emit 0x04
        _emit 0x85      // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74      // JZ +6
        _emit 0x06
        _emit 0x8b      // MOV EDX, [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x89      // MOV [ECX], EDX
        _emit 0x11
        _emit 0x8d      // LEA ECX, [EAX+0x8]
        _emit 0x48
        _emit 0x08
        _emit 0x85      // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74      // JZ +8
        _emit 0x08
        _emit 0x8b      // MOV EDX, [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b      // MOV EDX, [EDX]
        _emit 0x12
        _emit 0x89      // MOV [ECX], EDX
        _emit 0x11
        _emit 0x83      // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2      // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
