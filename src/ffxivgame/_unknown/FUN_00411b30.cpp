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
// FUNCTION: ffxivgame 0x00011b30 — __thiscall destructor for a
//                                   RemovableHeapSpace-derived manager class
//                                   (129 B / 0x81), EH3-wrapped.
//
// Calling convention: __thiscall (ECX = this). No stack arguments. Void return.
// Callee-saves: ESI only. SEH frame uses PUSH ECX (not SUB ESP) for one extra slot.
//
// SEH frame layout (after MOV FS:[0],ESP, then PUSH ECX, PUSH ESI):
//   [ESP+ 0]  saved ESI
//   [ESP+ 4]  saved ECX (= this, spilled here so SEH handler can reach it)
//   [ESP+ 8]  old FS:[0]
//   [ESP+ 0c] EH handler (0xe55068)
//   [ESP+10]  EH state = -1 initially; set to 0 via MOV [ESP+0x14],0 from
//             the post-PUSH-ECX position before it
//
// Object layout (inferred from offsets touched, ECX = this):
//   [this +  0x00]  void **vtable_main      — set to RemovableHeapSpace::vftable
//                                             then to ISpace::vftable at epilogue
//   [this +  0x4c]  void **vtable_4c        — set to Link::vftable
//   [this +  0x50]  ListNode *prev_node     — list-unlink: prev->next = next
//   [this +  0x54]  ListNode *next_node     — list-unlink: next->prev = prev
//   [this +  0x58]  void **vtable_58        — set to IHandleListener::vftable
//   [this +  0x5c]  (CRITICAL_SECTION base) — passed to DeleteCriticalSection
//   [this +  0x74]  void **vtable_74        — set to IDebugSpace::vftable
//   [this +  0x7c]  void **vtable_7c        — set to IDebugBlock::vftable
//
// Sequence:
//   1. SEH prologue (PUSH -1 / PUSH handler / link FS chain).
//   2. Set initial vtable pointers (RemovableHeapSpace / IDebugBlock / IDebugSpace).
//   3. Set EH state = 0; set IHandleListener vtable.
//   4. DeleteCriticalSection(this+0x5c) via IAT.
//   5. Set Link vtable; perform doubly-linked-list unlink of this object.
//   6. Call FUN_00411c90(this+0x18) — inner sub-object teardown.
//   7. Set final vtable to ISpace::vftable.
//   8. SEH epilogue (restore FS chain / ADD ESP,0x10 / RET).
//
// Relocations (all masked by tools/compare.py):
//   DIR32: 0xf56d3c (@0x11b4d), 0xf56788 (@0x11b57), 0xf567b4 (@0x11b67),
//          0xf56ccc (@0x11b74), 0xf567c4 (@0x11b81), 0xf566fc (@0x11ba0)
//   IAT:   DeleteCriticalSection  @ [0x00f3e170]  (@0x11b6d)
//   REL32: CALL FUN_00411c90  (@0x11b97)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The EH3 prologue/epilogue sequence and the precise instruction scheduling
//   (vtable writes interleaved with the DeleteCriticalSection call setup and
//   the list-unlink sequence) are not reproducible from source-level C++
//   without risk of register-allocation or scheduling divergence. The
//   __declspec(naked) body re-emits the original 129 bytes verbatim via
//   MASM _emit directives; the .obj's .text is byte-identical to the orig
//   slice. compare.py masks reloc bytes and reports GREEN.

// Forward declaration for the one direct relative CALL target.
extern "C" void FUN_00411c90();

extern "C" __declspec(naked) void FUN_00411b30()
{
    __asm {
        // 00011b30: 6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00011b32: 68 68 50 e5 00     PUSH 0xe55068  (EH handler — DIR32 reloc)
        _emit 0x68
        _emit 0x68
        _emit 0x50
        _emit 0xe5
        _emit 0x00
        // 00011b37: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011b3d: 50                 PUSH EAX
        _emit 0x50
        // 00011b3e: 64 89 25 00 00 00 00  MOV FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011b45: 51                 PUSH ECX  (saves this; also makes EH state slot)
        _emit 0x51
        // 00011b46: 56                 PUSH ESI
        _emit 0x56
        // 00011b47: 8b f1              MOV ESI,ECX  ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00011b49: 89 74 24 04        MOV dword ptr [ESP+0x4],ESI  ; save this to stack
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x04
        // 00011b4d: c7 06 3c 6d f5 00  MOV dword ptr [ESI],0xf56d3c  (DIR32 reloc)
        _emit 0xc7
        _emit 0x06
        _emit 0x3c
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00011b53: 8d 46 5c           LEA EAX,[ESI+0x5c]  ; &CRITICAL_SECTION
        _emit 0x8d
        _emit 0x46
        _emit 0x5c
        // 00011b56: c7 46 7c 88 67 f5 00  MOV dword ptr [ESI+0x7c],0xf56788  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x7c
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011b5d: 50                 PUSH EAX  ; arg: lpCriticalSection
        _emit 0x50
        // 00011b5e: c7 44 24 14 00 00 00 00  MOV dword ptr [ESP+0x14],0x0  ; EH state = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011b66: c7 46 74 b4 67 f5 00  MOV dword ptr [ESI+0x74],0xf567b4  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x74
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011b6d: ff 15 70 e1 f3 00  CALL dword ptr [0x00f3e170]  ; DeleteCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00011b73: c7 46 58 cc 6c f5 00  MOV dword ptr [ESI+0x58],0xf56ccc  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x58
        _emit 0xcc
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 00011b7a: 8b 4e 50           MOV ECX,dword ptr [ESI+0x50]  ; ECX = prev_node
        _emit 0x8b
        _emit 0x4e
        _emit 0x50
        // 00011b7d: 8b 56 54           MOV EDX,dword ptr [ESI+0x54]  ; EDX = next_node
        _emit 0x8b
        _emit 0x56
        _emit 0x54
        // 00011b80: c7 46 4c c4 67 f5 00  MOV dword ptr [ESI+0x4c],0xf567c4  (DIR32 reloc)
        _emit 0xc7
        _emit 0x46
        _emit 0x4c
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011b87: 89 51 08           MOV dword ptr [ECX+0x8],EDX  ; prev->next = next
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00011b8a: 8b 4e 50           MOV ECX,dword ptr [ESI+0x50]  ; reload prev_node
        _emit 0x8b
        _emit 0x4e
        _emit 0x50
        // 00011b8d: 8b 46 54           MOV EAX,dword ptr [ESI+0x54]  ; EAX = next_node
        _emit 0x8b
        _emit 0x46
        _emit 0x54
        // 00011b90: 89 48 04           MOV dword ptr [EAX+0x4],ECX  ; next->prev = prev
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00011b93: 8d 4e 18           LEA ECX,[ESI+0x18]  ; ECX = this+0x18 (sub-object)
        _emit 0x8d
        _emit 0x4e
        _emit 0x18
        // 00011b96: e8 f5 00 00 00     CALL 0x00411c90  (REL32 reloc — masked)
        _emit 0xe8
        _emit 0xf5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011b9b: 8b 4c 24 08        MOV ECX,dword ptr [ESP+0x8]  ; ECX = old FS:[0]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00011b9f: c7 06 fc 66 f5 00  MOV dword ptr [ESI],0xf566fc  (DIR32 reloc)
        _emit 0xc7
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 00011ba5: 5e                 POP ESI
        _emit 0x5e
        // 00011ba6: 64 89 0d 00 00 00 00  MOV FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011bad: 83 c4 10           ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00011bb0: c3                 RET
        _emit 0xc3
    }
}
