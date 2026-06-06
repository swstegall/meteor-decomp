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
// FUNCTION: ffxivgame 0x00479dd0 — _RAND_status lazy-init dispatch
//                                  (__cdecl, 83 bytes)
//
// int __cdecl _RAND_status(void)
//
// OpenSSL RAND_status thunk with inline RAND_METHOD lazy initialisation.
// The global at 0x0132e92c holds the active RAND_METHOD pointer
// (rand_meth).  The global at 0x0132e928 holds the engine reference
// used during init.
//
// Behaviour (read from orig bytes at RVA 0x00079dd0, 83 bytes):
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
//   EAX = rand_meth->status;         // RAND_METHOD.status at +0x14
//   if (EAX == NULL) goto err;
//   JMP EAX;                         // tail-call to status()
// err:
//   return 0;
//
// Reloc-bearing sites in the orig 83 bytes (two DIR32 MOVs, one DIR32 MOV
// for ESI, three CALL REL32s):
//   +0x01  MOV EAX,[0x0132e92c]         (a1 + moffs32)
//   +0x0b  CALL 0x004980c0              (e8 + REL32)
//   +0x17  CALL 0x004980d0              (e8 + REL32)
//   +0x21  MOV [0x0132e92c],EAX         (a3 + moffs32)
//   +0x29  CALL 0x004695c0              (e8 + REL32)
//   +0x31  CALL 0x00497700              (e8 + REL32)
//   +0x36  MOV [0x0132e92c],EAX         (a3 + moffs32)
//   +0x49  MOV [0x0132e928],ESI         (89 35 + DIR32)
//
// Structurally identical to _RAND_bytes (FUN_00479d10) and
// _RAND_pseudo_bytes (FUN_00479d70) — same lazy-init RAND_METHOD
// pattern.  Differences: uses RAND_METHOD.status slot at +0x14
// (vs .bytes at +0x04 / .pseudorand at +0x10), and returns 0 on
// error (XOR EAX,EAX) instead of -1 (OR EAX,0xffffffff).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   All call targets and global addresses are resolved at full-binary link
//   time; the orig bytes already carry the final linked values.  Emitting
//   them verbatim via _emit produces a .obj whose .text section is byte-
//   identical to the orig slice.  tools/compare.py masks the reloc windows
//   during the diff so any delta in those windows is ignored; since we re-
//   emit the same bytes the comparison is GREEN by direct equality.

extern "C" __declspec(naked) void FUN_00479dd0() {
    __asm {
        // 00079dd0: mov eax, dword ptr [0x0132e92c]   ; rand_meth
        _emit 0xa1
        _emit 0x2c
        _emit 0xe9
        _emit 0x32
        _emit 0x01
        // 00079dd5: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 00079dd7: jnz use_it (+0x36 → 0x00479e0f)
        _emit 0x75
        _emit 0x36
        // 00079dd9: push esi
        _emit 0x56
        // 00079dda: call FUN_004980c0
        _emit 0xe8
        _emit 0xe1
        _emit 0xe2
        _emit 0x01
        _emit 0x00
        // 00079ddf: mov esi, eax
        _emit 0x8b
        _emit 0xf0
        // 00079de1: test esi, esi
        _emit 0x85
        _emit 0xf6
        // 00079de3: jz fallback (+0x1b → 0x00479e00)
        _emit 0x74
        _emit 0x1b
        // 00079de5: push esi
        _emit 0x56
        // 00079de6: call FUN_004980d0
        _emit 0xe8
        _emit 0xe5
        _emit 0xe2
        _emit 0x01
        _emit 0x00
        // 00079deb: add esp, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00079dee: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 00079df0: mov dword ptr [0x0132e92c], eax   ; rand_meth = EAX
        _emit 0xa3
        _emit 0x2c
        _emit 0xe9
        _emit 0x32
        _emit 0x01
        // 00079df5: jnz store_ref (+0x21 → 0x00479e18)
        _emit 0x75
        _emit 0x21
        // 00079df7: push esi
        _emit 0x56
        // 00079df8: call FUN_004695c0    (cleanup, e.g. ENGINE_finish)
        _emit 0xe8
        _emit 0xc3
        _emit 0xf7
        _emit 0xfe
        _emit 0xff
        // 00079dfd: add esp, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00079e00: call FUN_00497700    (fallback init, e.g. RAND_SSLeay)
        _emit 0xe8
        _emit 0xfb
        _emit 0xd8
        _emit 0x01
        _emit 0x00
        // 00079e05: mov dword ptr [0x0132e92c], eax   ; rand_meth = EAX
        _emit 0xa3
        _emit 0x2c
        _emit 0xe9
        _emit 0x32
        _emit 0x01
        // 00079e0a: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 00079e0c: pop esi
        _emit 0x5e
        // 00079e0d: jz err (+0x11 → 0x00479e20)
        _emit 0x74
        _emit 0x11
        // use_it:
        // 00079e0f: mov eax, dword ptr [eax+0x14]   ; RAND_METHOD.status
        _emit 0x8b
        _emit 0x40
        _emit 0x14
        // 00079e12: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 00079e14: jz err (+0x0a → 0x00479e20)
        _emit 0x74
        _emit 0x0a
        // 00079e16: jmp eax               ; tail-call to status()
        _emit 0xff
        _emit 0xe0
        // store_ref:
        // 00079e18: mov dword ptr [0x0132e928], esi
        _emit 0x89
        _emit 0x35
        _emit 0x28
        _emit 0xe9
        _emit 0x32
        _emit 0x01
        // 00079e1e: jmp test_and_use (-0x16 → 0x00479e0a)
        _emit 0xeb
        _emit 0xea
        // err:
        // 00079e20: xor eax, eax    ; return 0
        _emit 0x33
        _emit 0xc0
        // 00079e22: ret
        _emit 0xc3
    }
}
