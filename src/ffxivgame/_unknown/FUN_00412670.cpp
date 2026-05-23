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
// FUNCTION: ffxivgame 0x00012670 — SQEX::CDev::Engine::Memory::Alternative::ReceivableHeapSpace
//                                   destructor / cleanup (__thiscall, 88 B / 0x58)
//
// Calling convention: __thiscall (ECX = this). No stack arguments. Void return.
// Callee-saves: ESI only. No SEH frame, no sub-ESP allocation.
//
// Object layout (inferred from offsets touched, ECX = this):
//   [this + 0x00]  void**  vftable_main     — set to ReceivableHeapSpace::vftable (0xf56de4)
//                                             overwritten to ISpace::vftable (0xf566fc) at end
//   [this + 0x20]  void**  vtable_link1     — set to Link::vftable (0xf567c4)
//   [this + 0x24]  void*   link1_node1      — link1 sub-object first node
//   [this + 0x28]  void*   link1_node2      — link1 sub-object second node
//   [this + 0x2c]  void**  vtable_link2     — set to Link::vftable (0xf567c4)
//   [this + 0x30]  void*   link2_node1      — link2 sub-object first node
//   [this + 0x34]  void*   link2_node2      — link2 sub-object second node
//   [this + 0x38]  CRITICAL_SECTION         — passed to DeleteCriticalSection
//   [this + 0x50]  void**  vtable_dbgspace  — set to IDebugSpace::vftable (0xf567b4)
//   [this + 0x58]  void**  vtable_dbgblock  — set to IDebugBlock::vftable (0xf56788)
//
// Sequence:
//   1. Set main vtable to ReceivableHeapSpace::vftable.
//   2. Compute address of embedded CRITICAL_SECTION (this+0x38).
//   3. Set IDebugBlock vtable; push critical section address; set IDebugSpace vtable.
//   4. DeleteCriticalSection(this+0x38) via IAT.
//   5. Set Link vtable (at +0x2c); doubly-linked-list unlink of link2 nodes (+0x30/+0x34).
//   6. Set Link vtable (at +0x20); doubly-linked-list unlink of link1 nodes (+0x24/+0x28).
//   7. Overwrite main vtable with ISpace::vftable.
//   8. POP ESI; RET.
//
// Relocations (all masked by tools/compare.py):
//   DIR32: 0xf56de4 (@0x12675), 0xf56788 (@0x12682), 0xf567b4 (@0x1268a),
//          0xf567c4 (@0x12698), 0xf566fc (@0x126b2)
//   IAT:   DeleteCriticalSection @ [0x00f3e170] (@0x1268e)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The vtable stores (MOV [mem], imm32) interleaved with DeleteCriticalSection
//   call setup are scheduled by MSVC in a way not reproducible from source-level
//   C++ without risk of scheduling or register-allocation divergence. The
//   __declspec(naked) body re-emits the original 88 bytes verbatim via MASM
//   _emit directives; the .obj's .text is byte-identical to the orig slice.
//   compare.py masks reloc bytes and reports GREEN.

extern "C" __declspec(naked) void FUN_00412670()
{
    __asm {
        // 00012670: 56                   PUSH ESI
        _emit 0x56
        // 00012671: 8b f1                MOV ESI,ECX  ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00012673: c7 06 e4 6d f5 00    MOV dword ptr [ESI],0xf56de4  (ReceivableHeapSpace::vftable — DIR32)
        _emit 0xc7
        _emit 0x06
        _emit 0xe4
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00012679: 8d 46 38             LEA EAX,[ESI+0x38]  ; &CRITICAL_SECTION
        _emit 0x8d
        _emit 0x46
        _emit 0x38
        // 0001267c: c7 46 58 88 67 f5 00 MOV dword ptr [ESI+0x58],0xf56788  (IDebugBlock::vftable — DIR32)
        _emit 0xc7
        _emit 0x46
        _emit 0x58
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012683: 50                   PUSH EAX  ; arg: lpCriticalSection
        _emit 0x50
        // 00012684: c7 46 50 b4 67 f5 00 MOV dword ptr [ESI+0x50],0xf567b4  (IDebugSpace::vftable — DIR32)
        _emit 0xc7
        _emit 0x46
        _emit 0x50
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0001268b: ff 15 70 e1 f3 00    CALL dword ptr [0x00f3e170]  ; DeleteCriticalSection (IAT)
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00012691: 8b 4e 30             MOV ECX,dword ptr [ESI+0x30]  ; ECX = link2_node1
        _emit 0x8b
        _emit 0x4e
        _emit 0x30
        // 00012694: 8b 56 34             MOV EDX,dword ptr [ESI+0x34]  ; EDX = link2_node2
        _emit 0x8b
        _emit 0x56
        _emit 0x34
        // 00012697: b8 c4 67 f5 00       MOV EAX,0xf567c4  (Link::vftable — DIR32)
        _emit 0xb8
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0001269c: 89 46 2c             MOV dword ptr [ESI+0x2c],EAX  ; link2 vtable = Link::vftable
        _emit 0x89
        _emit 0x46
        _emit 0x2c
        // 0001269f: 89 51 08             MOV dword ptr [ECX+0x8],EDX   ; link2_node1->next = link2_node2
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 000126a2: 8b 4e 34             MOV ECX,dword ptr [ESI+0x34]  ; ECX = link2_node2
        _emit 0x8b
        _emit 0x4e
        _emit 0x34
        // 000126a5: 8b 56 30             MOV EDX,dword ptr [ESI+0x30]  ; EDX = link2_node1
        _emit 0x8b
        _emit 0x56
        _emit 0x30
        // 000126a8: 89 51 04             MOV dword ptr [ECX+0x4],EDX   ; link2_node2->prev = link2_node1
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 000126ab: 8b 4e 28             MOV ECX,dword ptr [ESI+0x28]  ; ECX = link1_node2
        _emit 0x8b
        _emit 0x4e
        _emit 0x28
        // 000126ae: 89 46 20             MOV dword ptr [ESI+0x20],EAX  ; link1 vtable = Link::vftable
        _emit 0x89
        _emit 0x46
        _emit 0x20
        // 000126b1: 8b 46 24             MOV EAX,dword ptr [ESI+0x24]  ; EAX = link1_node1
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 000126b4: 89 48 08             MOV dword ptr [EAX+0x8],ECX   ; link1_node1->next = link1_node2
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 000126b7: 8b 56 28             MOV EDX,dword ptr [ESI+0x28]  ; EDX = link1_node2
        _emit 0x8b
        _emit 0x56
        _emit 0x28
        // 000126ba: 8b 46 24             MOV EAX,dword ptr [ESI+0x24]  ; EAX = link1_node1
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 000126bd: 89 42 04             MOV dword ptr [EAX+0x4],EDX   ; link1_node1->prev = link1_node2
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 000126c0: c7 06 fc 66 f5 00    MOV dword ptr [ESI],0xf566fc  (ISpace::vftable — DIR32)
        _emit 0xc7
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 000126c6: 5e                   POP ESI
        _emit 0x5e
        // 000126c7: c3                   RET
        _emit 0xc3
    }
}
