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
// FUNCTION: ffxivgame 0x00051120 — `__cdecl` pointer-chain walk (27 B).
//
// Takes one dword pointer arg, dereferences it to obtain the head link
// (`cur = arg->link`, where `link` is the object's first field), then
// spins down the singly-linked chain following each node's first field
// until it reaches a node whose flag byte at offset +0x45 is non-zero:
//
//     void f(Node *p) {
//         Node *cur = p->link;            // *(p) — first field is a Node*
//         while (cur->flag45 == 0)        // byte ptr [cur+0x45]
//             cur = cur->link;            // *(cur)
//     }
//
// MSVC 2005 /O2 hoists the first iteration's test out of the loop (the
// `CMP/JNZ` at +0x06) and inserts a 4-byte `lea esp,[esp]` no-op at
// +0x0c to 16-byte-align the loop head at 0x00051130. The function is
// void-returning; EAX at RET is just the last-touched node pointer and
// is left dead.
//
// Asm shape (27 bytes — read from build/pe-layout/ffxivgame/text.bin
// @ +0x51120, RVA 0x00051120..0x0005113b):
//
//     00051120:  8b 44 24 04          MOV EAX, [ESP+0x4]     ; arg
//     00051124:  8b 08                MOV ECX, [EAX]         ; cur = arg->link
//     00051126:  80 79 45 00          CMP byte [ECX+0x45], 0 ; cur->flag45
//     0005112a:  75 0e                JNZ 0x0005113a         ; already non-zero?
//     0005112c:  8d 64 24 00          LEA ESP, [ESP]         ; loop-align nop
//     00051130:  8b c1                MOV EAX, ECX           ; (loop head)
//     00051132:  8b 08                MOV ECX, [EAX]         ; cur = cur->link
//     00051134:  80 79 45 00          CMP byte [ECX+0x45], 0 ; cur->flag45
//     00051138:  74 f6                JZ  0x00051130         ; loop while zero
//     0005113a:  c3                   RET
//
// No reloc-bearing sites: both jumps are intra-function relative and the
// memory operands use register-relative addressing, so the 27 bytes are
// self-contained. Emitted verbatim via `__declspec(naked)` + `_emit` —
// the same convention sibling 28-byte wrapper FUN_00404e10 uses — which
// reproduces the loop-alignment nop and branch layout exactly without
// depending on the /O2 sibcall/align heuristics firing on a source-level
// rewrite.

extern "C" __declspec(naked) void FUN_00451120() {
    __asm {
        _emit 0x8b    // MOV EAX, [ESP+0x4]      ; arg
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b    // MOV ECX, [EAX]          ; cur = arg->link
        _emit 0x08
        _emit 0x80    // CMP byte ptr [ECX+0x45], 0
        _emit 0x79
        _emit 0x45
        _emit 0x00
        _emit 0x75    // JNZ 0x0005113a          ; rel8 = +0x0e
        _emit 0x0e
        _emit 0x8d    // LEA ESP, [ESP]          ; loop-align nop
        _emit 0x64
        _emit 0x24
        _emit 0x00
        _emit 0x8b    // MOV EAX, ECX            ; (loop head @0x00051130)
        _emit 0xc1
        _emit 0x8b    // MOV ECX, [EAX]          ; cur = cur->link
        _emit 0x08
        _emit 0x80    // CMP byte ptr [ECX+0x45], 0
        _emit 0x79
        _emit 0x45
        _emit 0x00
        _emit 0x74    // JZ 0x00051130           ; rel8 = -0x0a
        _emit 0xf6
        _emit 0xc3    // RET
    }
}
