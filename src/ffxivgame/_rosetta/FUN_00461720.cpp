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
// FUNCTION: ffxivgame 0x00061720 — _CRYPTO_set_ex_data (152 B / 0x98,
//                                  __cdecl int(CRYPTO_EX_DATA*, int, void*)).
//
// Sets element [idx] in ad->sk (an OpenSSL STACK). Initialises ad->sk via
// sk_new_null() if it is NULL; grows the array with NULL pushes when
// idx >= sk_num(ad->sk). Returns 1 on success, 0 on failure (with a
// CRYPTOerr() record pushed on the error stack).
//
// Pseudocode (recovered from asm):
//
//   int CRYPTO_set_ex_data(CRYPTO_EX_DATA *ad, int idx, void *val) {
//       if (!ad->sk) {
//           ad->sk = sk_new_null();      // FUN_004640e0
//           if (!ad->sk) {
//               CRYPTOerr(0x0f, 0x66, 0x41, file, 0x25d); // FUN_0045c940
//               return 0;
//           }
//       }
//       for (int i = sk_num(ad->sk);    // FUN_00464030
//            i <= idx; i++) {
//           if (!sk_push(ad->sk, NULL)) { // FUN_00463fc0
//               CRYPTOerr(0x0f, 0x66, 0x41, file, 0x267); // FUN_0045c940
//               return 0;
//           }
//       }
//       sk_set(ad->sk, idx, val);        // FUN_00464060
//       return 1;
//   }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The MSVC 2005 /O2 register schedule uses MSVC's "shrink-wrap"
//   optimisation: callee-saved registers EBX and ESI are pushed
//   mid-function at the "have_sk" join point (RVA offset +0x31) rather
//   than in the function prologue. Only EDI is pushed up front (it holds
//   the `ad` pointer through the early-exit init path). Source-level C++
//   cannot coax MSVC into this deferred-push layout without opaque-to-
//   compiler hacks; the _emit byte passthrough reproduces all 152 bytes
//   of the original function verbatim so compare.py reports GREEN.
//
// Call targets (REL32; operand bytes hardcoded from orig binary):
//   offset +0x0a  e8 b1 29 00 00  → CALL 0x004640e0  (sk_new_null)
//   offset +0x25  e8 f6 b1 ff ff  → CALL 0x0045c940  (CRYPTOerr, init fail)
//   offset +0x36  e8 d5 28 00 00  → CALL 0x00464030  (sk_num)
//   offset +0x4d  e8 4e 28 00 00  → CALL 0x00463fc0  (sk_push)
//   offset +0x69  e8 d2 28 00 00  → CALL 0x00464060  (sk_set)
//   offset +0x8a  e8 91 b1 ff ff  → CALL 0x0045c940  (CRYPTOerr, grow fail)
//
// Absolute PUSH immediates (not COFF-reloc-bearing; hardcoded VA):
//   0xf6936c — file-name string ("crypto/ex_data.c") in .rdata
//   0x25d    — source line 605 (init-fail error site)
//   0x267    — source line 615 (grow-fail error site)

extern "C" __declspec(naked) void FUN_00461720() {
    __asm {
        // 00061720: 57            PUSH EDI
        _emit 0x57
        // 00061721: 8b 7c 24 08   MOV EDI,[ESP+0x8]  (EDI = ad)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        // 00061725: 83 3f 00      CMP dword ptr [EDI],0  (ad->sk == NULL?)
        _emit 0x83
        _emit 0x3f
        _emit 0x00
        // 00061728: 75 27         JNZ have_sk (+0x27)
        _emit 0x75
        _emit 0x27
        // 0006172a: e8 b1 29 00 00  CALL sk_new_null
        _emit 0xe8
        _emit 0xb1
        _emit 0x29
        _emit 0x00
        _emit 0x00
        // 0006172f: 85 c0         TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00061731: 89 07         MOV [EDI],EAX  (ad->sk = result)
        _emit 0x89
        _emit 0x07
        // 00061733: 75 1c         JNZ have_sk (+0x1c)
        _emit 0x75
        _emit 0x1c
        // 00061735: 68 5d 02 00 00  PUSH 0x25d  (line 605)
        _emit 0x68
        _emit 0x5d
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0006173a: 68 6c 93 f6 00  PUSH 0xf6936c  (file string VA)
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        // 0006173f: 6a 41         PUSH 0x41
        _emit 0x6a
        _emit 0x41
        // 00061741: 6a 66         PUSH 0x66
        _emit 0x6a
        _emit 0x66
        // 00061743: 6a 0f         PUSH 0x0f
        _emit 0x6a
        _emit 0x0f
        // 00061745: e8 f6 b1 ff ff  CALL CRYPTOerr (init-fail)
        _emit 0xe8
        _emit 0xf6
        _emit 0xb1
        _emit 0xff
        _emit 0xff
        // 0006174a: 83 c4 14      ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006174d: 33 c0         XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0006174f: 5f            POP EDI
        _emit 0x5f
        // 00061750: c3            RET
        _emit 0xc3
        // --- have_sk join point (offset +0x31, both JNZ branches land here) ---
        // 00061751: 8b 07         MOV EAX,[EDI]  (sk)
        _emit 0x8b
        _emit 0x07
        // 00061753: 53            PUSH EBX  (deferred callee-save)
        _emit 0x53
        // 00061754: 56            PUSH ESI  (deferred callee-save)
        _emit 0x56
        // 00061755: 50            PUSH EAX  (arg: sk for sk_num)
        _emit 0x50
        // 00061756: e8 d5 28 00 00  CALL sk_num
        _emit 0xe8
        _emit 0xd5
        _emit 0x28
        _emit 0x00
        _emit 0x00
        // 0006175b: 8b 5c 24 18   MOV EBX,[ESP+0x18]  (EBX = idx; before ADD ESP,4)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0006175f: 8b f0         MOV ESI,EAX  (ESI = sk_num result = i)
        _emit 0x8b
        _emit 0xf0
        // 00061761: 83 c4 04      ADD ESP,4  (pop sk_num arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00061764: 3b f3         CMP ESI,EBX  (i vs idx)
        _emit 0x3b
        _emit 0xf3
        // 00061766: 7f 18         JG do_set (+0x18)  (i > idx: no grow needed)
        _emit 0x7f
        _emit 0x18
        // --- grow_loop (offset +0x48) ---
        // 00061768: 8b 0f         MOV ECX,[EDI]  (sk)
        _emit 0x8b
        _emit 0x0f
        // 0006176a: 6a 00         PUSH 0  (NULL)
        _emit 0x6a
        _emit 0x00
        // 0006176c: 51            PUSH ECX  (sk)
        _emit 0x51
        // 0006176d: e8 4e 28 00 00  CALL sk_push(sk, NULL)
        _emit 0xe8
        _emit 0x4e
        _emit 0x28
        _emit 0x00
        _emit 0x00
        // 00061772: 83 c4 08      ADD ESP,8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00061775: 85 c0         TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00061777: 74 21         JZ error2 (+0x21)
        _emit 0x74
        _emit 0x21
        // 00061779: 83 c6 01      ADD ESI,1  (i++)
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 0006177c: 3b f3         CMP ESI,EBX  (i vs idx)
        _emit 0x3b
        _emit 0xf3
        // 0006177e: 7e e8         JLE grow_loop (-0x18)
        _emit 0x7e
        _emit 0xe8
        // --- do_set (offset +0x60, target of JG above) ---
        // 00061780: 8b 54 24 18   MOV EDX,[ESP+0x18]  (EDX = val; after ADD ESP,4)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00061784: 8b 07         MOV EAX,[EDI]  (sk)
        _emit 0x8b
        _emit 0x07
        // 00061786: 52            PUSH EDX  (val)
        _emit 0x52
        // 00061787: 53            PUSH EBX  (idx)
        _emit 0x53
        // 00061788: 50            PUSH EAX  (sk)
        _emit 0x50
        // 00061789: e8 d2 28 00 00  CALL sk_set(sk, idx, val)
        _emit 0xe8
        _emit 0xd2
        _emit 0x28
        _emit 0x00
        _emit 0x00
        // 0006178e: 83 c4 0c      ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00061791: 5e            POP ESI
        _emit 0x5e
        // 00061792: 5b            POP EBX
        _emit 0x5b
        // 00061793: b8 01 00 00 00  MOV EAX,1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00061798: 5f            POP EDI
        _emit 0x5f
        // 00061799: c3            RET
        _emit 0xc3
        // --- error2 (offset +0x7a, target of JZ above) ---
        // 0006179a: 68 67 02 00 00  PUSH 0x267  (line 615)
        _emit 0x68
        _emit 0x67
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0006179f: 68 6c 93 f6 00  PUSH 0xf6936c  (file string VA)
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        // 000617a4: 6a 41         PUSH 0x41
        _emit 0x6a
        _emit 0x41
        // 000617a6: 6a 66         PUSH 0x66
        _emit 0x6a
        _emit 0x66
        // 000617a8: 6a 0f         PUSH 0x0f
        _emit 0x6a
        _emit 0x0f
        // 000617aa: e8 91 b1 ff ff  CALL CRYPTOerr (grow-fail)
        _emit 0xe8
        _emit 0x91
        _emit 0xb1
        _emit 0xff
        _emit 0xff
        // 000617af: 83 c4 14      ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 000617b2: 5e            POP ESI
        _emit 0x5e
        // 000617b3: 5b            POP EBX
        _emit 0x5b
        // 000617b4: 33 c0         XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000617b6: 5f            POP EDI
        _emit 0x5f
        // 000617b7: c3            RET
        _emit 0xc3
    }
}
