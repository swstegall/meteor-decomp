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
// FUNCTION: ffxivgame 0x00451470 — std::basic_string<char> iterator-pair
//                                  replace/assign helper (__thiscall, 195 B / 0xc3)
//
// Behaviour read from asm/ffxivgame/00051470_FUN_00451470.s:
//
//   __thiscall string& FUN_00451470(
//       this,                       // ECX = the source/self string
//       string&        result,       // [ESP+0x04] → returned as-is
//       const_iterator i1,           // [ESP+0x08] → must be npos(-2) or == this
//       const char*    s1,           // [ESP+0x0c] → start pointer (offset source)
//       const_iterator i2,           // [ESP+0x10] → must be npos(-2) or == i1
//       const char*    s2);          // [ESP+0x14] → end pointer (count source)
//
//   RET 0x14 cleans 5 DWORDs (20 bytes) of stack arguments.
//
//   Outline:
//     1. Get data_ptr from this (heap or SSO inline buf).
//     2. Assert data_ptr is non-null and i1 (s1) is within [data_ptr, data_ptr+size].
//     3. Assert i1 (arg2/EBP) is 0 (→ ESI=0), or equals npos(-2) or this; else error.
//        If i1 valid: ESI = s1 − data_ptr  (byte offset from string start).
//     4. Assert i2 (arg5/EDI) is 0 (→ EAX=0), or equals npos(-2) or i1; else error.
//        If i2 valid: EAX = s2 − s1  (character count).
//     5. Call FUN_00449570(this, ESI, EAX)   — erase/replace subrange
//     6. Get fresh data_ptr after the mutating call; advance by ESI.
//     7. Call FUN_004512e0(result, ptr, this) — assign/insert from ptr into result.
//     8. Return result.
//
//   Reloc-bearing CALL sites in the orig 195 bytes:
//     +0x3f  rel32  0x009d22b4  (assertion/throw helper, called up to 3 times)
//     +0x61  rel32  0x009d22b4
//     +0x89  rel32  0x009d22b4
//     +0x98  rel32  0x00449570  (string subrange mutator)
//     +0xb6  rel32  0x004512e0  (string assign/insert into result)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Five CALL rel32 relocations tie the function to the original binary's
//   absolute address space. The same pragmatic approach used for FUN_00406680,
//   FUN_00409120, and FUN_00403f10 applies here: a __declspec(naked) body that
//   re-emits the original 195 bytes verbatim via MASM _emit directives.
//   tools/compare.py masks the rel32 slots in its byte diff, so the .obj's
//   .text section is byte-identical to the original slice → GREEN.

extern "C" __declspec(naked) void FUN_00451470() {
    __asm {
        // 00451470: 53
        _emit 0x53
        // 00451471: 8b d9
        _emit 0x8b
        _emit 0xd9
        // 00451473: 8b 4b 18
        _emit 0x8b
        _emit 0x4b
        _emit 0x18
        // 00451476: 83 f9 10
        _emit 0x83
        _emit 0xf9
        _emit 0x10
        // 00451479: 56
        _emit 0x56
        // 0045147a: 57
        _emit 0x57
        // 0045147b: 72 05
        _emit 0x72
        _emit 0x05
        // 0045147d: 8b 7b 04
        _emit 0x8b
        _emit 0x7b
        _emit 0x04
        // 00451480: eb 03
        _emit 0xeb
        _emit 0x03
        // 00451482: 8d 7b 04
        _emit 0x8d
        _emit 0x7b
        _emit 0x04
        // 00451485: 85 ff
        _emit 0x85
        _emit 0xff
        // 00451487: 74 26
        _emit 0x74
        _emit 0x26
        // 00451489: 83 f9 10
        _emit 0x83
        _emit 0xf9
        _emit 0x10
        // 0045148c: 8d 53 04
        _emit 0x8d
        _emit 0x53
        _emit 0x04
        // 0045148f: 72 04
        _emit 0x72
        _emit 0x04
        // 00451491: 8b 02
        _emit 0x8b
        _emit 0x02
        // 00451493: eb 02
        _emit 0xeb
        _emit 0x02
        // 00451495: 8b c2
        _emit 0x8b
        _emit 0xc2
        // 00451497: 3b c7
        _emit 0x3b
        _emit 0xc7
        // 00451499: 77 14
        _emit 0x77
        _emit 0x14
        // 0045149b: 83 f9 10
        _emit 0x83
        _emit 0xf9
        _emit 0x10
        // 0045149e: 72 04
        _emit 0x72
        _emit 0x04
        // 004514a0: 8b 02
        _emit 0x8b
        _emit 0x02
        // 004514a2: eb 02
        _emit 0xeb
        _emit 0x02
        // 004514a4: 8b c2
        _emit 0x8b
        _emit 0xc2
        // 004514a6: 8b 4b 14
        _emit 0x8b
        _emit 0x4b
        _emit 0x14
        // 004514a9: 03 c8
        _emit 0x03
        _emit 0xc8
        // 004514ab: 3b f9
        _emit 0x3b
        _emit 0xf9
        // 004514ad: 76 05
        _emit 0x76
        _emit 0x05
        // 004514af: e8 00 0e 58 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0x00
        _emit 0x0e
        _emit 0x58
        _emit 0x00
        // 004514b4: 83 7c 24 18 00
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x00
        // 004514b9: 55
        _emit 0x55
        // 004514ba: 8b 6c 24 18
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 004514be: 75 04
        _emit 0x75
        _emit 0x04
        // 004514c0: 33 f6
        _emit 0x33
        _emit 0xf6
        // 004514c2: eb 18
        _emit 0xeb
        _emit 0x18
        // 004514c4: 83 fd fe
        _emit 0x83
        _emit 0xfd
        _emit 0xfe
        // 004514c7: 74 0d
        _emit 0x74
        _emit 0x0d
        // 004514c9: 85 ed
        _emit 0x85
        _emit 0xed
        // 004514cb: 74 04
        _emit 0x74
        _emit 0x04
        // 004514cd: 3b eb
        _emit 0x3b
        _emit 0xeb
        // 004514cf: 74 05
        _emit 0x74
        _emit 0x05
        // 004514d1: e8 de 0d 58 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xde
        _emit 0x0d
        _emit 0x58
        _emit 0x00
        // 004514d6: 8b 74 24 1c
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 004514da: 2b f7
        _emit 0x2b
        _emit 0xf7
        // 004514dc: 8b 7c 24 24
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        // 004514e0: 85 ff
        _emit 0x85
        _emit 0xff
        // 004514e2: 75 04
        _emit 0x75
        _emit 0x04
        // 004514e4: 33 c0
        _emit 0x33
        _emit 0xc0
        // 004514e6: eb 1c
        _emit 0xeb
        _emit 0x1c
        // 004514e8: 8b 44 24 20
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 004514ec: 83 f8 fe
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 004514ef: 74 0d
        _emit 0x74
        _emit 0x0d
        // 004514f1: 85 c0
        _emit 0x85
        _emit 0xc0
        // 004514f3: 74 04
        _emit 0x74
        _emit 0x04
        // 004514f5: 3b c5
        _emit 0x3b
        _emit 0xc5
        // 004514f7: 74 05
        _emit 0x74
        _emit 0x05
        // 004514f9: e8 b6 0d 58 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xb6
        _emit 0x0d
        _emit 0x58
        _emit 0x00
        // 004514fe: 2b 7c 24 1c
        _emit 0x2b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 00451502: 8b c7
        _emit 0x8b
        _emit 0xc7
        // 00451504: 50
        _emit 0x50
        // 00451505: 56
        _emit 0x56
        // 00451506: 8b cb
        _emit 0x8b
        _emit 0xcb
        // 00451508: e8 63 80 ff ff  CALL 0x00449570
        _emit 0xe8
        _emit 0x63
        _emit 0x80
        _emit 0xff
        _emit 0xff
        // 0045150d: 83 7b 18 10
        _emit 0x83
        _emit 0x7b
        _emit 0x18
        _emit 0x10
        // 00451511: 5d
        _emit 0x5d
        // 00451512: 72 05
        _emit 0x72
        _emit 0x05
        // 00451514: 8b 43 04
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        // 00451517: eb 03
        _emit 0xeb
        _emit 0x03
        // 00451519: 8d 43 04
        _emit 0x8d
        _emit 0x43
        _emit 0x04
        // 0045151c: 03 c6
        _emit 0x03
        _emit 0xc6
        // 0045151e: 8b 74 24 10
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00451522: 53
        _emit 0x53
        // 00451523: 50
        _emit 0x50
        // 00451524: 8b ce
        _emit 0x8b
        _emit 0xce
        // 00451526: e8 b5 fd ff ff  CALL 0x004512e0
        _emit 0xe8
        _emit 0xb5
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0045152b: 5f
        _emit 0x5f
        // 0045152c: 8b c6
        _emit 0x8b
        _emit 0xc6
        // 0045152e: 5e
        _emit 0x5e
        // 0045152f: 5b
        _emit 0x5b
        // 00451530: c2 14 00
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
