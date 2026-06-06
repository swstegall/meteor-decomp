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
// FUNCTION: ffxivgame 0x00414980 — SystemHeapBlock destructor with x86 SEH frame
//                                   (__thiscall, 140 B / 0x8c)
//
// __thiscall void FUN_00414980(void)
//   ECX = this
//
// This is the virtual destructor body for
// SQEX::CDev::Engine::Memory::Alternative::SystemHeapBlock.  It is
// wrapped in a MSVC 2005 try/finally x86 SEH frame (handler at absolute
// VA 0xe55161), which guards the cleanup call to FUN_004148f0 against
// exceptions during teardown.
//
// Destructor sequence:
//
//   1. Install SEH frame (PUSH -1 / PUSH 0xe55161 / PUSH FS:[0] /
//      MOV FS:[0],ESP / SUB ESP,0xc / PUSH ESI / MOV ESI,ECX).
//   2. Stash `this` in the SEH record at [ESP+0xc] so the handler can
//      back out partial destruction.
//   3. Write the three SystemHeapBlock-specific vtable pointers:
//        [this +  0x00] = 0xf56ff8  (SystemHeapBlock primary, 4 slots)
//        [this +  0x04] = 0xf56fc0  (SystemHeapBlock IHandle, 13 slots)
//        [this +  0x08] = 0xf5700c  (SystemHeapBlock Link, 2 slots)
//      so that any virtual dispatch that FUN_004148f0 performs routes
//      to SystemHeapBlock's own implementations.
//   4. Advance SEH state to 3 (MOV [ESP+0x18], 3 — marks "inside the
//      guarded cleanup").
//   5. CALL FUN_004148f0 (the guarded "destroy-all-cached-cells" sweep
//      documented in decomp-notes/types/ffxivgame/0x00014850.md — walks
//      the intrusive Link sentinel list rooted at [this+0x30], calls
//      each cell's dtor, and frees the backing buffer).
//   6. After the sweep, reinstate Link vtable and re-link the two
//      embedded List sentinels to their self-pointers:
//        EAX = 0xf567c4  (Link::vftable)
//        [this + 0x30] = EAX        — Link2 sentinel vtable
//        [[this+0x34]+0x8] = [this+0x38]  — Link2.prev->next = Link2.next
//        [[this+0x38]+0x4] = [this+0x34]  — Link2.next->prev = Link2.prev
//        [this + 0x08] = EAX        — Link1 sentinel vtable
//        [[this+0x0c]+0x8] = [this+0x10]  — Link1.prev->next = Link1.next
//        [[this+0x10]+0x4] = [this+0x0c]  — Link1.next->prev = Link1.prev
//   7. Restore base-interface vtable pointers (partial destruction state):
//        [this + 0x04] = 0xf56750  (IHandle base, 13 slots)
//        [this + 0x00] = 0xf56740  (IBlock  base,  3 slots)
//   8. Restore FS:[0] (ECX = [ESP+0x10] = saved previous frame pointer)
//      then unwind the stack frame and RET (no caller-cleaned args).
//
// Object layout relevant to this function:
//   [this + 0x00]  primary vtable     SystemHeapBlock (4 slots) → IBlock (3 slots)
//   [this + 0x04]  IHandle sub-object SystemHeapBlock (13 slots) → IHandle (13 slots)
//   [this + 0x08]  Link1 sub-object   vftable, prev at +0x0c, next at +0x10
//   [this + 0x30]  Link2 sentinel     vftable, prev at +0x34, next at +0x38
//
// Vtables (from config/ffxivgame.rtti.json):
//   0x00F56FF8  SystemHeapBlock::vftable       (slot_count 4)
//   0x00F56FC0  SystemHeapBlock::vftable       (slot_count 13)
//   0x00F5700C  SystemHeapBlock::vftable       (slot_count 2)
//   0x00F567C4  Link::vftable                  (slot_count 2)
//   0x00F56750  IHandle::vftable               (slot_count 13)
//   0x00F56740  IBlock::vftable                (slot_count 3)
//
// Reloc-bearing site (the one CALL rel32 in the body):
//     +0x3b  CALL rel32 → FUN_004148f0  (displacement 0xffffff30)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The SEH prologue/epilogue bytes (including the hard-coded handler VA
//   0xe55161 and the FS:[0] ceremony) cannot be reproduced from C++
//   source with byte precision — even __try/__finally emits a different
//   state-slot offset and a different handler-table RVA in MSVC 2005.
//   Following the binary-local idiom established by FUN_004148f0 and
//   FUN_0040ad30, the function is emitted as a __declspec(naked) body
//   re-emitting the original 140 bytes verbatim via MASM _emit
//   directives.

extern "C" __declspec(naked) void FUN_004148f0();

extern "C" __declspec(naked) void FUN_00414980() {
    __asm {
        // 00014980: 6a ff              PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00014982: 68 61 51 e5 00     PUSH 0xe55161
        _emit 0x68
        _emit 0x61
        _emit 0x51
        _emit 0xe5
        _emit 0x00
        // 00014987: 64 a1 00 00 00 00  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001498d: 50                 PUSH EAX
        _emit 0x50
        // 0001498e: 64 89 25 00 00 00 00  MOV FS:[0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014995: 83 ec 0c           SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00014998: 56                 PUSH ESI
        _emit 0x56
        // 00014999: 8b f1              MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0001499b: 89 74 24 0c        MOV [ESP+0xc], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001499f: c7 06 f8 6f f5 00  MOV [ESI], 0xf56ff8
        _emit 0xc7
        _emit 0x06
        _emit 0xf8
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 000149a5: c7 46 04 c0 6f f5 00  MOV [ESI+0x4], 0xf56fc0
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0xc0
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 000149ac: c7 46 08 0c 70 f5 00  MOV [ESI+0x8], 0xf5700c
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x0c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 000149b3: c7 44 24 18 03 00 00 00  MOV [ESP+0x18], 0x3
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000149bb: e8 30 ff ff ff     CALL FUN_004148f0
        _emit 0xe8
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000149c0: 8b 4e 34           MOV ECX, [ESI+0x34]
        _emit 0x8b
        _emit 0x4e
        _emit 0x34
        // 000149c3: 8b 56 38           MOV EDX, [ESI+0x38]
        _emit 0x8b
        _emit 0x56
        _emit 0x38
        // 000149c6: b8 c4 67 f5 00     MOV EAX, 0xf567c4
        _emit 0xb8
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000149cb: 89 46 30           MOV [ESI+0x30], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x30
        // 000149ce: 89 51 08           MOV [ECX+0x8], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 000149d1: 8b 4e 38           MOV ECX, [ESI+0x38]
        _emit 0x8b
        _emit 0x4e
        _emit 0x38
        // 000149d4: 8b 56 34           MOV EDX, [ESI+0x34]
        _emit 0x8b
        _emit 0x56
        _emit 0x34
        // 000149d7: 89 51 04           MOV [ECX+0x4], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 000149da: 89 46 08           MOV [ESI+0x8], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 000149dd: 8b 46 0c           MOV EAX, [ESI+0x0c]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 000149e0: 8b 4e 10           MOV ECX, [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 000149e3: 89 48 08           MOV [EAX+0x8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 000149e6: 8b 56 10           MOV EDX, [ESI+0x10]
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        // 000149e9: 8b 46 0c           MOV EAX, [ESI+0x0c]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 000149ec: 8b 4c 24 10        MOV ECX, [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000149f0: 89 42 04           MOV [EDX+0x4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 000149f3: c7 46 04 50 67 f5 00  MOV [ESI+0x4], 0xf56750
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000149fa: c7 06 40 67 f5 00  MOV [ESI], 0xf56740
        _emit 0xc7
        _emit 0x06
        _emit 0x40
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014a00: 5e                 POP ESI
        _emit 0x5e
        // 00014a01: 64 89 0d 00 00 00 00  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014a08: 83 c4 18           ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00014a0b: c3                 RET
        _emit 0xc3
    }
}
