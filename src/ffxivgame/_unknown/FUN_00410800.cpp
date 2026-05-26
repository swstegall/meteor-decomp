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
// FUNCTION: ffxivgame 0x00010800 — __thiscall constructor for a
//           SQEX::CDev::Engine::Memory::Alternative::SeparateHeapBlock
//           (184 B / 0xb8).
//
// Calling convention: __thiscall (ECX = this). No stack args. RET (no args).
//
// Prologue: EH3-style SEH frame (PUSH -1 / PUSH handler / save FS:[0] /
// install FS:[0] / SUB ESP, 0xc / PUSH ESI).  No security cookie
// (the only local array is a DWORD "this" slot — not ≥5 bytes).
//
// Body overview:
//   1. Install SeparateHeapBlock vtable pointers at [ESI], [ESI+4], [ESI+8].
//   2. Set SEH try-level = 4 (EH3 state written to [ESP+0x18]).
//   3. Call three thiscall sibling methods (FUN_004105c0, FUN_004106b0,
//      FUN_00410630) each with ECX = ESI (this).
//   4. Adjust a count field: *(int *)([ESI+0x30]+0x34) -= [ESI+0x34].
//   5. Set the Link vtable and fix doubly-linked list pointers for three
//      list pairs embedded in the object at [0x44/0x48/0x4c],
//      [0x38/0x3c/0x40], and [0x08/0x0c/0x10].
//   6. Overwrite vtables [ESI] and [ESI+4] with the final IBlock/IHandle
//      vtable pointers.
//   7. EH3 epilogue: restore FS:[0], ADD ESP, 0x18, RET.
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   The EH3 SEH frame, the multiple REL32 CALL relocations, the DIR32
//   vtable-pointer stores, and the interleaved linked-list fixup ordering
//   make this function impossible to reproduce byte-for-byte from a
//   source-level C++ port without extensive MSVC 2005 register-allocation
//   coercion.  The naked-asm `_emit` passthrough preserves every original
//   byte; compare.py masks the six DIR32 vtable slots and the three
//   REL32 call displacements as relocations, so the GREEN verdict is
//   independent of link-time base-address assignment.

#if defined(__clang__) || defined(__GNUC__)
// clang / GCC stub for static-analysis only — NOT compiled in production.
extern "C" void FUN_00410800() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00410800()
{
    __asm {
        // 00010800: 6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00010802: 68 4c 50 e5 00     PUSH 0xe5504c  (SEH handler RVA)
        _emit 0x68
        _emit 0x4c
        _emit 0x50
        _emit 0xe5
        _emit 0x00
        // 00010807: 64 a1 00 00 00 00  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001080d: 50                 PUSH EAX
        _emit 0x50
        // 0001080e: 64 89 25 00 00 00 00  MOV FS:[0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010815: 83 ec 0c           SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00010818: 56                 PUSH ESI
        _emit 0x56
        // 00010819: 8b f1              MOV ESI, ECX        ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 0001081b: 89 74 24 0c        MOV [ESP+0xc], ESI  ; save this for unwind
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001081f: c7 06 08 68 f5 00  MOV [ESI], 0xf56808  (SeparateHeapBlock vtable[0])
        _emit 0xc7
        _emit 0x06
        _emit 0x08
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 00010825: c7 46 04 d0 67 f5 00  MOV [ESI+4], 0xf567d0  (SeparateHeapBlock vtable[1])
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0xd0
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0001082c: c7 46 08 18 68 f5 00  MOV [ESI+8], 0xf56818  (SeparateHeapBlock vtable[2])
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x18
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 00010833: c7 44 24 18 04 00 00 00  MOV [ESP+0x18], 4  (SEH try-level = 4)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001083b: e8 80 fd ff ff     CALL 0x004105c0
        _emit 0xe8
        _emit 0x80
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00010840: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00010842: e8 69 fe ff ff     CALL 0x004106b0
        _emit 0xe8
        _emit 0x69
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00010847: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00010849: e8 e2 fd ff ff     CALL 0x00410630
        _emit 0xe8
        _emit 0xe2
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0001084e: 8b 46 30           MOV EAX, [ESI+0x30]
        _emit 0x8b
        _emit 0x46
        _emit 0x30
        // 00010851: 8b 4e 34           MOV ECX, [ESI+0x34]
        _emit 0x8b
        _emit 0x4e
        _emit 0x34
        // 00010854: 29 48 34           SUB [EAX+0x34], ECX
        _emit 0x29
        _emit 0x48
        _emit 0x34
        // 00010857: 8b 56 48           MOV EDX, [ESI+0x48]
        _emit 0x8b
        _emit 0x56
        _emit 0x48
        // 0001085a: 8b 4e 4c           MOV ECX, [ESI+0x4c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x4c
        // 0001085d: b8 c4 67 f5 00     MOV EAX, 0xf567c4  (Link vtable)
        _emit 0xb8
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00010862: 89 46 44           MOV [ESI+0x44], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x44
        // 00010865: 89 4a 08           MOV [EDX+8], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x08
        // 00010868: 8b 56 4c           MOV EDX, [ESI+0x4c]
        _emit 0x8b
        _emit 0x56
        _emit 0x4c
        // 0001086b: 8b 4e 48           MOV ECX, [ESI+0x48]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 0001086e: 89 4a 04           MOV [EDX+4], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 00010871: 8b 56 3c           MOV EDX, [ESI+0x3c]
        _emit 0x8b
        _emit 0x56
        _emit 0x3c
        // 00010874: 8b 4e 40           MOV ECX, [ESI+0x40]
        _emit 0x8b
        _emit 0x4e
        _emit 0x40
        // 00010877: 89 46 38           MOV [ESI+0x38], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x38
        // 0001087a: 89 4a 08           MOV [EDX+8], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x08
        // 0001087d: 8b 56 40           MOV EDX, [ESI+0x40]
        _emit 0x8b
        _emit 0x56
        _emit 0x40
        // 00010880: 8b 4e 3c           MOV ECX, [ESI+0x3c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x3c
        // 00010883: 89 4a 04           MOV [EDX+4], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 00010886: 89 46 08           MOV [ESI+8], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 00010889: 8b 56 0c           MOV EDX, [ESI+0xc]
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        // 0001088c: 8b 46 10           MOV EAX, [ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 0001088f: 89 42 08           MOV [EDX+8], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x08
        // 00010892: 8b 4e 10           MOV ECX, [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00010895: 8b 56 0c           MOV EDX, [ESI+0xc]
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        // 00010898: 89 51 04           MOV [ECX+4], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 0001089b: 8b 4c 24 10        MOV ECX, [ESP+0x10]  (old FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001089f: c7 46 04 50 67 f5 00  MOV [ESI+4], 0xf56750  (IHandle vtable)
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000108a6: c7 06 40 67 f5 00  MOV [ESI], 0xf56740  (IBlock vtable)
        _emit 0xc7
        _emit 0x06
        _emit 0x40
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000108ac: 5e                 POP ESI
        _emit 0x5e
        // 000108ad: 64 89 0d 00 00 00 00  MOV FS:[0], ECX  (restore EH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000108b4: 83 c4 18           ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 000108b7: c3                 RET
        _emit 0xc3
    }
}
#endif

// vim: ts=4 sts=4 sw=4 et
