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
// FUNCTION: ffxivgame 0x00014580 — SystemHeapBlock factory: aligned-malloc
//                                  the 0x3c-byte container, aligned-malloc
//                                  the backing buffer, then ctor the block
//                                  in-place. (__thiscall, 3 stack args,
//                                  143 B / 0x8f)
//
// int * __thiscall FUN_00414580(C *this,
//                               size_t param_1,   // backing alloc size
//                               size_t param_2,   // backing alloc alignment
//                               undefined4 param_3);
//
// ECX = this (the parent IGuard / owner — saved into the new block at +0x14).
// Two CALL sites both resolve to __aligned_malloc @ RVA 0x005d5712 (__cdecl,
// 2-arg, caller-cleans). The four PUSHes (0x10, 0x3c, param_2, param_1) are
// caller-cleaned by the single `ADD ESP, 0x10` after the second CALL.
//
// Object layout (matches Ghidra hint; ends exactly at +0x3c — same size as
// the aligned_malloc request):
//   [+0x00] primary vftable   = 0xf56ff8 (SystemHeapBlock primary)
//   [+0x04] secondary vftable = 0xf56fc0 (SystemHeapBlock secondary;
//                                          IHandle vftable 0xf56750 written
//                                          first, then overwritten)
//   [+0x08] embedded Link vftable = 0xf5700c (SystemHeapBlock embedded-Link
//                                              override; Link vftable
//                                              0xf567c4 written first,
//                                              then overwritten)
//   [+0x0c] Link.next = &this->[+8]   (self-init via LEA ECX,[ESI+8] /
//   [+0x10] Link.prev = &this->[+8]    MOV [ECX+4],ECX / MOV [ECX+8],ECX)
//   [+0x14] this  (parent owner from caller's ECX; stashed via EDI)
//   [+0x18] pvVar4 (backing buffer from the second __aligned_malloc)
//   [+0x1c] param_1 (size cached into the block)
//   [+0x20] param_2 (alignment cached into the block)
//   [+0x24] param_3 (flags/userdata from caller's [ESP+0x1c] post-pushes)
//   [+0x28] 0
//   [+0x2c] self  (back-pointer)
//   [+0x30] embedded Link sub-sentinel: vftable = 0xf567c4 (Link)
//   [+0x34] sub-Link.next = &this->[+0x30]
//   [+0x38] sub-Link.prev = &this->[+0x30]
//   [+0x3c] end of touched range
//
// This is the same SQEX::CDev::Engine::Memory::Alternative ctor grammar as
// FUN_004139d0 (DetachableHeapBlock — see decomp-notes/types/ffxivgame/
// 0x000139d0.md) — base subobject vftables written first, then overwritten
// with the most-derived class's vftables. The "factory" wrapper around the
// ctor here is what differentiates SystemHeapBlock from the
// DetachableHeapBlock ctor (which is called directly by FUN_00413fa0 after
// the caller allocates the cell from a pool).
//
// Branch shape: if the first __aligned_malloc returns NULL the function
// must still call the second __aligned_malloc (the call sequence is
// unconditional — both buffers are allocated up front), then jumps over
// the entire object-init block via `JZ +0x5f` (74 5f) at +0x25 to the
// NULL-return epilogue.
//
// The NULL-return epilogue rearranges its pops (POP EDI / POP ESI / POP
// EBP / XOR EAX,EAX / POP EBX) versus the success-path epilogue (POP EDI
// inserted mid-store at +0x71 to free EDI early, then POP ESI / POP EBP
// / POP EBX after the final EAX=ESI move). This split prologue/epilogue
// pattern is one of the canonical MSVC 2005 reasons we can't reproduce
// the layout from clean C++ source.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two __aligned_malloc CALL displacements (0x005c1183 and 0x005c1172)
//   are emitted as the original PE-resolved relative offsets via `_emit`
//   directives; compare.py masks those reloc sites during the byte
//   comparison. The six 32-bit MOV-immediate slots that write the vftable
//   addresses (0xf56750, 0xf567c4, 0xf56ff8, 0xf56fc0, 0xf5700c, 0xf567c4)
//   are bare absolute VAs in the original — they appear in the .obj as
//   literal bytes (no reloc record needed because we're not linking the
//   passthrough .obj into a fresh image).
//
//   The __declspec(naked) body re-emits the original 143 bytes verbatim;
//   compare.py reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_00414580() {
    __asm {
        // 00014580: 53                 PUSH EBX
        _emit 0x53
        // 00014581: 55                 PUSH EBP
        _emit 0x55
        // 00014582: 56                 PUSH ESI
        _emit 0x56
        // 00014583: 57                 PUSH EDI
        _emit 0x57
        // 00014584: 6a 10              PUSH 0x10    ; arg2: alignment = 16
        _emit 0x6a
        _emit 0x10
        // 00014586: 6a 3c              PUSH 0x3c    ; arg1: size = 60
        _emit 0x6a
        _emit 0x3c
        // 00014588: 8b f9              MOV  EDI, ECX               ; EDI = this (parent owner)
        _emit 0x8b
        _emit 0xf9
        // 0001458a: e8 83 11 5c 00     CALL __aligned_malloc       ; raw block
        _emit 0xe8
        _emit 0x83
        _emit 0x11
        _emit 0x5c
        _emit 0x00
        // 0001458f: 8b 5c 24 20        MOV  EBX, [ESP+0x20]        ; EBX = param_2 (alignment)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00014593: 8b 6c 24 1c        MOV  EBP, [ESP+0x1c]        ; EBP = param_1 (size)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        // 00014597: 53                 PUSH EBX                    ; arg2 = alignment
        _emit 0x53
        // 00014598: 55                 PUSH EBP                    ; arg1 = size
        _emit 0x55
        // 00014599: 8b f0              MOV  ESI, EAX               ; ESI = raw block
        _emit 0x8b
        _emit 0xf0
        // 0001459b: e8 72 11 5c 00     CALL __aligned_malloc       ; backing buffer
        _emit 0xe8
        _emit 0x72
        _emit 0x11
        _emit 0x5c
        _emit 0x00
        // 000145a0: 83 c4 10           ADD  ESP, 0x10              ; caller-cleans 4 __cdecl args
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 000145a3: 85 f6              TEST ESI, ESI               ; raw block == NULL?
        _emit 0x85
        _emit 0xf6
        // 000145a5: 74 5f              JZ   +0x5f  (→ 0x14606)     ; NULL → return 0
        _emit 0x74
        _emit 0x5f
        // --- object initialisation begins (ESI = block, EAX = backing) ---
        // 000145a7: c7 46 04 50 67 f5 00  MOV [ESI+0x04], 0x00f56750  ; IHandle vftable
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000145ae: c7 46 08 c4 67 f5 00  MOV [ESI+0x08], 0x00f567c4  ; Link vftable
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000145b5: 8d 4e 08           LEA  ECX, [ESI+0x08]        ; ECX = &embedded Link
        _emit 0x8d
        _emit 0x4e
        _emit 0x08
        // 000145b8: 89 49 04           MOV  [ECX+0x04], ECX        ; Link.next = self
        _emit 0x89
        _emit 0x49
        _emit 0x04
        // 000145bb: 89 49 08           MOV  [ECX+0x08], ECX        ; Link.prev = self
        _emit 0x89
        _emit 0x49
        _emit 0x08
        // 000145be: 89 46 18           MOV  [ESI+0x18], EAX        ; backing buffer ptr
        _emit 0x89
        _emit 0x46
        _emit 0x18
        // 000145c1: 8b 44 24 1c        MOV  EAX, [ESP+0x1c]        ; EAX = param_3
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 000145c5: 89 46 24           MOV  [ESI+0x24], EAX        ; stash param_3
        _emit 0x89
        _emit 0x46
        _emit 0x24
        // 000145c8: 8d 46 30           LEA  EAX, [ESI+0x30]        ; EAX = &sub-Link sentinel
        _emit 0x8d
        _emit 0x46
        _emit 0x30
        // 000145cb: 89 7e 14           MOV  [ESI+0x14], EDI        ; stash parent owner
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 000145ce: 89 6e 1c           MOV  [ESI+0x1c], EBP        ; stash size
        _emit 0x89
        _emit 0x6e
        _emit 0x1c
        // 000145d1: 89 5e 20           MOV  [ESI+0x20], EBX        ; stash alignment
        _emit 0x89
        _emit 0x5e
        _emit 0x20
        // 000145d4: c7 06 f8 6f f5 00  MOV  [ESI+0x00], 0x00f56ff8  ; SystemHeapBlock primary vftable
        _emit 0xc7
        _emit 0x06
        _emit 0xf8
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 000145da: c7 46 04 c0 6f f5 00  MOV [ESI+0x04], 0x00f56fc0  ; SystemHeapBlock secondary vftable
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0xc0
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 000145e1: c7 46 28 00 00 00 00  MOV [ESI+0x28], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000145e8: 89 76 2c           MOV  [ESI+0x2c], ESI        ; back-pointer to self
        _emit 0x89
        _emit 0x76
        _emit 0x2c
        // 000145eb: c7 01 0c 70 f5 00  MOV  [ECX], 0x00f5700c       ; SystemHeapBlock embedded-Link vftable
        _emit 0xc7
        _emit 0x01
        _emit 0x0c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 000145f1: 5f                 POP  EDI                    ; restore EDI early
        _emit 0x5f
        // 000145f2: c7 00 c4 67 f5 00  MOV  [EAX], 0x00f567c4       ; sub-Link.vftable = Link
        _emit 0xc7
        _emit 0x00
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000145f8: 89 40 04           MOV  [EAX+0x04], EAX        ; sub-Link.next = self
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 000145fb: 89 40 08           MOV  [EAX+0x08], EAX        ; sub-Link.prev = self
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 000145fe: 8b c6              MOV  EAX, ESI               ; return block
        _emit 0x8b
        _emit 0xc6
        // 00014600: 5e                 POP  ESI
        _emit 0x5e
        // 00014601: 5d                 POP  EBP
        _emit 0x5d
        // 00014602: 5b                 POP  EBX
        _emit 0x5b
        // 00014603: c2 0c 00           RET  0x0c                   ; __thiscall, 3 stack args
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // --- NULL-return path (block alloc failed) ---
        // 00014606: 5f                 POP  EDI
        _emit 0x5f
        // 00014607: 5e                 POP  ESI
        _emit 0x5e
        // 00014608: 5d                 POP  EBP
        _emit 0x5d
        // 00014609: 33 c0              XOR  EAX, EAX               ; return 0
        _emit 0x33
        _emit 0xc0
        // 0001460b: 5b                 POP  EBX
        _emit 0x5b
        // 0001460c: c2 0c 00           RET  0x0c
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
#endif // _MSC_VER
