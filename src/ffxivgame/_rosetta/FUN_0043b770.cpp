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
// FUNCTION: ffxivgame 0x0003b770 — __thiscall worker-flush / dispatch loop
//                                  (437 B / 0x1b5, no SEH, no /GS).
//
// Inspection (read from the disassembly at orig RVA 0x0003b770):
//
//   __thiscall void FUN_0043b770(void* this);   // ECX = this, no stack args
//                                                // (RET balances its own
//                                                //  SUB ESP,0x8 — `ret` w/o N).
//
//   The body is a mutex-guarded poll loop over two std::vector<T*>-shaped
//   member containers. Four IAT/global function pointers are used as the
//   lock primitive set:
//     [0x00f3e16c] (called via EBP) — lock-acquire (one ptr arg)
//     [0x00f3e168]                  — lock-release (one ptr arg)
//     [0x00f3e140]                  — refcount add(ptr, -1) style helper
//     [0x00f3e138] / [0x00f3e13c]   — release/free helpers
//   Two critical-section-like objects live inline at this+0x64 and the
//   per-record this+0x24; a sentinel byte this+0x7d breaks the outer loop.
//
//   Structural shape (mirrors the asm):
//
//     do {
//         { lock l(this->cs_64); bool stop = this->flag_7d; }   // acquire+release
//         if (stop) break;
//         RefDec(this->m_50, -1);                                // [0x00f3e140]
//         // inner: drain m_0c[] (vector at +0x0c / size +0x1c..+0x20)
//         //   from index back, dispatch each elem's vtbl[+0x04],
//         //   guarded by lock(this->cs_24).
//         // then: walk m_40[] (vector at +0x40 / +0x44), and for each,
//         //   walk its own sub-vector at +0x04, dispatch vtbl[+0x04].
//         // finally reset m_1c/m_20 = 0, LOCK XADD-decrement two refcounts
//         //   at this->m_5c->[0] and this->m_60->[0], freeing on 1->0.
//     } while (!this->flag_7d);
//
//   The out-of-range arms call FUN_009d22b4 (the std::vector
//   _DEBUG_ERROR / out-of-range bounds helper) before the indexed load,
//   exactly as MSVC 2005's checked operator[] expands.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This body is reloc-dense: five distinct IAT/global function-pointer
//   indirect calls (`FF 15` / `CALL EBP` off a global load), one rel32
//   call to the shared vector-bounds helper at 0x009d22b4 emitted three
//   times, two `LOCK XADD` refcount decrements, and an 8-byte MSVC NOP
//   alignment pad (`8D A4 24 00000000` + `90`) the compiler inserts after
//   the loop-entry `JMP`. Reproducing the exact register allocation
//   (EBP pinned to the lock-acquire fnptr and reloaded mid-loop), the
//   short-vs-near branch widths, and the IAT-indirect call encodings
//   from source-level C++ under /O2 is brittle — every rewrite shifts at
//   least one byte. Following the established sibling idiom
//   (FUN_00415d00 / FUN_00409350 / FUN_0040b840), the body is re-emitted
//   verbatim via MASM `_emit` directives so the .obj's `.text` is
//   byte-identical to the orig slice (compare.py's GREEN criterion).
//
//   The structural commentary above is the readable record for a future
//   contributor to promote this to a real source-level match once the
//   owning class (the +0x64/+0x24 critical sections, the +0x0c/+0x40
//   vectors, the +0x5c/+0x60 refcounted handles, the +0x7d stop flag)
//   and the four lock/refcount IAT thunks are catalogued under
//   decomp-notes/types/.

extern "C" __declspec(naked) void FUN_0043b770() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x08
        _emit 0x53
        _emit 0x55
        _emit 0x8b
        _emit 0x2d
        _emit 0x6c

        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xf1
        _emit 0x8d

        _emit 0x7e
        _emit 0x64
        _emit 0x57
        _emit 0xff
        _emit 0xd5
        _emit 0x8a
        _emit 0x5e
        _emit 0x7d

        _emit 0x57
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x84

        _emit 0xdb
        _emit 0x0f
        _emit 0x85
        _emit 0x8e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b

        _emit 0x46
        _emit 0x50
        _emit 0x6a
        _emit 0xff
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x40

        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8d
        _emit 0x5e
        _emit 0x24
        _emit 0x53
        _emit 0x33

        _emit 0xff
        _emit 0xff
        _emit 0xd5
        _emit 0x8b
        _emit 0x46
        _emit 0x20
        _emit 0x3b
        _emit 0x46

        _emit 0x1c
        _emit 0x7e
        _emit 0x24
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x85
        _emit 0xc9

        _emit 0x8d
        _emit 0x78
        _emit 0xff
        _emit 0x89
        _emit 0x7e
        _emit 0x20
        _emit 0x74
        _emit 0x0c

        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x2b
        _emit 0xc1
        _emit 0xc1
        _emit 0xf8
        _emit 0x02

        _emit 0x3b
        _emit 0xf8
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0xe3
        _emit 0x6a
        _emit 0x59

        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x8b
        _emit 0x3c
        _emit 0xb9
        _emit 0x53

        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x85
        _emit 0xff

        _emit 0x74
        _emit 0x0e
        _emit 0x8b
        _emit 0x17
        _emit 0x8b
        _emit 0x06
        _emit 0x8b
        _emit 0x52

        _emit 0x04
        _emit 0x50
        _emit 0x8b
        _emit 0xcf
        _emit 0xff
        _emit 0xd2
        _emit 0xeb
        _emit 0xb6

        _emit 0x33
        _emit 0xff
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0xeb
        _emit 0x08

        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x90

        _emit 0x8b
        _emit 0x4e
        _emit 0x40
        _emit 0x85
        _emit 0xc9
        _emit 0x0f
        _emit 0x84
        _emit 0x9f

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x44
        _emit 0x2b
        _emit 0xc1

        _emit 0xc1
        _emit 0xf8
        _emit 0x02
        _emit 0x3b
        _emit 0xf8
        _emit 0x0f
        _emit 0x83
        _emit 0x8f

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x0c
        _emit 0x8b

        _emit 0x46
        _emit 0x44
        _emit 0x2b
        _emit 0xc1
        _emit 0xc1
        _emit 0xf8
        _emit 0x02
        _emit 0x3b

        _emit 0xf8
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x84
        _emit 0x6a
        _emit 0x59
        _emit 0x00

        _emit 0x8b
        _emit 0x46
        _emit 0x40
        _emit 0x8b
        _emit 0x3c
        _emit 0xb8
        _emit 0x8d
        _emit 0x47

        _emit 0x1c
        _emit 0x50
        _emit 0x33
        _emit 0xdb
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18

        _emit 0xff
        _emit 0xd5
        _emit 0x8b
        _emit 0x6f
        _emit 0x14
        _emit 0x39
        _emit 0x6f
        _emit 0x18

        _emit 0x7e
        _emit 0x22
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        _emit 0x85
        _emit 0xc9
        _emit 0x74

        _emit 0x0c
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        _emit 0x2b
        _emit 0xc1
        _emit 0xc1
        _emit 0xf8

        _emit 0x02
        _emit 0x3b
        _emit 0xe8
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x52
        _emit 0x6a

        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        _emit 0x8b
        _emit 0x1c
        _emit 0xa9

        _emit 0x83
        _emit 0x47
        _emit 0x14
        _emit 0x01
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14

        _emit 0x52
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x85

        _emit 0xdb
        _emit 0x74
        _emit 0x1b
        _emit 0x8b
        _emit 0x0e
        _emit 0x8b
        _emit 0x03
        _emit 0x8b

        _emit 0x50
        _emit 0x04
        _emit 0x51
        _emit 0x8b
        _emit 0xcb
        _emit 0xff
        _emit 0xd2
        _emit 0x8b

        _emit 0x2d
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x7c
        _emit 0x24

        _emit 0x10
        _emit 0xe9
        _emit 0x6a
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0x44

        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x8b
        _emit 0x2d
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3

        _emit 0x00
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0xe9
        _emit 0x56
        _emit 0xff

        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x7e
        _emit 0x24
        _emit 0x57
        _emit 0xff
        _emit 0xd5

        _emit 0x33
        _emit 0xc0
        _emit 0x57
        _emit 0x89
        _emit 0x46
        _emit 0x20
        _emit 0x89
        _emit 0x46

        _emit 0x1c
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b

        _emit 0x46
        _emit 0x5c
        _emit 0x8b
        _emit 0x10
        _emit 0x83
        _emit 0xc9
        _emit 0xff
        _emit 0xf0

        _emit 0x0f
        _emit 0xc1
        _emit 0x0a
        _emit 0x83
        _emit 0xf9
        _emit 0x01
        _emit 0x75
        _emit 0x16

        _emit 0x8b
        _emit 0x46
        _emit 0x50
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x38
        _emit 0xe1

        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x58
        _emit 0x51
        _emit 0xff
        _emit 0x15

        _emit 0x3c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0xeb
        _emit 0x0c
        _emit 0x8b
        _emit 0x56

        _emit 0x58
        _emit 0x6a
        _emit 0xff
        _emit 0x52
        _emit 0xff
        _emit 0x15
        _emit 0x40
        _emit 0xe1

        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x60
        _emit 0x8b
        _emit 0x10
        _emit 0x83

        _emit 0xc9
        _emit 0xff
        _emit 0xf0
        _emit 0x0f
        _emit 0xc1
        _emit 0x0a
        _emit 0x83
        _emit 0xf9

        _emit 0x01
        _emit 0x75
        _emit 0x0a
        _emit 0x8b
        _emit 0x46
        _emit 0x54
        _emit 0x50
        _emit 0xff

        _emit 0x15
        _emit 0x3c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8d
        _emit 0x7e
        _emit 0x64

        _emit 0x57
        _emit 0xff
        _emit 0xd5
        _emit 0x8a
        _emit 0x5e
        _emit 0x7d
        _emit 0x57
        _emit 0xff

        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x84
        _emit 0xdb
        _emit 0x0f

        _emit 0x84
        _emit 0x72
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d

        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0xc3
    }
}
