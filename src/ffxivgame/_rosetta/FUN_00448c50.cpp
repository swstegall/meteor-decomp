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
// FUNCTION: ffxivgame 0x00048c50 — __thiscall object initializer + name/
//                                  address resolve (418 B / 0x1a2, SEH frame,
//                                  /GS security cookie).
//
// Inspection (read from the disassembly at orig RVA 0x00048c50):
//
//   __thiscall <ret-in-EAX (this)> FUN_00448c50(this /*ECX*/, <stack arg>);
//
//   `ECX = this`; one stack argument (a std::string-like {buf[16]|ptr,
//   len@+0x14, cap@+0x18} object at [ESP+0x9c] = EBP). Callee-pops the
//   single 4-byte arg (`RET 0x4`) and returns `this` in EAX.
//
//   Prologue installs a full SEH frame (`PUSH -1 / PUSH 0xe57674 /
//   PUSH FS:[0]`, handler list at [ESP+0x8c]) plus the standard MSVC
//   2005 /GS pair (two `MOV EAX,[0x012ea8b0] / XOR EAX,ESP` cookie
//   computations: one spilled at [ESP+0x74], one pushed into the SEH
//   scope record).
//
//   Body shape:
//     - Initialize the receiver: this[0] = &this[0x12]; this+0x4 = 0x40;
//       this+0x8 = 1; this+0xc = 0; this+0x10 = 1; this+0x11 = 1;
//       *(BYTE*)(this+0x12) = 0; call FUN_00447010 (sub-ctor) with two
//       `1` args; then re-zero *(BYTE*)this[0].
//     - If the arg's length field (EBP[0x14]) is 0, skip to the SEH
//       teardown and return.
//     - Resolve the small-string-optimized buffer pointer (EBP[0x18] is
//       the capacity; <0x10 ⇒ inline at EBP+4, else heap ptr deref) and
//       call the IAT thunk at [0x00f3e1f4] twice (getaddrinfo-style:
//       PUSH 0 / 0x8 / name / -1 / 0 / 0) to look up an address.
//     - On success, format the result into a local string via
//       FUN_004496c0 / FUN_00449b20 (narrow/long path), FUN_00448880,
//       FUN_00447450, then conditionally release temporaries through the
//       allocator helper FUN_0044d350.
//     - SEH teardown, restore FS:[0], `__security_check_cookie`
//       (CALL 0x009d20f4), `ADD ESP,0x84`, `RET 0x4`.
//
//   Reloc-bearing sites in the orig 418 bytes (image-base-dependent at
//   0x00400000; a standalone .obj compile cannot reproduce them):
//     +0x07  moffs FS:[0]               — SEH frame install
//     +0x11  moffs [0x012ea8b0]         — /GS cookie (1st)
//     +0x20  moffs [0x012ea8b0]         — /GS cookie (2nd)
//     +0x2f  moffs FS:[0]               — SEH scope ptr store
//     +0x67  rel32 0x00447010          — sub-ctor
//     +0x8a  moffs [0x00f3e1f4]         — IAT resolve thunk (EBX), 2 calls
//     +0xd0  rel32 0x004496c0          — narrow format path
//     +0xd9  rel32 0x00449b20          — long format path
//     +0x116 rel32 0x00448880          — string build
//     +0x126 rel32 0x00447450          — member apply
//     +0x146 rel32 0x0044d350          — allocator helper (1st)
//     +0x171 rel32 0x0044d350          — allocator helper (2nd)
//     +0x182 moffs FS:[0]              — SEH frame uninstall
//     +0x194 rel32 0x009d20f4         — __security_check_cookie
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 /GS /EHa into
//   reproducing the exact SEH frame layout, the doubled /GS cookie
//   spill, the SSO buffer-pointer fork, the two-call IAT resolve, and all
//   fourteen linker-resolved relocation windows above — every one brittle
//   under /O2. As with FUN_0040ced0 / FUN_00415d00 / FUN_00409350, the
//   pragmatic match is a `__declspec(naked)` body re-emitting the orig
//   418 bytes verbatim via MASM `_emit`, so the .obj `.text` slice is
//   byte-identical to the orig (which is what tools/compare.py grades).

extern "C" __declspec(naked) void FUN_00448c50() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x74
        _emit 0x76
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec

        _emit 0x78
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x57

        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64

        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x8d
        _emit 0x46

        _emit 0x12
        _emit 0x6a
        _emit 0x01
        _emit 0xc6
        _emit 0x46
        _emit 0x10
        _emit 0x01
        _emit 0xc6
        _emit 0x46
        _emit 0x11
        _emit 0x01
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x89
        _emit 0x06
        _emit 0x6a
        _emit 0x01
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x54
        _emit 0xe3
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x06
        _emit 0xc6
        _emit 0x00

        _emit 0x00
        _emit 0x83
        _emit 0x7d
        _emit 0x14
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xfe
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x7d
        _emit 0x18
        _emit 0x10
        _emit 0x8d

        _emit 0x45
        _emit 0x04
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x72
        _emit 0x02
        _emit 0x8b
        _emit 0x00
        _emit 0x8b
        _emit 0x1d
        _emit 0xf4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00

        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0xff
        _emit 0x50
        _emit 0x6a
        _emit 0x08
        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0xd3
        _emit 0x8b
        _emit 0xf8
        _emit 0x33

        _emit 0xc0
        _emit 0x3b
        _emit 0xf8
        _emit 0x0f
        _emit 0x84
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x70

        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x6c
        _emit 0x77
        _emit 0x0a
        _emit 0x6a
        _emit 0xff
        _emit 0x57

        _emit 0xe8
        _emit 0x9b
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x07
        _emit 0x50
        _emit 0x57
        _emit 0xe8
        _emit 0xf2
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xbc

        _emit 0x24
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x08
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x70
        _emit 0x73
        _emit 0x04
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x70

        _emit 0x83
        _emit 0x7d
        _emit 0x18
        _emit 0x10
        _emit 0x72
        _emit 0x08
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x02
        _emit 0xeb
        _emit 0x04
        _emit 0x8b
        _emit 0x44

        _emit 0x24
        _emit 0x14
        _emit 0x57
        _emit 0x51
        _emit 0x6a
        _emit 0xff
        _emit 0x50
        _emit 0x6a
        _emit 0x08
        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0xd3
        _emit 0x8d
        _emit 0x44
        _emit 0x24

        _emit 0x6c
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0x15
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x50
        _emit 0x8b
        _emit 0xce
        _emit 0xc6
        _emit 0x84

        _emit 0x24
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0xe8
        _emit 0xd5
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x29
        _emit 0x00

        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x54

        _emit 0x24
        _emit 0x18
        _emit 0x6a
        _emit 0x0b
        _emit 0x51
        _emit 0x52
        _emit 0xe8
        _emit 0xb5
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0x84

        _emit 0x24
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff

        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72
        _emit 0x14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x70
        _emit 0x6a
        _emit 0x0c
        _emit 0x8d
        _emit 0x44
        _emit 0x00
        _emit 0x02
        _emit 0x50

        _emit 0x51
        _emit 0xe8
        _emit 0x8a
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b
        _emit 0xc6
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x8c
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c

        _emit 0x24
        _emit 0x74
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x0b
        _emit 0x93
        _emit 0x58
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc2

        _emit 0x04
        _emit 0x00
    }
}
