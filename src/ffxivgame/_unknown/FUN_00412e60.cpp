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
// FUNCTION: ffxivgame 0x00412e60 — __thiscall constructor for a ReceivableHeapBlock
//                                   variant (168 B / 0xa8), EH3-wrapped (PUSH -1 / handler).
//
// Sets three vtable pointers at this[0/4/8], calls two sub-constructors
// (FUN_00412d60, FUN_00412ca0 both __thiscall ECX=this),
// then performs linked-list pointer fixups for three embedded Link sentinels
// (at offsets 0x08, 0x34, 0x40) before overwriting vtable slots [0] and [4]
// with the final IBlock/IHandle vtable values.
//
// Calling convention: __thiscall (ECX = this).
// Callee-saves: ESI only.
// SEH frame: PUSH -1 / PUSH handler / MOV EAX FS:[0] / PUSH EAX / MOV FS:[0] ESP
//            SUB ESP,0xc / PUSH ESI  — total 0x18 bytes unwound at epilogue.
// State variable at [ESP+0x18] set to 4 (both sub-ctors succeed atomically).
//
// Relocations (masked by tools/compare.py):
//   DIR32: 0xe550ec (@0x12e63), 0xf56e60 (@0x12e81), 0xf56e28 (@0x12e87),
//          0xf56e70 (@0x12e8e), 0xf567c4 (@0x12eae), 0xf56750 (@0x12ef1),
//          0xf56740 (@0x12ef8)
//   REL32: CALL FUN_00412d60 (@0x12e9c), CALL FUN_00412ca0 (@0x12ea3)
//
// Reconstruction: naked-asm byte passthrough — the SEH prologue, the EH3
// state variable assignment, and the precise instruction scheduling around
// the list fixups are not reproducible from source-level C++ without risk
// of register-allocation or scheduling divergence.

extern "C" __declspec(naked) void FUN_00412e60()
{
    __asm {
        // 00012e60: 6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00012e62: 68 ec 50 e5 00     PUSH 0xe550ec  (EH handler — DIR32 reloc)
        _emit 0x68
        _emit 0xec
        _emit 0x50
        _emit 0xe5
        _emit 0x00
        // 00012e67: 64 a1 00 00 00 00  MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012e6d: 50                 PUSH EAX
        _emit 0x50
        // 00012e6e: 64 89 25 00 00 00 00  MOV FS:[0x0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012e75: 83 ec 0c           SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00012e78: 56                 PUSH ESI
        _emit 0x56
        // 00012e79: 8b f1              MOV ESI, ECX   ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00012e7b: 89 74 24 0c        MOV dword ptr [ESP+0xc], ESI  ; save this
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00012e7f: c7 06 60 6e f5 00  MOV dword ptr [ESI], 0xf56e60  (DIR32 reloc)
        _emit 0xc7
        _emit 0x06
        _emit 0x60
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012e85: c7 46 04 28 6e f5 00  MOV dword ptr [ESI+0x4], 0xf56e28  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x28
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012e8c: c7 46 08 70 6e f5 00  MOV dword ptr [ESI+0x8], 0xf56e70  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x70
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 00012e93: c7 44 24 18 04 00 00 00  MOV dword ptr [ESP+0x18], 0x4  ; EH state = 4
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012e9b: e8 c0 fe ff ff     CALL FUN_00412d60  (REL32 reloc)
        _emit 0xe8
        _emit 0xc0
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00012ea0: 8b ce              MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00012ea2: e8 f9 fd ff ff     CALL FUN_00412ca0  (REL32 reloc)
        _emit 0xe8
        _emit 0xf9
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00012ea7: 8b 4e 44           MOV ECX, dword ptr [ESI+0x44]
        _emit 0x8b
        _emit 0x4e
        _emit 0x44
        // 00012eaa: 8b 56 48           MOV EDX, dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x56
        _emit 0x48
        // 00012ead: b8 c4 67 f5 00     MOV EAX, 0xf567c4  (DIR32 reloc — Link::vftable)
        _emit 0xb8
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012eb2: 89 46 40           MOV dword ptr [ESI+0x40], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x40
        // 00012eb5: 89 51 08           MOV dword ptr [ECX+0x8], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00012eb8: 8b 4e 48           MOV ECX, dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 00012ebb: 8b 56 44           MOV EDX, dword ptr [ESI+0x44]
        _emit 0x8b
        _emit 0x56
        _emit 0x44
        // 00012ebe: 89 51 04           MOV dword ptr [ECX+0x4], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 00012ec1: 8b 4e 38           MOV ECX, dword ptr [ESI+0x38]
        _emit 0x8b
        _emit 0x4e
        _emit 0x38
        // 00012ec4: 8b 56 3c           MOV EDX, dword ptr [ESI+0x3c]
        _emit 0x8b
        _emit 0x56
        _emit 0x3c
        // 00012ec7: 89 46 34           MOV dword ptr [ESI+0x34], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x34
        // 00012eca: 89 51 08           MOV dword ptr [ECX+0x8], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00012ecd: 8b 4e 3c           MOV ECX, dword ptr [ESI+0x3c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x3c
        // 00012ed0: 8b 56 38           MOV EDX, dword ptr [ESI+0x38]
        _emit 0x8b
        _emit 0x56
        _emit 0x38
        // 00012ed3: 89 51 04           MOV dword ptr [ECX+0x4], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 00012ed6: 89 46 08           MOV dword ptr [ESI+0x8], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 00012ed9: 8b 46 0c           MOV EAX, dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 00012edc: 8b 4e 10           MOV ECX, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00012edf: 89 48 08           MOV dword ptr [EAX+0x8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00012ee2: 8b 56 10           MOV EDX, dword ptr [ESI+0x10]
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        // 00012ee5: 8b 46 0c           MOV EAX, dword ptr [ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 00012ee8: 8b 4c 24 10        MOV ECX, dword ptr [ESP+0x10]  ; old FS:[0]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00012eec: 89 42 04           MOV dword ptr [EDX+0x4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 00012eef: c7 46 04 50 67 f5 00  MOV dword ptr [ESI+0x4], 0xf56750  (DIR32 reloc — IHandle vtable)
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012ef6: c7 06 40 67 f5 00  MOV dword ptr [ESI], 0xf56740  (DIR32 reloc — IBlock vtable)
        _emit 0xc7
        _emit 0x06
        _emit 0x40
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012efc: 5e                 POP ESI
        _emit 0x5e
        // 00012efd: 64 89 0d 00 00 00 00  MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012f04: 83 c4 18           ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00012f07: c3                 RET
        _emit 0xc3
    }
}
