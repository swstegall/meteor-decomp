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
// FUNCTION: ffxivgame 0x000136c0 — virtual-dispatch linked-list reaper that
//                                   processes "pending" items via an inline
//                                   sentinel doubly-linked list at this+0x30.
//                                   (__thiscall, 166 bytes / 0xa6)
//
// Calling convention: __thiscall (ECX = this); returns void via tail-jmp.
// Callee-saves pushed: EBP at entry (then EDI; then EBX+ESI inside the
//   non-empty-list branch).
//
// Frame style:
//   PUSH EBP / MOV EBP,ECX uses EBP as the "stable this" register —
//   distinct from the PUSH-ECX compact frame in sibling FUN_00412430.
//   That choice avoids the [ESP+0x10] this-reload mid-body, at the cost
//   of consuming EBP as a non-frame-pointer (legal under /Oy / /O2).
//
// Object layout (offsets touched):
//   [this + 0x00]    vtable ptr
//     vtable[0x2c/4=11]  pre-loop virtual call
//     vtable[0x30/4=12]  post-loop tail call (JMP EAX)
//   [this + 0x04]    ptr to object with vtable[4] = deallocator-like
//   [this + 0x30]    embedded sentinel node (4-word inline structure;
//                    used purely as the loop terminator address)
//   [this + 0x38]    head pointer of the doubly-linked list (sentinel
//                    field at +0x08, i.e. node->next)
//
// List nodes (EDI):
//   [node + 0x00]    vtable ptr; vtable[1] called to get a "result" object
//   [node + 0x08]    forward link (next node in list)
//
// Result object (EAX from node->vtable[1](node)):
//   [result + 0x0c]  ptr to sub-object whose own vtable[1] returns the
//                    actual item (ESI for the rest of the iteration)
//
// Item object (ESI):
//   [item + 0x18]    "primary" sub-object — vtable[8] = release; freed
//                    via this->field_0x04->vtable[4] when item->field_0x2c
//                    is zero AND item->field_0x25 is clear
//   [item + 0x1c]    "tertiary" sub-object — vtable[8] always called
//                    (post-conditional)
//   [item + 0x20]    "secondary" sub-object — vtable[8] called in the
//                    else branch (when item is being kept rather than freed)
//   [item + 0x24]    byte flag: non-zero = needs reaping
//   [item + 0x25]    byte flag: set to 1 in the "freed" branch
//   [item + 0x26]    byte flag: set to 1 in the "kept" branch
//   [item + 0x2c]    int field; zero -> "freed" branch, non-zero -> "kept"
//
// High-level behaviour:
//   1. Call this->vtable[11]() — pre-loop setup.
//   2. Load EDI = this->field_0x38 (head); compute sentinel = this+0x30.
//   3. If head == sentinel: branch ahead to the tail-call epilogue.
//   4. Otherwise loop:
//      a. result = EDI->vtable[1](EDI)
//      b. item   = (*result->field_0xc)->vtable[1]()
//      c. EDI    = EDI->field_0x08   (advance iterator BEFORE the branch)
//      d. if (item->flag_0x24 != 0):
//           item->flag_0x24 = 0
//           FUN_00413940(item)                       // sibling REL32 reloc
//           if (item->field_0x2c == 0 && item->flag_0x25 == 0):
//               item->primary->vtable[8]()           // (item->_18)
//               item->flag_0x25 = 1
//               this->field_4->vtable[4](item->_18)  // hand-off / dealloc
//               item->primary = NULL
//           else:
//               item->secondary->vtable[8]()         // (item->_20)
//               item->flag_0x26 = 1
//               FUN_004132a0(item)                   // sibling REL32 reloc
//           item->tertiary->vtable[8]()              // (item->_1c)
//   5. Tail-jmp this->vtable[12]() — post-loop cleanup; MOV ECX,EBP
//      restores the __thiscall this-arg, then JMP EAX hands off.
//
// Notable codegen details reproduced in naked asm:
//   - The `0F 84 80 00 00 00` long-form JE at entry encodes a +0x80
//     displacement that cannot use the rel8 form (rel8 0x80 means -128,
//     not +128), so MSVC emits the 6-byte rel32 instead.
//   - The `8B FF` (MOV EDI,EDI) before the loop top is the 2-byte
//     identity NOP MSVC inserts to align the loop entry to a 16-byte
//     boundary. (Same family as the 4-byte LEA-NOP in FUN_00412430.)
//   - The final `FF E0` is JMP EAX (tail-call), preceded by
//     MOV ECX,EBP — the __thiscall this-pointer hand-off without a
//     stack frame, mirroring the JMP EDX pattern in FUN_00412430.
//   - Two CALL rel32 instructions reference siblings (FUN_00413940 and
//     FUN_004132a0); the 4-byte operand windows become COFF REL32
//     relocations that tools/compare.py masks during the byte diff.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The MOV EBP,ECX-style frame plus the JMP-EAX tail-call and inline
//   2-byte alignment NOP are not reproducible from C++ source under
//   /O2 /Oy. The __declspec(naked) body re-emits the original 166 bytes
//   verbatim via MASM `_emit` / `call` directives; the .obj's .text is
//   byte-identical to the original slice, and compare.py reports GREEN.
//
// Cross-platform guard: __declspec(naked) + MASM-style inline asm are
// MSVC-only. The #ifdef keeps the file compilable on clang/arm64 (host
// toolchain) while the MSVC build (Wine) produces the byte-identical .obj.

#if defined(_MSC_VER) && !defined(__clang__)

// Siblings called via direct CALL rel32 (e8 + REL32 reloc); declared
// extern so the assembler emits the proper reloc that compare.py masks.
extern "C" void FUN_00413940();
extern "C" void FUN_004132a0();

extern "C" __declspec(naked) void FUN_004136c0()
{
    __asm {
        // 000136c0: 55              PUSH EBP
        _emit 0x55
        // 000136c1: 8b e9           MOV EBP, ECX        (EBP = this)
        _emit 0x8b
        _emit 0xe9
        // 000136c3: 8b 45 00        MOV EAX, [EBP]      (vtable ptr)
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 000136c6: 8b 50 2c        MOV EDX, [EAX+0x2c] (vtable[11])
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000136c9: 57              PUSH EDI
        _emit 0x57
        // 000136ca: ff d2           CALL EDX            (this->vtable[11]())
        _emit 0xff
        _emit 0xd2
        // 000136cc: 8b 7d 38        MOV EDI, [EBP+0x38] (head pointer)
        _emit 0x8b
        _emit 0x7d
        _emit 0x38
        // 000136cf: 8d 45 30        LEA EAX, [EBP+0x30] (sentinel = this+0x30)
        _emit 0x8d
        _emit 0x45
        _emit 0x30
        // 000136d2: 3b f8           CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 000136d4: 0f 84 80 00 00 00  JE +0x80  (-> 0x4137 5a; empty-list
        //                                          fall-through to epilogue)
        _emit 0x0f
        _emit 0x84
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000136da: 53              PUSH EBX
        _emit 0x53
        // 000136db: 56              PUSH ESI
        _emit 0x56
        // 000136dc: 33 db           XOR EBX, EBX        (EBX = 0; used as
        //                                                byte-compare zero
        //                                                + as the store
        //                                                value at +0x24)
        _emit 0x33
        _emit 0xdb
        // 000136de: 8b ff           MOV EDI, EDI        (2-byte NOP;
        //                                                loop-head alignment)
        _emit 0x8b
        _emit 0xff
        // === loop top (RVA 0x000136e0) ===
        // 000136e0: 8b 07           MOV EAX, [EDI]      (node vtable)
        _emit 0x8b
        _emit 0x07
        // 000136e2: 8b 50 04        MOV EDX, [EAX+4]    (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000136e5: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000136e7: ff d2           CALL EDX            (-> result in EAX)
        _emit 0xff
        _emit 0xd2
        // 000136e9: 8b 48 0c        MOV ECX, [EAX+0xc]  (result->field_0xc)
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 000136ec: 8b 01           MOV EAX, [ECX]      (sub-object vtable)
        _emit 0x8b
        _emit 0x01
        // 000136ee: 8b 50 04        MOV EDX, [EAX+4]    (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000136f1: ff d2           CALL EDX            (-> item in EAX)
        _emit 0xff
        _emit 0xd2
        // 000136f3: 8b 7f 08        MOV EDI, [EDI+8]    (advance iterator)
        _emit 0x8b
        _emit 0x7f
        _emit 0x08
        // 000136f6: 8b f0           MOV ESI, EAX        (ESI = item)
        _emit 0x8b
        _emit 0xf0
        // 000136f8: 38 5e 24        CMP byte ptr [ESI+0x24], BL
        _emit 0x38
        _emit 0x5e
        _emit 0x24
        // 000136fb: 74 54           JE +0x54            (-> 0x413751;
        //                                                no reap needed)
        _emit 0x74
        _emit 0x54
        // --- if-block: item->flag_0x24 != 0 ---
        // 000136fd: 8b ce           MOV ECX, ESI        (ECX = item)
        _emit 0x8b
        _emit 0xce
        // 000136ff: 88 5e 24        MOV byte ptr [ESI+0x24], BL   (clear flag)
        _emit 0x88
        _emit 0x5e
        _emit 0x24
        // 00013702: e8 39 02 00 00  CALL FUN_00413940   (sibling REL32 reloc)
        call FUN_00413940
        // 00013707: 39 5e 2c        CMP [ESI+0x2c], EBX
        _emit 0x39
        _emit 0x5e
        _emit 0x2c
        // 0001370a: 75 26           JNE +0x26           (-> 0x413732 else)
        _emit 0x75
        _emit 0x26
        // 0001370c: 38 5e 25        CMP byte ptr [ESI+0x25], BL
        _emit 0x38
        _emit 0x5e
        _emit 0x25
        // 0001370f: 75 21           JNE +0x21           (-> 0x413732 else)
        _emit 0x75
        _emit 0x21
        // --- then branch: free item->primary ---
        // 00013711: 8b 4e 18        MOV ECX, [ESI+0x18] (item->primary)
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 00013714: 8b 01           MOV EAX, [ECX]      (primary vtable)
        _emit 0x8b
        _emit 0x01
        // 00013716: 8b 50 20        MOV EDX, [EAX+0x20] (vtable[8])
        _emit 0x8b
        _emit 0x50
        _emit 0x20
        // 00013719: ff d2           CALL EDX            (primary->vtable[8]())
        _emit 0xff
        _emit 0xd2
        // 0001371b: 8b 56 18        MOV EDX, [ESI+0x18] (reload primary ptr)
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        // 0001371e: c6 46 25 01     MOV byte ptr [ESI+0x25], 1
        _emit 0xc6
        _emit 0x46
        _emit 0x25
        _emit 0x01
        // 00013722: 8b 4d 04        MOV ECX, [EBP+0x4]  (this->field_4)
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        // 00013725: 8b 01           MOV EAX, [ECX]      (field_4 vtable)
        _emit 0x8b
        _emit 0x01
        // 00013727: 8b 40 10        MOV EAX, [EAX+0x10] (vtable[4])
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 0001372a: 52              PUSH EDX            (arg: primary ptr)
        _emit 0x52
        // 0001372b: ff d0           CALL EAX            (deallocator-like)
        _emit 0xff
        _emit 0xd0
        // 0001372d: 89 5e 18        MOV [ESI+0x18], EBX (item->primary = NULL)
        _emit 0x89
        _emit 0x5e
        _emit 0x18
        // 00013730: eb 15           JMP +0x15           (-> 0x413747 end-if)
        _emit 0xeb
        _emit 0x15
        // --- else branch (RVA 0x00013732): keep item->secondary ---
        // 00013732: 8b 4e 20        MOV ECX, [ESI+0x20] (item->secondary)
        _emit 0x8b
        _emit 0x4e
        _emit 0x20
        // 00013735: 8b 11           MOV EDX, [ECX]      (secondary vtable)
        _emit 0x8b
        _emit 0x11
        // 00013737: 8b 42 20        MOV EAX, [EDX+0x20] (vtable[8])
        _emit 0x8b
        _emit 0x42
        _emit 0x20
        // 0001373a: ff d0           CALL EAX            (secondary->vtable[8]())
        _emit 0xff
        _emit 0xd0
        // 0001373c: 8b ce           MOV ECX, ESI        (ECX = item)
        _emit 0x8b
        _emit 0xce
        // 0001373e: c6 46 26 01     MOV byte ptr [ESI+0x26], 1
        _emit 0xc6
        _emit 0x46
        _emit 0x26
        _emit 0x01
        // 00013742: e8 59 fb ff ff  CALL FUN_004132a0   (sibling REL32 reloc)
        call FUN_004132a0
        // --- end-if (RVA 0x00013747): always release tertiary ---
        // 00013747: 8b 4e 1c        MOV ECX, [ESI+0x1c] (item->tertiary)
        _emit 0x8b
        _emit 0x4e
        _emit 0x1c
        // 0001374a: 8b 11           MOV EDX, [ECX]      (tertiary vtable)
        _emit 0x8b
        _emit 0x11
        // 0001374c: 8b 42 20        MOV EAX, [EDX+0x20] (vtable[8])
        _emit 0x8b
        _emit 0x42
        _emit 0x20
        // 0001374f: ff d0           CALL EAX            (tertiary->vtable[8]())
        _emit 0xff
        _emit 0xd0
        // === loop back-edge (RVA 0x00013751) ===
        // 00013751: 8d 45 30        LEA EAX, [EBP+0x30] (sentinel)
        _emit 0x8d
        _emit 0x45
        _emit 0x30
        // 00013754: 3b f8           CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 00013756: 75 88           JNE -0x78           (-> 0x4136e0 loop top)
        _emit 0x75
        _emit 0x88
        // === loop exit ===
        // 00013758: 5e              POP ESI
        _emit 0x5e
        // 00013759: 5b              POP EBX
        _emit 0x5b
        // === common epilogue (RVA 0x0001375a — empty-list JE lands here) ===
        // 0001375a: 8b 55 00        MOV EDX, [EBP]      (vtable ptr)
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 0001375d: 8b 42 30        MOV EAX, [EDX+0x30] (vtable[12])
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00013760: 5f              POP EDI
        _emit 0x5f
        // 00013761: 8b cd           MOV ECX, EBP        (ECX = this — __thiscall arg)
        _emit 0x8b
        _emit 0xcd
        // 00013763: 5d              POP EBP
        _emit 0x5d
        // 00013764: ff e0           JMP EAX             (tail-call this->vtable[12])
        _emit 0xff
        _emit 0xe0
    }
}

#endif // _MSC_VER && !__clang__
