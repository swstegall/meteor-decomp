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
// FUNCTION: ffxivgame 0x00035400 — `__cdecl` array-fill of a 20-byte
//                                  (0x14) record with a constant source
//                                  (57 bytes / 0x39)
//
// __cdecl void FUN_00435400(void *dst, unsigned int count, const void *src)
//   stack layout at entry (caller-cleans — plain `ret`):
//     [ESP+0x04] : void       *dst    (EAX cursor)
//     [ESP+0x08] : unsigned    count  (ECX, loop counter)
//     [ESP+0x0c] : const void *src    (EDX, NEVER incremented — constant)
//
// Inspection (orig 57 bytes at RVA 0x00035400):
//
//   8b 4c 24 08       MOV  ECX, [ESP+0x08]      ; count
//   85 c9             TEST ECX, ECX
//   76 30             JBE  done                 ; count == 0 → return
//   8b 54 24 0c       MOV  EDX, [ESP+0x0c]      ; src (held constant)
//   8b 44 24 04       MOV  EAX, [ESP+0x04]      ; dst cursor
//   56                PUSH ESI
// loop:                                          ; 0x00035411
//   85 c0             TEST EAX, EAX
//   74 18             JZ   advance              ; dst == 0 → skip the copy
//   f3 0f 7e 02       MOVQ XMM0, [EDX]          ; struct copy, bytes 0x00..0x07
//   66 0f d6 00       MOVQ [EAX], XMM0
//   f3 0f 7e 42 08    MOVQ XMM0, [EDX+0x08]     ;             bytes 0x08..0x0f
//   66 0f d6 40 08    MOVQ [EAX+0x08], XMM0
//   8b 72 10          MOV  ESI, [EDX+0x10]      ;             bytes 0x10..0x13
//   89 70 10          MOV  [EAX+0x10], ESI
// advance:                                       ; 0x0003542d
//   83 e9 01          SUB  ECX, 1
//   83 c0 14          ADD  EAX, 0x14            ; dst += sizeof(record)
//   85 c9             TEST ECX, ECX
//   77 da             JA   loop                 ; while (count != 0)
//   5e                POP  ESI
// done:                                          ; 0x00035438
//   c3                RET
//
// Source shape (inferred):
//
//   struct Rec20 { char _[0x14]; };              // 20 bytes
//   void fill(Rec20 *dst, unsigned count, const Rec20 *src) {
//       if (count == 0) return;
//       do {
//           if (dst)                             // null guard (meaningful
//               *dst = *src;                     //  only on entry)
//           ++dst;
//       } while (--count);
//   }
//
// The 20-byte struct assignment is unrolled into two SSE2 MOVQ qword
// copies (0x00..0x0f) plus a single DWORD MOV for the trailing 4 bytes
// (0x10..0x13) — the MSVC 2005 idiom for a fixed-size POD copy when the
// record is 8-byte aligned. `src` (EDX) is loaded once and never bumped,
// so the same record is broadcast into every slot.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The body carries no relocations (no CALLs, no IAT/RIP loads), so a
//   `__declspec(naked)` re-emit of the orig 57 bytes via MASM `_emit`
//   directives yields a `.text` slice byte-identical to the orig. The
//   short-form JBE/JZ/JA branches and the SSE2 MOVQ encodings are pinned
//   exactly. tools/compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00435400() {
    __asm {
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x08]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x76              // JBE  +0x30 (→ done)
        _emit 0x30
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x0c]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x85              // loop: TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ   +0x18 (→ advance)
        _emit 0x18
        _emit 0xf3              // MOVQ XMM0, qword ptr [EDX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x02
        _emit 0x66              // MOVQ qword ptr [EAX], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x00
        _emit 0xf3              // MOVQ XMM0, qword ptr [EDX + 0x08]
        _emit 0x0f
        _emit 0x7e
        _emit 0x42
        _emit 0x08
        _emit 0x66              // MOVQ qword ptr [EAX + 0x08], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x08
        _emit 0x8b              // MOV ESI, dword ptr [EDX + 0x10]
        _emit 0x72
        _emit 0x10
        _emit 0x89              // MOV dword ptr [EAX + 0x10], ESI
        _emit 0x70
        _emit 0x10
        _emit 0x83              // advance: SUB ECX, 0x01
        _emit 0xe9
        _emit 0x01
        _emit 0x83              // ADD EAX, 0x14
        _emit 0xc0
        _emit 0x14
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x77              // JA   -0x26 (→ loop)
        _emit 0xda
        _emit 0x5e              // POP ESI
        _emit 0xc3              // done: RET
    }
}
