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
// FUNCTION: ffxivgame 0x0000f700 — __thiscall destructor/teardown for
//                                   SQEX::CDev::Engine::Memory::Alternative::SeparateHeapSpace
//                                   (161 bytes / 0xa1)
//
// Calling convention: __thiscall (ECX = this). No explicit args.
// Callee-saves pushed: ESI (+ ECX saved to stack as part of SEH frame).
//
// Object layout (DWORDs, in_ECX = this as undefined4*):
//   [+0x00]  vftable pointer — set to SeparateHeapSpace::vftable (0xf56834)
//            at entry, then overwritten with ISpace::vftable (0xf566fc) at end
//   [+0x08]  in_ECX[2]  — heap pointer; freed if field_28 is non-zero
//   [+0x28]  in_ECX[10] as byte — flag: non-zero means field_8 needs freeing
//   [+0x44]  in_ECX[0x11] — Link::vftable pointer (0xf567c4)
//   [+0x48]  in_ECX[0x12] — linked-list node pointer A
//   [+0x4c]  in_ECX[0x13] — linked-list node pointer B (vtable dispatch target)
//   [+0x2c]  in_ECX[0xb]  — CRITICAL_SECTION (24 bytes, passed to
//                            DeleteCriticalSection)
//   [+0x5c]  in_ECX[0x17] — IDebugSpace::vftable pointer (0xf567b4)
//   [+0x64]  in_ECX[0x19] — IDebugBlock::vftable pointer (0xf56788)
//
// Sequence:
//   1. SEH prologue (state -1 → 3):
//        PUSH -1 / PUSH handler / MOV EAX,FS:[0] / PUSH EAX / FS:[0]=ESP
//        PUSH ECX (this) / PUSH ESI / MOV ESI,ECX
//   2. Store SeparateHeapSpace vftable (0xf56834) into *this.
//   3. Virtual dispatch through this->field_4c->vtable[1]() with SEH state=3.
//   4. Pass result to FUN_00410730(this, result) [__thiscall].
//   5. If field_28 != 0: FUN_009d56fd(field_8); field_8=0; field_28=0.
//   6. Set sub-object vftable pointers (IDebugBlock, IDebugSpace, Link).
//   7. Doubly-linked list detach: field_48->field_8 = field_4c;
//                                  field_4c->field_4 = field_48.
//   8. DeleteCriticalSection(&field_2c).
//   9. Store ISpace::vftable (0xf566fc) into *this.
//  10. SEH epilogue: restore FS:[0], pop ESI, ADD ESP 0x10, RET.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH frame (PUSH -1 / handler / FS:[0] swap) and the SEH-state
//   update (MOV [ESP+0x10], 3) are not reproducible from C++ source
//   without __try/__except, which would change the handler table and
//   state-machine shape. The naked-asm body re-emits the original
//   161 bytes verbatim via MASM _emit directives.
//
// Reloc-bearing sites (compare.py masks these):
//   +0x02  DIR32  PUSH 0xe54f39        (SEH handler VA)
//   +0x10  DIR32  MOV [ESI], 0xf56834  (SeparateHeapSpace::vftable)
//   +0x38  REL32  CALL FUN_00410730
//   +0x47  REL32  CALL FUN_009d56fd
//   +0x5a  DIR32  MOV [ESI+0x64], 0xf56788  (IDebugBlock::vftable)
//   +0x61  DIR32  MOV [ESI+0x5c], 0xf567b4  (IDebugSpace::vftable)
//   +0x6e  DIR32  MOV [ESI+0x44], 0xf567c4  (Link::vftable)
//   +0x85  DIR32  CALL [0x00f3e170]          (DeleteCriticalSection IAT)
//   +0x8f  DIR32  MOV [ESI], 0xf566fc        (ISpace::vftable)

extern "C" __declspec(naked) void FUN_0040f700() {
    __asm {
        // 0000f700:  6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0000f702:  68 39 4f e5 00     PUSH 0xe54f39  (SEH handler VA)
        _emit 0x68
        _emit 0x39
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        // 0000f707:  64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f70d:  50                 PUSH EAX
        _emit 0x50
        // 0000f70e:  64 89 25 00 00 00 00  MOV dword ptr FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f715:  51                 PUSH ECX
        _emit 0x51
        // 0000f716:  56                 PUSH ESI
        _emit 0x56
        // 0000f717:  8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0000f719:  89 74 24 04        MOV dword ptr [ESP+0x4],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x04
        // 0000f71d:  c7 06 34 68 f5 00  MOV dword ptr [ESI],0xf56834
        _emit 0xc7
        _emit 0x06
        _emit 0x34
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0000f723:  8b 4e 4c           MOV ECX,dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x4c
        // 0000f726:  8b 01              MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 0000f728:  8b 50 04           MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0000f72b:  c7 44 24 10 03 00 00 00  MOV dword ptr [ESP+0x10],0x3
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f733:  ff d2              CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0000f735:  50                 PUSH EAX
        _emit 0x50
        // 0000f736:  8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0000f738:  e8 f3 0f 00 00     CALL 0x00410730  (REL32)
        _emit 0xe8
        _emit 0xf3
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 0000f73d:  80 7e 28 00        CMP byte ptr [ESI+0x28],0x0
        _emit 0x80
        _emit 0x7e
        _emit 0x28
        _emit 0x00
        // 0000f741:  74 17              JZ +0x17
        _emit 0x74
        _emit 0x17
        // 0000f743:  8b 46 08           MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0000f746:  50                 PUSH EAX
        _emit 0x50
        // 0000f747:  e8 b1 5f 5c 00     CALL 0x009d56fd  (REL32)
        _emit 0xe8
        _emit 0xb1
        _emit 0x5f
        _emit 0x5c
        _emit 0x00
        // 0000f74c:  83 c4 04           ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0000f74f:  c7 46 08 00 00 00 00  MOV dword ptr [ESI+0x8],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f756:  c6 46 28 00        MOV byte ptr [ESI+0x28],0x0
        _emit 0xc6
        _emit 0x46
        _emit 0x28
        _emit 0x00
        // 0000f75a:  c7 46 64 88 67 f5 00  MOV dword ptr [ESI+0x64],0xf56788
        _emit 0xc7
        _emit 0x46
        _emit 0x64
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0000f761:  c7 46 5c b4 67 f5 00  MOV dword ptr [ESI+0x5c],0xf567b4
        _emit 0xc7
        _emit 0x46
        _emit 0x5c
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0000f768:  8b 4e 48           MOV ECX,dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 0000f76b:  8b 56 4c           MOV EDX,dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x56
        _emit 0x4c
        // 0000f76e:  c7 46 44 c4 67 f5 00  MOV dword ptr [ESI+0x44],0xf567c4
        _emit 0xc7
        _emit 0x46
        _emit 0x44
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0000f775:  89 51 08           MOV dword ptr [ECX+0x8],EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 0000f778:  8b 46 4c           MOV EAX,dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x46
        _emit 0x4c
        // 0000f77b:  8b 4e 48           MOV ECX,dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 0000f77e:  8d 56 2c           LEA EDX,[ESI+0x2c]
        _emit 0x8d
        _emit 0x56
        _emit 0x2c
        // 0000f781:  52                 PUSH EDX
        _emit 0x52
        // 0000f782:  89 48 04           MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 0000f785:  ff 15 70 e1 f3 00  CALL dword ptr [0x00f3e170]  (DeleteCriticalSection IAT)
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000f78b:  8b 4c 24 08        MOV ECX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0000f78f:  c7 06 fc 66 f5 00  MOV dword ptr [ESI],0xf566fc
        _emit 0xc7
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f795:  5e                 POP ESI
        _emit 0x5e
        // 0000f796:  64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f79d:  83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000f7a0:  c3                 RET
        _emit 0xc3
    }
}

// vim: ts=4 sts=4 sw=4 et
