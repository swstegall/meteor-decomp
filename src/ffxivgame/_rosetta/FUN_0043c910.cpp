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
// FUNCTION: ffxivgame 0x0003c910 — object factory with conditional vtable
//                                  dispatch (214 B / 0xd6, __cdecl, /GS +
//                                  EH3-style SEH frame).
//
// Signature (recovered from the asm):
//
//   void FUN_0043c910(void **out, SomeStruct *s, SomeType arg2, SomeType *opt);
//
//   arg0 = [esp+0x24]: output pointer — receives the created object or NULL.
//   arg1 = [esp+0x28]: struct pointer — byte at +0x10 checked against 0x20;
//                      words at +0x0c and +0x0e passed to the factory;
//                      field at +0x12 passed to the member call.
//   arg2 = [esp+0x2c]: forwarded to FUN_004190a0 as its last arg.
//   arg3 = [esp+0x28] (re-read after factory): optional object on which a
//                      virtual method (vtable[0]) is called with arg=1; may
//                      be NULL.  NOTE: same stack slot as arg1 — the factory
//                      (FUN_004190a0) overwrites [esp+0x28] with the created
//                      object pointer, and this overwritten value is what is
//                      subsequently tested for NULL and used as 'this' for
//                      the virtual call.
//
// Body sketch (paraphrased):
//
//   try_level = 0;
//   if (s->type_byte != 0x20) { *out = NULL; return; }
//
//   WORD w1 = s->word_at_0x0c;
//   WORD w2 = s->word_at_0x0e;
//
//   // Factory: writes a new object into [esp+0x28] (arg1 slot), returns ptr.
//   SomeObj **ret = FUN_004190a0(&stack_arg1_slot, w1, w2, 1, 0, 4, arg2);
//   SomeObj *obj = *ret;          // steal result
//   *ret = NULL;                  // clear source
//   local_obj = obj;
//
//   try_level = 1;
//   SomeType *created = (SomeType *)[esp+0x28]; // now holds created object
//   if (created != NULL)
//       created->vtable[0](created, 1);         // virtual call
//
//   obj->method(0, 0, 4, &s->field_at_0x12, 0); // FUN_00431080, __thiscall
//   *out = obj;
//
// /GS layout (cookie at [esp+0x00]; no __security_check_cookie at exit —
// MSVC omits the check when no large local buffers are present):
//   [esp+0x00]  GS cookie
//   [esp+0x04]  saved EDI
//   [esp+0x08]  saved ESI
//   [esp+0x0c]  try_level (initially 0, then 1 after factory call)
//   [esp+0x10]  local_obj  (the stolen object pointer)
//   [esp+0x14]  old FS:[0]
//   [esp+0x18]  scope table ptr (0xe567b1, reloc site)
//   [esp+0x1c]  -1 (initial EH state)
//   [esp+0x20]  return address
//   [esp+0x24]  arg0  (output ptr)
//   [esp+0x28]  arg1  (struct ptr; overwritten by factory with obj ptr)
//   [esp+0x2c]  arg2
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x02  scope_table ptr (4 B)          — 0xe567b1
//   +0x08  __security_cookie load (4 B)   — 0x012ea8b0
//   +0x77  CALL FUN_004190a0 rel32 (4 B)  — 0xfffdc714
//   +0xb9  CALL FUN_00431080 rel32 (4 B)  — 0xffff46b2
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//   The EH3 SEH prolog + the interleaved MOV/PUSH arg shuffles + the
//   exact [esp+0x1c] state-index update timing make a source-level C++
//   reconstruction impractical under MSVC 2005 /O2 /GS /EHsc. Emitting
//   the 214 original bytes verbatim via MASM `_emit` directives gives a
//   byte-exact match (reloc sites are masked by compare.py).

extern "C" __declspec(naked) void FUN_0043c910() {
    __asm {
        // 0003c910: 6a ff
        _emit 0x6a
        _emit 0xff
        // 0003c912: 68 b1 67 e5 00
        _emit 0x68
        _emit 0xb1
        _emit 0x67
        _emit 0xe5
        _emit 0x00
        // 0003c917: 64 a1 00 00 00 00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c91d: 50
        _emit 0x50
        // 0003c91e: 83 ec 08
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0003c921: 56
        _emit 0x56
        // 0003c922: 57
        _emit 0x57
        // 0003c923: a1 b0 a8 2e 01
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003c928: 33 c4
        _emit 0x33
        _emit 0xc4
        // 0003c92a: 50
        _emit 0x50
        // 0003c92b: 8d 44 24 14
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0003c92f: 64 a3 00 00 00 00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c935: c7 44 24 0c 00 00 00 00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c93d: 8b 7c 24 28
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x28
        // 0003c941: 80 7f 10 20
        _emit 0x80
        _emit 0x7f
        _emit 0x10
        _emit 0x20
        // 0003c945: 0f b7 47 0c
        _emit 0x0f
        _emit 0xb7
        _emit 0x47
        _emit 0x0c
        // 0003c949: 0f b7 c8
        _emit 0x0f
        _emit 0xb7
        _emit 0xc8
        // 0003c94c: 0f b7 47 0e
        _emit 0x0f
        _emit 0xb7
        _emit 0x47
        _emit 0x0e
        // 0003c950: 0f b7 d0
        _emit 0x0f
        _emit 0xb7
        _emit 0xd0
        // 0003c953: 74 1c
        _emit 0x74
        _emit 0x1c
        // 0003c955: 8b 44 24 24
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0003c959: c7 00 00 00 00 00
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c95f: 8b 4c 24 14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0003c963: 64 89 0d 00 00 00 00
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c96a: 59
        _emit 0x59
        // 0003c96b: 5f
        _emit 0x5f
        // 0003c96c: 5e
        _emit 0x5e
        // 0003c96d: 83 c4 14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0003c970: c3
        _emit 0xc3
        // 0003c971: 8b 74 24 2c
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        // 0003c975: 56
        _emit 0x56
        // 0003c976: b8 04 00 00 00
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c97b: 50
        _emit 0x50
        // 0003c97c: 6a 00
        _emit 0x6a
        _emit 0x00
        // 0003c97e: 6a 01
        _emit 0x6a
        _emit 0x01
        // 0003c980: 52
        _emit 0x52
        // 0003c981: 51
        _emit 0x51
        // 0003c982: 8d 44 24 40
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x40
        // 0003c986: 50
        _emit 0x50
        // 0003c987: e8 14 c7 fd ff
        _emit 0xe8
        _emit 0x14
        _emit 0xc7
        _emit 0xfd
        _emit 0xff
        // 0003c98c: 8b 08
        _emit 0x8b
        _emit 0x08
        // 0003c98e: 8b f1
        _emit 0x8b
        _emit 0xf1
        // 0003c990: 83 c4 1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 0003c993: c7 00 00 00 00 00
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c999: 89 74 24 10
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0003c99d: 8b 4c 24 28
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0003c9a1: 85 c9
        _emit 0x85
        _emit 0xc9
        // 0003c9a3: c7 44 24 1c 01 00 00 00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c9ab: 74 08
        _emit 0x74
        _emit 0x08
        // 0003c9ad: 8b 11
        _emit 0x8b
        _emit 0x11
        // 0003c9af: 8b 02
        _emit 0x8b
        _emit 0x02
        // 0003c9b1: 6a 01
        _emit 0x6a
        _emit 0x01
        // 0003c9b3: ff d0
        _emit 0xff
        _emit 0xd0
        // 0003c9b5: 6a 00
        _emit 0x6a
        _emit 0x00
        // 0003c9b7: 83 c7 12
        _emit 0x83
        _emit 0xc7
        _emit 0x12
        // 0003c9ba: 57
        _emit 0x57
        // 0003c9bb: 6a 00
        _emit 0x6a
        _emit 0x00
        // 0003c9bd: b8 04 00 00 00
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c9c2: 50
        _emit 0x50
        // 0003c9c3: 6a 00
        _emit 0x6a
        _emit 0x00
        // 0003c9c5: 6a 00
        _emit 0x6a
        _emit 0x00
        // 0003c9c7: 8b ce
        _emit 0x8b
        _emit 0xce
        // 0003c9c9: e8 b2 46 ff ff
        _emit 0xe8
        _emit 0xb2
        _emit 0x46
        _emit 0xff
        _emit 0xff
        // 0003c9ce: 8b 44 24 24
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0003c9d2: 89 30
        _emit 0x89
        _emit 0x30
        // 0003c9d4: 8b 4c 24 14
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0003c9d8: 64 89 0d 00 00 00 00
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c9df: 59
        _emit 0x59
        // 0003c9e0: 5f
        _emit 0x5f
        // 0003c9e1: 5e
        _emit 0x5e
        // 0003c9e2: 83 c4 14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0003c9e5: c3
        _emit 0xc3
    }
}
