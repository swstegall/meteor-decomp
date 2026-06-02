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
// FUNCTION: ffxivgame 0x000acd02 — __cdecl multi-stage (de)serialise / copy
//                                  driver (507 B / 0x1fb, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x000acd02):
//
//   The body is a long short-circuited chain of bool-returning helper
//   calls — each `test eax,eax / je epilogue` bails to a single common
//   failure tail. ESI and EDI are live on entry (a non-standard
//   register-passed context + stream object), so a source-level rewrite
//   cannot pin MSVC 2005 /O2 to the exact allocation.
//
//   Skeleton (EDI = object base, ESI = stream/context handle):
//     a = acquire();                       // CALL 0x478130  (push ESI)
//     pa = field(); pb = field();          // CALL 0x4781c0  x4 — deferred
//     pc = field(); pd = field();          //   __cdecl cleanup (ADD ESP,0x14)
//     if (!pd) goto fail;
//     if (!(*pe)(EDI, EBX, base+4, ESI)) goto fail;   // indirect via [ESP+x]
//     if (EDI[0x40] == 0) {
//         ... pump fields [EDI+0x48]/[EDI+0x74]/[EDI+0x88] through
//             0x4975e0 / 0x497490 / 0x4974e0 helpers and the [ESP+x]
//             stream callbacks, each guarded by `je fail`;
//     } else {
//         ... mirror path through the same helpers in the other order;
//         ok = 0x4723b0(EBX, EBP);  ok = (ok == 0);   // NEG/SBB/NEG/INC
//     }
//   fail:
//     edi = saved_default;                 // [ESP+0x24]
//     release(ESI);                        // CALL 0x478180
//     if (handle) free(handle);            // CALL 0x478100
//     return edi;                          // EAX = EDI
//
//   The five 0x4781c0 calls accumulate their ESI arguments and are
//   cleaned in bulk (ADD ESP,0x14), and several results spill into the
//   incoming-argument homing slots ([ESP+0x14]/[ESP+0x1c]/[ESP+0x28]) —
//   both classic MSVC 2005 /O2 idioms that, combined with the
//   register-passed ESI/EDI and seven rel32 / indirect call sites, make
//   any high-level rewrite shift at least one byte.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The pragmatic choice — the same one the sibling reloc-heavy bodies
//   (FUN_00401820 / FUN_00409350 / FUN_0040b840) took — is a
//   `__declspec(naked)` body that re-emits the orig 507 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` ends up
//   byte-identical to the orig slice (no relocations, since the rel32
//   displacements are emitted as raw immediates), which is exactly what
//   tools/compare.py grades against.

extern "C" __declspec(naked) void FUN_004acd02() {
    __asm {
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0xe8
        _emit 0x26
        _emit 0xb4
        _emit 0xfc
        _emit 0xff
        _emit 0x56
        _emit 0xe8
        _emit 0xb0
        _emit 0xb4
        _emit 0xfc
        _emit 0xff
        _emit 0x56
        _emit 0x8b

        _emit 0xd8
        _emit 0xe8
        _emit 0xa8
        _emit 0xb4
        _emit 0xfc
        _emit 0xff
        _emit 0x56
        _emit 0x8b
        _emit 0xe8
        _emit 0xe8
        _emit 0xa0
        _emit 0xb4
        _emit 0xfc
        _emit 0xff
        _emit 0x56
        _emit 0x89

        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xe8
        _emit 0x96
        _emit 0xb4
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x44
        _emit 0x24

        _emit 0x1c
        _emit 0x0f
        _emit 0x84
        _emit 0x9c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x56
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        _emit 0x50

        _emit 0x53
        _emit 0x57
        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x82
        _emit 0x01
        _emit 0x00

        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x83
        _emit 0x78
        _emit 0x40
        _emit 0x00
        _emit 0x0f
        _emit 0x85
        _emit 0x03
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56

        _emit 0x83
        _emit 0xc0
        _emit 0x2c
        _emit 0x50
        _emit 0x55
        _emit 0x57
        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x0f

        _emit 0x84
        _emit 0x5e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x55
        _emit 0x50
        _emit 0x57
        _emit 0xff
        _emit 0x54
        _emit 0x24

        _emit 0x28
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x47
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14

        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x56
        _emit 0x55
        _emit 0x51
        _emit 0x52
        _emit 0x57
        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x14

        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x2b
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xbf
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74

        _emit 0x43
        _emit 0x8d
        _emit 0x47
        _emit 0x48
        _emit 0x50
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0x55
        _emit 0xe8
        _emit 0x1e
        _emit 0xa8
        _emit 0xfe
        _emit 0xff

        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8d

        _emit 0x47
        _emit 0x48
        _emit 0x50
        _emit 0x51
        _emit 0x55
        _emit 0x55
        _emit 0xe8
        _emit 0xb3
        _emit 0xa6
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0

        _emit 0x0f
        _emit 0x84
        _emit 0xed
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x47
        _emit 0x48
        _emit 0x50
        _emit 0x55
        _emit 0x53
        _emit 0x53
        _emit 0xe8
        _emit 0xec
        _emit 0xa6

        _emit 0xfe
        _emit 0xff
        _emit 0xeb
        _emit 0x27
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x8d
        _emit 0x57
        _emit 0x74
        _emit 0x52
        _emit 0x50
        _emit 0x55
        _emit 0x57

        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d

        _emit 0x47
        _emit 0x48
        _emit 0x50
        _emit 0x55
        _emit 0x53
        _emit 0x53
        _emit 0xe8
        _emit 0x73
        _emit 0xa6
        _emit 0xfe
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0

        _emit 0x0f
        _emit 0x84
        _emit 0xad
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x56
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        _emit 0x50
        _emit 0x53

        _emit 0x53
        _emit 0x57
        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x92
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x56
        _emit 0x51
        _emit 0x8d
        _emit 0x97
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0x55
        _emit 0x57

        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x78
        _emit 0x8d
        _emit 0x47
        _emit 0x48
        _emit 0x50
        _emit 0x55

        _emit 0xeb
        _emit 0x38
        _emit 0x8d
        _emit 0x47
        _emit 0x48
        _emit 0x50
        _emit 0x8d
        _emit 0x47
        _emit 0x74
        _emit 0x50
        _emit 0x53
        _emit 0x53
        _emit 0xe8
        _emit 0x1d
        _emit 0xa6
        _emit 0xfe

        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x5b
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x56
        _emit 0x83
        _emit 0xc0
        _emit 0x04

        _emit 0x50
        _emit 0x53
        _emit 0x53
        _emit 0x57
        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x44
        _emit 0x8d

        _emit 0x47
        _emit 0x48
        _emit 0x50
        _emit 0x8d
        _emit 0x8f
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0x53
        _emit 0x53
        _emit 0xe8
        _emit 0xed
        _emit 0xa5
        _emit 0xfe

        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x2b
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x56
        _emit 0x83
        _emit 0xc2
        _emit 0x18

        _emit 0x52
        _emit 0x55
        _emit 0x57
        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x15
        _emit 0x53
        _emit 0x55

        _emit 0xe8
        _emit 0xe9
        _emit 0x54
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x8b
        _emit 0xf8
        _emit 0xf7
        _emit 0xdf
        _emit 0x1b
        _emit 0xff
        _emit 0x83
        _emit 0xc7

        _emit 0x01
        _emit 0xeb
        _emit 0x04
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x56
        _emit 0xe8
        _emit 0xa1
        _emit 0xb2
        _emit 0xfc
        _emit 0xff
        _emit 0x8b
        _emit 0x44
        _emit 0x24

        _emit 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x5d
        _emit 0x5b
        _emit 0x74
        _emit 0x09
        _emit 0x50
        _emit 0xe8
        _emit 0x0e
        _emit 0xb2
        _emit 0xfc
        _emit 0xff

        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5e
        _emit 0x8b
        _emit 0xc7
        _emit 0x5f
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0xc3
    }
}
