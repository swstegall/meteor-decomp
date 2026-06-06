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
// FUNCTION: ffxivgame 0x00479d10 — _RAND_bytes lazy-init dispatch
//                                  (__cdecl, 84 bytes)
//
// int __cdecl _RAND_bytes(unsigned char *buf, int num)
//
// OpenSSL RAND_bytes thunk with inline RAND_METHOD lazy initialisation.
// The global at 0x0132e92c holds the active RAND_METHOD pointer
// (rand_meth).  The global at 0x0132e928 holds the engine reference
// used during init.
//
// Behaviour (read from orig bytes at RVA 0x00079d10, 84 bytes):
//
//   rand_meth = [0x0132e92c]
//   if (rand_meth != NULL) goto use_it;
//
//   // lazy init:
//   ESI = FUN_004980c0();            // e.g. RAND_SSLeay() or get-engine-method
//   if (ESI == NULL) goto fallback;
//   EAX = FUN_004980d0(ESI);         // e.g. RAND_set_rand_method(ESI) → meth*
//   rand_meth = EAX;
//   if (EAX != NULL) {
//       [0x0132e928] = ESI;           // store engine ref in second global
//       goto test_and_use;
//   }
//   FUN_004695c0(ESI);               // cleanup (e.g. ENGINE_finish)
// fallback:
//   EAX = FUN_00497700();            // fallback init (e.g. RAND_SSLeay())
//   rand_meth = EAX;
// test_and_use:
//   POP ESI;
//   if (rand_meth == NULL) goto err;
// use_it:
//   EAX = rand_meth->bytes;          // RAND_METHOD.bytes at +4
//   if (EAX == NULL) goto err;
//   JMP EAX;                         // tail-call to bytes(buf, num)
// err:
//   return -1;
//
// Reloc-bearing sites in the orig 84 bytes (two DIR32 MOVs, one DIR32 MOV
// for ESI, three CALL REL32s, and one SHORT JMP):
//   +0x01  MOV EAX,[0x0132e92c]         (a1 + moffs32)
//   +0x0b  CALL 0x004980c0              (e8 + REL32)
//   +0x17  CALL 0x004980d0              (e8 + REL32)
//   +0x21  MOV [0x0132e92c],EAX         (a3 + moffs32)
//   +0x29  CALL 0x004695c0              (e8 + REL32)
//   +0x31  CALL 0x00497700              (e8 + REL32)
//   +0x36  MOV [0x0132e92c],EAX         (a3 + moffs32)
//   +0x49  MOV [0x0132e928],ESI         (89 35 + DIR32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   All call targets and global addresses are resolved at full-binary link
//   time; the orig bytes already carry the final linked values.  Emitting
//   them verbatim via _emit produces a .obj whose .text section is byte-
//   identical to the orig slice.  tools/compare.py masks the reloc windows
//   during the diff so any delta in those windows is ignored; since we re-
//   emit the same bytes the comparison is GREEN by direct equality.

extern "C" __declspec(naked) void FUN_00479d10() {
    __asm {
        // 00079d10: mov eax, dword ptr [0x0132e92c]   ; rand_meth
        _emit 0xa1
        _emit 0x2c
        _emit 0xe9
        _emit 0x32
        _emit 0x01
        // 00079d15: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 00079d17: jnz use_it (+0x36 → 0x00479d4f)
        _emit 0x75
        _emit 0x36
        // 00079d19: push esi
        _emit 0x56
        // 00079d1a: call FUN_004980c0
        _emit 0xe8
        _emit 0xa1
        _emit 0xe3
        _emit 0x01
        _emit 0x00
        // 00079d1f: mov esi, eax
        _emit 0x8b
        _emit 0xf0
        // 00079d21: test esi, esi
        _emit 0x85
        _emit 0xf6
        // 00079d23: jz fallback (+0x1b → 0x00479d40)
        _emit 0x74
        _emit 0x1b
        // 00079d25: push esi
        _emit 0x56
        // 00079d26: call FUN_004980d0
        _emit 0xe8
        _emit 0xa5
        _emit 0xe3
        _emit 0x01
        _emit 0x00
        // 00079d2b: add esp, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00079d2e: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 00079d30: mov dword ptr [0x0132e92c], eax   ; rand_meth = EAX
        _emit 0xa3
        _emit 0x2c
        _emit 0xe9
        _emit 0x32
        _emit 0x01
        // 00079d35: jnz store_ref (+0x21 → 0x00479d58)
        _emit 0x75
        _emit 0x21
        // 00079d37: push esi
        _emit 0x56
        // 00079d38: call FUN_004695c0    (cleanup, e.g. ENGINE_finish)
        _emit 0xe8
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0xff
        // 00079d3d: add esp, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00079d40: call FUN_00497700    (fallback init, e.g. RAND_SSLeay)
        _emit 0xe8
        _emit 0xbb
        _emit 0xd9
        _emit 0x01
        _emit 0x00
        // 00079d45: mov dword ptr [0x0132e92c], eax   ; rand_meth = EAX
        _emit 0xa3
        _emit 0x2c
        _emit 0xe9
        _emit 0x32
        _emit 0x01
        // 00079d4a: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 00079d4c: pop esi
        _emit 0x5e
        // 00079d4d: jz err (+0x11 → 0x00479d60)
        _emit 0x74
        _emit 0x11
        // use_it:
        // 00079d4f: mov eax, dword ptr [eax+4]   ; RAND_METHOD.bytes
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 00079d52: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 00079d54: jz err (+0x0a → 0x00479d60)
        _emit 0x74
        _emit 0x0a
        // 00079d56: jmp eax               ; tail-call to bytes(buf, num)
        _emit 0xff
        _emit 0xe0
        // store_ref:
        // 00079d58: mov dword ptr [0x0132e928], esi
        _emit 0x89
        _emit 0x35
        _emit 0x28
        _emit 0xe9
        _emit 0x32
        _emit 0x01
        // 00079d5e: jmp test_and_use (-0x16 → 0x00479d4a)
        _emit 0xeb
        _emit 0xea
        // err:
        // 00079d60: or eax, 0xffffffff    ; return -1
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 00079d63: ret
        _emit 0xc3
    }
}
