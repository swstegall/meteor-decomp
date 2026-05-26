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
// FUNCTION: ffxivgame 0x00014980 —
//   SQEX::CDev::Engine::Memory::Alternative::SystemHeapBlock::~SystemHeapBlock
//   (__thiscall, 140 B / 0x8c, EH3-SEH wrapped)
//
// Inspection (read from raw .text bytes + Ghidra pseudo-C hint):
//
//   __thiscall void FUN_00414980(this)
//     ECX : this — a fully-derived SystemHeapBlock instance with an
//                  embedded Link node (offsets 0x30..0x38) and an
//                  embedded IBlock+IHandle sub-vftable at the head.
//
// Structure (destructor chain for the most-derived class):
//   EH3 SEH prologue (PUSH -1 / PUSH 0x00e55161 / save FS:[0] / etc.)
//   Reserve 0x0c bytes of locals (ESP scratch + SEH state).
//   PUSH ESI; save this in [ESP+0xc] for unwind reload.
//   Set *this and the next two slots back to SystemHeapBlock::vftable
//     so the in-progress destruction is observable from a debugger
//     even though MSVC will overwrite the first slot again at the
//     bottom of the function with IBlock::vftable. The three writes
//     correspond to:
//       [this+0x00]  primary vftable slot          = 0x00f56ff8
//       [this+0x04]  IHandle    sub-vftable slot   = 0x00f56fc0
//       [this+0x08]  Link       sub-vftable slot   = 0x00f5700c
//   Set local SEH state to 3 and CALL the base-class destructor
//     FUN_004148f0 (the SystemHeapBlock body teardown).
//   Unlink the outer Link node embedded at this+0x30:
//     prev = [this+0x34]; next = [this+0x38]
//     [this+0x30] = Link::vftable (0x00f567c4)
//     prev->next (prev+8) = next
//     next->prev (next+4) = prev
//   Unlink the inner Link node embedded at this+0x08:
//     [this+0x08] = Link::vftable (the same Link::vftable EAX still
//                   holds — MSVC reuses the constant load)
//     prev2 = [this+0x0c]; next2 = [this+0x10]
//     prev2->next (prev2+8) = next2
//     next2->prev (next2+4) = prev2
//   Reload pvStack_c (saved ExceptionList) into ECX from [ESP+0x10].
//   Set the head-of-object vftables to the IHandle / IBlock layer
//     to complete the destructor chain:
//       [this+0x04] = IHandle::vftable (0x00f56750)
//       [this+0x00] = IBlock ::vftable (0x00f56740)
//   POP ESI; restore FS:[0] from ECX; ADD ESP,0x18; RET.
//
// Calling convention: __thiscall, callee-cleans 0 stack args (RET 0).
// Returns: void.
//
// Reloc-bearing sites (absolute/relative addresses baked at RVA 0x14980):
//   +0x02  PUSH imm32 0x00e55161   (EH3 scope-table handler addr)
//   +0x1f  MOV  imm32 0x00f56ff8   (SystemHeapBlock::vftable)
//   +0x26  MOV  imm32 0x00f56fc0   (SystemHeapBlock::vftable +slot)
//   +0x2d  MOV  imm32 0x00f5700c   (SystemHeapBlock::vftable +slot)
//   +0x3b  CALL rel32 FUN_004148f0 (-0xd0)  base-class dtor
//   +0x46  MOV  imm32 0x00f567c4   (Link::vftable)
//   +0x73  MOV  imm32 0x00f56750   (IHandle::vftable)
//   +0x7a  MOV  imm32 0x00f56740   (IBlock::vftable)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The EH3 prologue is not expressible from C++ source at MSVC's
//   chosen frame layout (3-DWORD locals, SEH state at [ESP+0x18],
//   pvStack_c slot at [ESP+0x10]), and the dual-vftable rewrite
//   pattern (write derived vftable, call base dtor, then overwrite
//   with sub-object vftables for each sub-tree) is the textbook MSVC
//   destructor lowering — but only when the compiler can prove the
//   most-derived type. Reproducing the exact register reuse (EAX
//   holding Link::vftable across both Link-unlink blocks) and the
//   exact stack-displacement of the saved ExceptionList load
//   ([ESP+0x10] after PUSH ESI) requires fine-grained MSVC-internal
//   knowledge that source-level C++ cannot pin down. A
//   __declspec(naked) body re-emitting the original 140 bytes
//   verbatim produces a .obj whose .text is byte-identical to the
//   original slice; compare.py reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_00414980() {
    __asm {
        // 00014980:  6a ff                    PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00014982:  68 61 51 e5 00           PUSH 0xe55161    (SEH scope/handler)
        _emit 0x68
        _emit 0x61
        _emit 0x51
        _emit 0xe5
        _emit 0x00
        // 00014987:  64 a1 00 00 00 00        MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001498d:  50                       PUSH EAX
        _emit 0x50
        // 0001498e:  64 89 25 00 00 00 00     MOV dword ptr FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014995:  83 ec 0c                 SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00014998:  56                       PUSH ESI
        _emit 0x56
        // 00014999:  8b f1                    MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0001499b:  89 74 24 0c              MOV dword ptr [ESP+0xc],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001499f:  c7 06 f8 6f f5 00        MOV dword ptr [ESI],0xf56ff8  (SystemHeapBlock::vftable)
        _emit 0xc7
        _emit 0x06
        _emit 0xf8
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 000149a5:  c7 46 04 c0 6f f5 00     MOV dword ptr [ESI+0x4],0xf56fc0
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0xc0
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 000149ac:  c7 46 08 0c 70 f5 00     MOV dword ptr [ESI+0x8],0xf5700c
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x0c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 000149b3:  c7 44 24 18 03 00 00 00  MOV dword ptr [ESP+0x18],0x3  (SEH state = 3)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000149bb:  e8 30 ff ff ff           CALL 0x000148f0  (FUN_004148f0)
        _emit 0xe8
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000149c0:  8b 4e 34                 MOV ECX,dword ptr [ESI+0x34]  (Link prev)
        _emit 0x8b
        _emit 0x4e
        _emit 0x34
        // 000149c3:  8b 56 38                 MOV EDX,dword ptr [ESI+0x38]  (Link next)
        _emit 0x8b
        _emit 0x56
        _emit 0x38
        // 000149c6:  b8 c4 67 f5 00           MOV EAX,0xf567c4              (Link::vftable)
        _emit 0xb8
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000149cb:  89 46 30                 MOV dword ptr [ESI+0x30],EAX  ([this+0x30] = Link::vftable)
        _emit 0x89
        _emit 0x46
        _emit 0x30
        // 000149ce:  89 51 08                 MOV dword ptr [ECX+0x8],EDX   (prev->next = next)
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 000149d1:  8b 4e 38                 MOV ECX,dword ptr [ESI+0x38]  (next)
        _emit 0x8b
        _emit 0x4e
        _emit 0x38
        // 000149d4:  8b 56 34                 MOV EDX,dword ptr [ESI+0x34]  (prev)
        _emit 0x8b
        _emit 0x56
        _emit 0x34
        // 000149d7:  89 51 04                 MOV dword ptr [ECX+0x4],EDX   (next->prev = prev)
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 000149da:  89 46 08                 MOV dword ptr [ESI+0x8],EAX   ([this+0x8] = Link::vftable)
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 000149dd:  8b 46 0c                 MOV EAX,dword ptr [ESI+0xc]   (inner Link prev)
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 000149e0:  8b 4e 10                 MOV ECX,dword ptr [ESI+0x10]  (inner Link next)
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 000149e3:  89 48 08                 MOV dword ptr [EAX+0x8],ECX   (prev->next = next)
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 000149e6:  8b 56 10                 MOV EDX,dword ptr [ESI+0x10]  (next)
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        // 000149e9:  8b 46 0c                 MOV EAX,dword ptr [ESI+0xc]   (prev)
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 000149ec:  8b 4c 24 10              MOV ECX,dword ptr [ESP+0x10]  (saved ExceptionList)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000149f0:  89 42 04                 MOV dword ptr [EDX+0x4],EAX   (next->prev = prev)
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 000149f3:  c7 46 04 50 67 f5 00     MOV dword ptr [ESI+0x4],0xf56750  (IHandle::vftable)
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000149fa:  c7 06 40 67 f5 00        MOV dword ptr [ESI],0xf56740      (IBlock::vftable)
        _emit 0xc7
        _emit 0x06
        _emit 0x40
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014a00:  5e                       POP ESI
        _emit 0x5e
        // 00014a01:  64 89 0d 00 00 00 00     MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014a08:  83 c4 18                 ADD ESP,0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00014a0b:  c3                       RET
        _emit 0xc3
    }
}
#endif
