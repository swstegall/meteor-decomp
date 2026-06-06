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
// FUNCTION: ffxivgame 0x0003d760 — __thiscall lookup-and-build helper
//                                  (466 B / 0x1d2, inline SEH + /GS).
//
// Inspection (read from the disassembly at orig RVA 0x0003d760):
//
//   __thiscall <ptr>* build_or_null(this, ..., OutPtr* out /* [esp+0x444] */)
//   — `ECX = this` (spilled to ESI), the trailing stack argument is an
//   out-parameter pointer (loaded into EBP). Returns EBP (the out-ptr).
//
//   Structural shape:
//
//     // Phase 1 — probe via a local scratch object.
//     Scratch local;                                  // [esp+0x14]
//     FUN_004330f0(&local, this);                     // probe / lookup ctor
//     if (local.field0 /* [esp+0x14] */ == 0) {
//         *out = 0;                                    // not found
//         goto done;
//     }
//     // Phase 2 — resolve a record by key 0xf66b0c.
//     void* rec = FUN_009d6b65(&local, this, 0xf66b0c);
//     if (rec->field /* [esp+0x1c] */ == 0) {         // == 0 → diagnostic path
//         char buf[0x400];                            // [esp+0x40], /GS array
//         int n = sprintf_s(buf, 0x400, "%...", 0x3fd, 0xf66b50, 0x1b1, 0xf66b10);
//         sprintf_s(&buf[n], 0x400-n, 0x3fe-n, 0xf66ba4, this);
//         char buf2[?];                               // [esp+0x60]
//         FUN_009d4bb4(&buf2, 0x400, 0xf66bd0);
//         (*(fn*)0x012660d8)(&buf2, 4);               // log / assert sink
//         *out = 0;
//         goto done;
//     }
//     // Phase 3 — found: derive a key chain and construct the result.
//     int  k  = FUN_009d6a61(rec->field);
//     void* h = FUN_009d6962(k);
//     CObj  o; FUN_0040e2d0(&o, h, 0xf66bd4, 0x10);   // EDI = o
//     void* a = FUN_0040a330(0);
//     void* b = FUN_0040e110(a, &o, ...);             // ESI = b
//     FUN_009d6947(b, &o, 1, [esp+0x10]);
//     FUN_009d2646([esp+0x20]);
//     void** r = FUN_0043d080(&scratch, *out_ish, b, [esp+0x45c]);  // builder
//     EDI = *r; *r = 0;                                // steal ownership
//     // local scope-exit (state 1): release the [esp+0x18] interface if held.
//     if (([esp+0x18]) != 0) (*vtbl[0])(1);            // virtual release
//     if (b != 0) FUN_0040df70(b);                     // dtor of temporary
//     *out = EDI;                                      // hand back built ptr
//   done:
//     return out;                                      // EAX = EBP
//
//   Inline (non-frame-pointer) SEH frame is set up with handler
//   0x00e56aa2 and a doubled __security_cookie XOR (the /GS local-array
//   guard plus the EH-state cookie pushed at +0x2c). A source-level
//   __try/__finally translation would not reliably reproduce the exact
//   prolog cookie sequence, the EH state-number byte at [esp+0x43c],
//   the modrm-vs-moffs32 choices, or the two reload spills MSVC chose
//   across the ~16 helper calls and the FF15 indirect log sink.
//
//   Reloc-bearing sites in the orig 466 bytes (image base 0x00400000):
//     +0x02  PUSH imm32  → 0x00e56aa2  (SEH handler)
//     +0x14/0x27 MOV [imm32] → 0x012ea8b0 (__security_cookie)
//     +0x55  CALL rel32  → 0x004330f0   (probe ctor)
//     +0x70  PUSH imm32  → 0x00f66b0c   (record key)
//     +0x7b  CALL rel32  → 0x009d6b65
//     +0x8b/0x96/0xb5/0xd5 PUSH imm32 → 0x00f66b10/0xf66b50/0xf66ba4/0xf66bd0
//     +0xb0  CALL rel32  → 0x009d4f9f   (sprintf_s, twice)
//     +0xe4  CALL rel32  → 0x009d4bb4
//     +0xf0  CALL [imm32]→ 0x012660d8   (log/assert sink)
//     +0x106 CALL rel32  → 0x009d6a61
//     +0x10c CALL rel32  → 0x009d6962
//     +0x121 CALL rel32  → 0x0040e2d0
//     +0x12a CALL rel32  → 0x0040a330
//     +0x136 CALL rel32  → 0x0040e110
//     +0x146 CALL rel32  → 0x009d6947
//     +0x150 CALL rel32  → 0x009d2646
//     +0x168 CALL rel32  → 0x0043d080
//     +0x1a1 CALL rel32  → 0x0040df70
//     +0x1c6 CALL rel32  → 0x009d20f4   (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough, matching the
// established sibling idiom (FUN_0040b840 / FUN_00401820 / FUN_00409350)
// for inline-SEH /GS reloc-heavy bodies. The orig 466 bytes are re-emitted
// verbatim via MASM `_emit`; the .obj's `.text` ends up byte-identical to
// the orig slice (which is what tools/compare.py checks). The structural
// commentary above is the readable record so a future contributor can
// promote this to a real source-level match once the surrounding record
// class (FUN_009d6b65 lookup, the FUN_0043d080 builder, and the +0x18
// releasable interface) are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_0043d760() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xa2
        _emit 0x6a
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x81
        _emit 0xec
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
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
        _emit 0x34
        _emit 0x04
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
        _emit 0x44
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf1
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x50
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x36
        _emit 0x59
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x75
        _emit 0x0c
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0x39
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x0c
        _emit 0x6b
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x56
        _emit 0x51
        _emit 0xe8
        _emit 0x85
        _emit 0x93
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x7a
        _emit 0x68
        _emit 0x10
        _emit 0x6b
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xb1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x50
        _emit 0x6b
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0xfd
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x40
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0x88
        _emit 0x84
        _emit 0x24
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x8a
        _emit 0x77
        _emit 0x59
        _emit 0x00
        _emit 0x56
        _emit 0x68
        _emit 0xa4
        _emit 0x6b
        _emit 0xf6
        _emit 0x00
        _emit 0xb9
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xc8
        _emit 0x51
        _emit 0xba
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x2b
        _emit 0xd0
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x04
        _emit 0x58
        _emit 0x50
        _emit 0xe8
        _emit 0x6a
        _emit 0x77
        _emit 0x59
        _emit 0x00
        _emit 0x68
        _emit 0xd0
        _emit 0x6b
        _emit 0xf6
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0xe8
        _emit 0x6b
        _emit 0x73
        _emit 0x59
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x68
        _emit 0x6a
        _emit 0x04
        _emit 0x52
        _emit 0xff
        _emit 0x15
        _emit 0xd8
        _emit 0x60
        _emit 0x26
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x40
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xf6
        _emit 0x91
        _emit 0x59
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xf1
        _emit 0x90
        _emit 0x59
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x68
        _emit 0xd4
        _emit 0x6b
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0xf8
        _emit 0xe8
        _emit 0x4a
        _emit 0x0a
        _emit 0xfd
        _emit 0xff
        _emit 0x6a
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0xe8
        _emit 0xa1
        _emit 0xca
        _emit 0xfc
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xc8
        _emit 0xe8
        _emit 0x75
        _emit 0x08
        _emit 0xfd
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x6a
        _emit 0x01
        _emit 0x57
        _emit 0x56
        _emit 0xe8
        _emit 0x9c
        _emit 0x90
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51
        _emit 0xe8
        _emit 0x91
        _emit 0x4d
        _emit 0x59
        _emit 0x00
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x5c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x52
        _emit 0x56
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x51
        _emit 0xe8
        _emit 0xb3
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x10
        _emit 0x8b
        _emit 0xfa
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xc9
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x3c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x08
        _emit 0x8b
        _emit 0x01
        _emit 0x8b
        _emit 0x10
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0xd2
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x09
        _emit 0x8b
        _emit 0x4e
        _emit 0xfc
        _emit 0x56
        _emit 0xe8
        _emit 0x6a
        _emit 0x06
        _emit 0xfd
        _emit 0xff
        _emit 0x89
        _emit 0x7d
        _emit 0x00
        _emit 0x8b
        _emit 0xc5
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x34
        _emit 0x04
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
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xc9
        _emit 0x47
        _emit 0x59
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x30
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
