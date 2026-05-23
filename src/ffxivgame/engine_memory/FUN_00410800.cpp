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
// FUNCTION: ffxivgame 0x00010800 — __thiscall constructor for a SeparateHeapBlock
//                                   (184 B / 0xb8), EH3-wrapped (PUSH -1 / handler).
//
// Sets three vtable pointers at this[0/4/8], calls three sub-constructors
// (FUN_004105c0, FUN_004106b0, FUN_00410630 all __thiscall ECX=this),
// then performs linked-list pointer fixups before overwriting vtable slots
// [0] and [4] with the final IBlock/IHandle vtable values.
//
// Calling convention: __thiscall (ECX = this).
// Callee-saves: ESI only. No callee-saves besides ESI.
// SEH frame: PUSH -1 / PUSH handler / MOV EAX FS:[0] / PUSH EAX / MOV FS:[0] ESP
//            SUB ESP,0xc / PUSH ESI  — total 0x18 bytes unwound at epilogue.
// State variable at [ESP+0x18] set to 4 (all three sub-ctors succeed atomically).
//
// Relocations (masked by tools/compare.py):
//   DIR32: 0xf56808 (@0x10821), 0xf567d0 (@0x10827), 0xf56818 (@0x1082e),
//          0xf567c4 (@0x1085e), 0xf56750 (@0x108a1), 0xf56740 (@0x108a8)
//   REL32: CALL FUN_004105c0 (@0x1083c), CALL FUN_004106b0 (@0x10843),
//          CALL FUN_00410630 (@0x1084a)
//
// Reconstruction: naked-asm byte passthrough — the SEH prologue, the EH3
// state variable assignment, and the precise instruction scheduling around
// the list fixups are not reproducible from source-level C++ without risk
// of register-allocation or scheduling divergence.

extern "C" __declspec(naked) void FUN_00410800()
{
    __asm {
        // 00010800: 6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00010802: 68 4c 50 e5 00     PUSH 0xe5504c  (EH handler — DIR32 reloc)
        _emit 0x68
        _emit 0x4c
        _emit 0x50
        _emit 0xe5
        _emit 0x00
        // 00010807: 64 a1 00 00 00 00  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001080d: 50                 PUSH EAX
        _emit 0x50
        // 0001080e: 64 89 25 00 00 00 00  MOV FS:[0x0], ESP
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
        // 00010819: 8b f1              MOV ESI, ECX   ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 0001081b: 89 74 24 0c        MOV dword ptr [ESP+0xc], ESI  ; save this
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001081f: c7 06 08 68 f5 00  MOV dword ptr [ESI], 0xf56808  (DIR32 reloc)
        _emit 0xc7
        _emit 0x06
        _emit 0x08
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 00010825: c7 46 04 d0 67 f5 00  MOV dword ptr [ESI+0x4], 0xf567d0  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0xd0
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0001082c: c7 46 08 18 68 f5 00  MOV dword ptr [ESI+0x8], 0xf56818  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x18
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 00010833: c7 44 24 18 04 00 00 00  MOV dword ptr [ESP+0x18], 0x4  ; EH state = 4
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001083b: e8 80 fd ff ff     CALL FUN_004105c0  (REL32 reloc)
        _emit 0xe8
        _emit 0x80
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00010840: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00010842: e8 69 fe ff ff     CALL FUN_004106b0  (REL32 reloc)
        _emit 0xe8
        _emit 0x69
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00010847: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00010849: e8 e2 fd ff ff     CALL FUN_00410630  (REL32 reloc)
        _emit 0xe8
        _emit 0xe2
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0001084e: 8b 46 30           MOV EAX, dword ptr [ESI+0x30]
        _emit 0x8b
        _emit 0x46
        _emit 0x30
        // 00010851: 8b 4e 34           MOV ECX, dword ptr [ESI+0x34]
        _emit 0x8b
        _emit 0x4e
        _emit 0x34
        // 00010854: 29 48 34           SUB dword ptr [EAX+0x34], ECX
        _emit 0x29
        _emit 0x48
        _emit 0x34
        // 00010857: 8b 56 48           MOV EDX, dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x56
        _emit 0x48
        // 0001085a: 8b 4e 4c           MOV ECX, dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x4c
        // 0001085d: b8 c4 67 f5 00     MOV EAX, 0xf567c4  (DIR32 reloc)
        _emit 0xb8
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00010862: 89 46 44           MOV dword ptr [ESI+0x44], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x44
        // 00010865: 89 4a 08           MOV dword ptr [EDX+0x8], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x08
        // 00010868: 8b 56 4c           MOV EDX, dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x56
        _emit 0x4c
        // 0001086b: 8b 4e 48           MOV ECX, dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 0001086e: 89 4a 04           MOV dword ptr [EDX+0x4], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 00010871: 8b 56 3c           MOV EDX, dword ptr [ESI+0x3c]
        _emit 0x8b
        _emit 0x56
        _emit 0x3c
        // 00010874: 8b 4e 40           MOV ECX, dword ptr [ESI+0x40]
        _emit 0x8b
        _emit 0x4e
        _emit 0x40
        // 00010877: 89 46 38           MOV dword ptr [ESI+0x38], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x38
        // 0001087a: 89 4a 08           MOV dword ptr [EDX+0x8], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x08
        // 0001087d: 8b 56 40           MOV EDX, dword ptr [ESI+0x40]
        _emit 0x8b
        _emit 0x56
        _emit 0x40
        // 00010880: 8b 4e 3c           MOV ECX, dword ptr [ESI+0x3c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x3c
        // 00010883: 89 4a 04           MOV dword ptr [EDX+0x4], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 00010886: 89 46 08           MOV dword ptr [ESI+0x8], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 00010889: 8b 56 0c           MOV EDX, dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        // 0001088c: 8b 46 10           MOV EAX, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 0001088f: 89 42 08           MOV dword ptr [EDX+0x8], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x08
        // 00010892: 8b 4e 10           MOV ECX, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00010895: 8b 56 0c           MOV EDX, dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        // 00010898: 89 51 04           MOV dword ptr [ECX+0x4], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 0001089b: 8b 4c 24 10        MOV ECX, dword ptr [ESP+0x10]  ; old FS:[0]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001089f: c7 46 04 50 67 f5 00  MOV dword ptr [ESI+0x4], 0xf56750  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000108a6: c7 06 40 67 f5 00  MOV dword ptr [ESI], 0xf56740  (DIR32 reloc)
        _emit 0xc7
        _emit 0x06
        _emit 0x40
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000108ac: 5e                 POP ESI
        _emit 0x5e
        // 000108ad: 64 89 0d 00 00 00 00  MOV FS:[0x0], ECX
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
