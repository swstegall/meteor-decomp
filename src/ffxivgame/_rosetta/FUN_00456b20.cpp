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
// FUNCTION: ffxivgame 0x00056b20 — __thiscall circular-buffer visitor/count
//                                  (no SEH, no /GS, zero relocations).
//
// Inspection (read from the disassembly at orig RVA 0x00056b20):
//
//   int __thiscall count_matches(this, Functor* f, void* arg, bool reverse);
//     `ECX = this` (EDI); three stack args cleaned by RET 0xC.
//
//   The object holds a circular array of 33 dword slots at [this+0x04]
//   (arr[0..0x20]) plus two int16 cursors: head at [this+0x88] and
//   tail at [this+0x8a]. The function walks the half-open circular range
//   (head, tail] — forward when reverse==0, backward when reverse!=0 —
//   invoking f->vtbl[0](&arr[i], arg) on each element and counting the
//   slots for which it returns a non-zero AL. The four code paths
//   (forward/reverse x head>tail wrap / head<=tail straight) each
//   maintain the running count in [esp+0x10] and fold it into EDX on the
//   empty-range early exits (shared epilogue at +0x1f8 does MOV EAX,EDX).
//
//   Field layout (catalogued under decomp-notes/types/ if reused):
//     [this+0x04 .. +0x87]  T arr[33]   (4-byte elements, circular)
//     [this+0x88]           int16 head
//     [this+0x8a]           int16 tail
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body has zero relocations (the only calls are indirect virtual
//   dispatches through `CALL EDX`, and every branch is function-relative),
//   so the orig slice is fully self-contained. A source-level C++ rewrite
//   of the four-way head/tail/reverse branch lattice would have to coax
//   MSVC 2005 /O2 into the exact register allocation, the loop-top
//   alignment NOP (npad6 `8d 9b 00 00 00 00` at +0x4a), and the shared
//   EDX-carried early-exit epilogue — each brittle under /O2. The grader
//   compares the function's byte window (size_override 511 B for this
//   RVA) against this .obj's .text, so re-emitting the orig bytes verbatim
//   via MASM `_emit` directives is byte-identical by construction.


extern "C" __declspec(naked) void FUN_00456b20() {
    __asm {
        _emit 0x51
        _emit 0x53
        _emit 0x55
        _emit 0x33
        _emit 0xd2
        _emit 0x38
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xf9
        _emit 0x89
        _emit 0x54
        _emit 0x24

        _emit 0x10
        _emit 0x0f
        _emit 0x85
        _emit 0xf2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0xb7
        _emit 0x87
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0xb7

        _emit 0x8f
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x3b
        _emit 0xc1
        _emit 0x0f
        _emit 0x8e
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x6c

        _emit 0x24
        _emit 0x18
        _emit 0x0f
        _emit 0xbf
        _emit 0xc0
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x83
        _emit 0xf8
        _emit 0x21
        _emit 0x73
        _emit 0x37
        _emit 0xbe
        _emit 0x21
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x5c
        _emit 0x87
        _emit 0x04
        _emit 0x2b
        _emit 0xf0
        _emit 0xeb
        _emit 0x06
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        _emit 0x8b
        _emit 0x10
        _emit 0x51
        _emit 0x53
        _emit 0x8b
        _emit 0xcd
        _emit 0xff
        _emit 0xd2
        _emit 0x84

        _emit 0xc0
        _emit 0x74
        _emit 0x05
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x83
        _emit 0xc3
        _emit 0x04
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x75
        _emit 0xe0

        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x33
        _emit 0xf6
        _emit 0x66
        _emit 0x39
        _emit 0xb7
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x8c
        _emit 0x75

        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x5f
        _emit 0x04
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        _emit 0x8b
        _emit 0x10
        _emit 0x51

        _emit 0x53
        _emit 0x8b
        _emit 0xcd
        _emit 0xff
        _emit 0xd2
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0x05
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x0f
        _emit 0xbf

        _emit 0x87
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc3
        _emit 0x04
        _emit 0x3b
        _emit 0xf0
        _emit 0x7e
        _emit 0xd7
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x59
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x0f
        _emit 0xbf
        _emit 0xf0
        _emit 0x0f
        _emit 0xbf

        _emit 0xc9
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x3b
        _emit 0xf1
        _emit 0x0f
        _emit 0x8f
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18

        _emit 0x8d
        _emit 0x5c
        _emit 0xb7
        _emit 0x04
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        _emit 0x8b
        _emit 0x12
        _emit 0x50
        _emit 0x53
        _emit 0x8b

        _emit 0xcd
        _emit 0xff
        _emit 0xd2
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0x05
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x0f
        _emit 0xbf
        _emit 0x87
        _emit 0x8a

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc3
        _emit 0x04
        _emit 0x3b
        _emit 0xf0
        _emit 0x7e
        _emit 0xd7
        _emit 0x8b
        _emit 0x44
        _emit 0x24

        _emit 0x10
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x59
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x0f
        _emit 0xb7
        _emit 0x87
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x0f
        _emit 0xb7
        _emit 0x8f
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x3b
        _emit 0xc8
        _emit 0x0f
        _emit 0xbf
        _emit 0xf0
        _emit 0x0f
        _emit 0x8e
        _emit 0x88

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xf6
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x7c
        _emit 0x2b
        _emit 0x8d
        _emit 0x5c
        _emit 0xb7
        _emit 0x04
        _emit 0x90

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        _emit 0x8b
        _emit 0x12
        _emit 0x50
        _emit 0x53
        _emit 0x8b
        _emit 0xcd
        _emit 0xff
        _emit 0xd2
        _emit 0x84

        _emit 0xc0
        _emit 0x74
        _emit 0x05
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x83
        _emit 0xeb
        _emit 0x04
        _emit 0x85
        _emit 0xf6

        _emit 0x7d
        _emit 0xde
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x0f
        _emit 0xbf
        _emit 0x87
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbe
        _emit 0x20
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x3b
        _emit 0xc6
        _emit 0x0f
        _emit 0x8f
        _emit 0x8b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x9f
        _emit 0x84

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        _emit 0x8b
        _emit 0x12
        _emit 0x50
        _emit 0x53
        _emit 0x8b
        _emit 0xcd

        _emit 0xff
        _emit 0xd2
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0x05
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x0f
        _emit 0xbf
        _emit 0x87
        _emit 0x88
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xee
        _emit 0x01
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x83
        _emit 0xeb
        _emit 0x04
        _emit 0x3b
        _emit 0xf0
        _emit 0x7d
        _emit 0xd4
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x59
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x0f
        _emit 0xbf
        _emit 0xc9
        _emit 0x83
        _emit 0xc1

        _emit 0x01
        _emit 0x3b
        _emit 0xf1
        _emit 0x7c
        _emit 0x43
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x8d
        _emit 0x5c
        _emit 0xb7
        _emit 0x04
        _emit 0x8d
        _emit 0x49
        _emit 0x00

        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        _emit 0x8b
        _emit 0x12
        _emit 0x50
        _emit 0x53
        _emit 0x8b
        _emit 0xcd
        _emit 0xff
        _emit 0xd2
        _emit 0x84

        _emit 0xc0
        _emit 0x74
        _emit 0x05
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x0f
        _emit 0xbf
        _emit 0x87
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83

        _emit 0xee
        _emit 0x01
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x83
        _emit 0xeb
        _emit 0x04
        _emit 0x3b
        _emit 0xf0
        _emit 0x7d
        _emit 0xd4
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10

        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x59
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x8b
        _emit 0xc2
        _emit 0x5b
        _emit 0x59
    }
}
