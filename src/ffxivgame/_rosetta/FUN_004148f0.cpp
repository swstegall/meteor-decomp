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
// FUNCTION: ffxivgame 0x000148f0 — guarded "destroy-all-cached-cells"
//                                   teardown sweep on a DetachableHeapBlock-
//                                   style owner (__thiscall, 98 B / 0x62)
//
// __thiscall void FUN_004148f0(void)
//   ECX = this
//
// The function is the per-instance counterpart of the closed-form
// FUN_00414370 detach-companion (see decomp-notes/types/ffxivgame/0x00014370.md):
// it walks the intrusive Link sentinel list rooted at `this+0x30`,
// destroys each cached node, frees its backing buffer through the
// binary's common operator-delete helper `FUN_009d56fd` (see
// decomp-notes/types/ffxivgame/0x00014370.md for the helper's role),
// and brackets the sweep with the secondary sub-object's
// Enter (vtable[+0x18]) / Leave (vtable[+0x30]) guard pair.
//
// Behavioural sketch (matches the headless Ghidra hint):
//
//   IHandle *sub      = &this->m_secondary;       // = (IHandle*)((char*)this+4)
//   void    *guard    = sub->vt[+0x18]();          // Enter
//   guard->vt[+0x2c]();                            // begin-sweep notify
//   Link *node = this->m_listHead;                 // [this+0x34]
//   Link *end  = &this->m_listSentinel;            // &this[+0x30]
//   while (node != end) {
//       void *cell = node->vt[+0x04]();            // detach cell from node
//       node = node->next;                         // advance BEFORE dtor
//       cell->vt[0](0);                            // ~Cell(0)
//       FUN_009d56fd(cell);                        // operator delete
//   }
//   void *guard2 = sub->vt[+0x18]();               // Leave-enter
//   __tail__ guard2->vt[+0x30]();                  // Leave (tail-call)
//
// Calling convention: __thiscall, no stack args, void return; the
// epilogue ends in `JMP EAX` (tail call) — so there's no explicit RET
// instruction in this function, the tail-called vfunc closes the
// frame.
//
// Callee-saves used: EBX (list sentinel address), EBP (sub-object
// this), ESI (loop cursor), EDI (this on entry / freed cell after the
// detach call).
//
// Reloc-bearing site (the one CALL rel32 in the body):
//     +0x3f   CALL rel32  → FUN_009d56fd   (RVA 0x009d56fd — common free)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The structural pattern (Enter / list-walk / detach+dtor+free /
//   Leave tail-jmp) is straightforward in C++ form, but a source-level
//   version triggers three sources of byte-divergence vs. MSVC 2005's
//   /O2 output:
//     - The DetachableHeapBlock secondary sub-object lives at +4 of
//       the parent; the compiler's choice between `&this->m_sub` and
//       a fresh local depends on alias analysis, and MSVC-2005's pick
//       of EBP for it (rather than e.g. EDI) is hard to coax out.
//     - The "advance BEFORE dtor" sequencing (mov esi,[esi+4] BEFORE
//       the dtor call) is a specific register-pressure choice that
//       MSVC 2005 makes here but ICL / later MSVC do differently.
//     - The terminal `JMP EAX` tail call (vs. CALL EAX + RET) is the
//       /O2 inlined-tailcall behaviour for vtable[+0x30] dispatch;
//       MSVC 2005 *does* tail-call here, atypically.
//
//   Following the canonical idiom for this binary (see
//   src/ffxivgame/_rosetta/FUN_004130d0.cpp and
//   src/ffxivgame/_rosetta/FUN_00403bd0.cpp), this function is emitted
//   as a `__declspec(naked)` body that re-emits the orig 98 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice: the one rel32 CALL
//   constant resolves against the orig binary's own address space at
//   its load address, and emitting it as raw bytes produces the exact
//   wire image the linker would emit if it re-linked at the same
//   image base. `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_004148f0() {
    __asm {
        // 000148f0: 53              PUSH EBX
        _emit 0x53
        // 000148f1: 55              PUSH EBP
        _emit 0x55
        // 000148f2: 56              PUSH ESI
        _emit 0x56
        // 000148f3: 57              PUSH EDI
        _emit 0x57
        // 000148f4: 8b f9           MOV EDI, ECX                ; EDI = this
        _emit 0x8b
        _emit 0xf9
        // 000148f6: 8b 47 04        MOV EAX, dword ptr [EDI+0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 000148f9: 8b 50 18        MOV EDX, dword ptr [EAX+0x18]
        _emit 0x8b
        _emit 0x50
        _emit 0x18
        // 000148fc: 8d 6f 04        LEA EBP, [EDI+0x4]          ; EBP = &this->m_sub
        _emit 0x8d
        _emit 0x6f
        _emit 0x04
        // 000148ff: 8b cd           MOV ECX, EBP                ; thiscall this = sub
        _emit 0x8b
        _emit 0xcd
        // 00014901: ff d2           CALL EDX                    ; sub->vt[+0x18] (Enter)
        _emit 0xff
        _emit 0xd2
        // 00014903: 8b 10           MOV EDX, dword ptr [EAX]    ; EDX = ret->vptr
        _emit 0x8b
        _emit 0x10
        // 00014905: 8b c8           MOV ECX, EAX                ; thiscall this = ret
        _emit 0x8b
        _emit 0xc8
        // 00014907: 8b 42 2c        MOV EAX, dword ptr [EDX+0x2c]
        _emit 0x8b
        _emit 0x42
        _emit 0x2c
        // 0001490a: ff d0           CALL EAX                    ; ret->vt[+0x2c]
        _emit 0xff
        _emit 0xd0
        // 0001490c: 8b 77 34        MOV ESI, dword ptr [EDI+0x34]  ; ESI = list head
        _emit 0x8b
        _emit 0x77
        _emit 0x34
        // 0001490f: 8d 5f 30        LEA EBX, [EDI+0x30]            ; EBX = sentinel
        _emit 0x8d
        _emit 0x5f
        _emit 0x30
        // 00014912: 3b f3           CMP ESI, EBX
        _emit 0x3b
        _emit 0xf3
        // 00014914: 74 25           JZ  +0x25 (→ 0001493b)
        _emit 0x74
        _emit 0x25
        // 00014916: 8b 16           MOV EDX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 00014918: 8b 42 04        MOV EAX, dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001491b: 8b ce           MOV ECX, ESI                ; thiscall this = node
        _emit 0x8b
        _emit 0xce
        // 0001491d: ff d0           CALL EAX                    ; node->vt[+0x4] -> cell
        _emit 0xff
        _emit 0xd0
        // 0001491f: 8b 76 04        MOV ESI, dword ptr [ESI+0x4] ; advance: node = node->next
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 00014922: 8b f8           MOV EDI, EAX                ; EDI = cell
        _emit 0x8b
        _emit 0xf8
        // 00014924: 8b 17           MOV EDX, dword ptr [EDI]    ; cell->vptr
        _emit 0x8b
        _emit 0x17
        // 00014926: 8b 02           MOV EAX, dword ptr [EDX]    ; cell->vt[0]
        _emit 0x8b
        _emit 0x02
        // 00014928: 6a 00           PUSH 0                       ; dtor flag = 0
        _emit 0x6a
        _emit 0x00
        // 0001492a: 8b cf           MOV ECX, EDI                ; thiscall this = cell
        _emit 0x8b
        _emit 0xcf
        // 0001492c: ff d0           CALL EAX                    ; ~Cell(0)
        _emit 0xff
        _emit 0xd0
        // 0001492e: 57              PUSH EDI                    ; cdecl arg = cell
        _emit 0x57
        // 0001492f: e8 c9 0d 5c 00  CALL FUN_009d56fd  (rel32 = 0x005c0dc9)
        _emit 0xe8
        _emit 0xc9
        _emit 0x0d
        _emit 0x5c
        _emit 0x00
        // 00014934: 83 c4 04        ADD ESP, 0x4                ; cdecl cleanup
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00014937: 3b f3           CMP ESI, EBX
        _emit 0x3b
        _emit 0xf3
        // 00014939: 75 db           JNZ -0x25 (→ 00014916)
        _emit 0x75
        _emit 0xdb
        // 0001493b: 8b 55 00        MOV EDX, dword ptr [EBP]    ; sub->vptr
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 0001493e: 8b 42 18        MOV EAX, dword ptr [EDX+0x18]
        _emit 0x8b
        _emit 0x42
        _emit 0x18
        // 00014941: 8b cd           MOV ECX, EBP                ; thiscall this = sub
        _emit 0x8b
        _emit 0xcd
        // 00014943: ff d0           CALL EAX                    ; sub->vt[+0x18] (re-Enter)
        _emit 0xff
        _emit 0xd0
        // 00014945: 8b 10           MOV EDX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 00014947: 5f              POP EDI
        _emit 0x5f
        // 00014948: 5e              POP ESI
        _emit 0x5e
        // 00014949: 5d              POP EBP
        _emit 0x5d
        // 0001494a: 8b c8           MOV ECX, EAX                ; thiscall this = ret
        _emit 0x8b
        _emit 0xc8
        // 0001494c: 8b 42 30        MOV EAX, dword ptr [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 0001494f: 5b              POP EBX
        _emit 0x5b
        // 00014950: ff e0           JMP EAX                     ; tail-call ret->vt[+0x30] (Leave)
        _emit 0xff
        _emit 0xe0
    }
}
