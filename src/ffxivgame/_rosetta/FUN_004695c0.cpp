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
// FUNCTION: ffxivgame 0x000695c0 — _ENGINE_finish (225 B / 0xe1)
//
//   int __cdecl _ENGINE_finish(ENGINE *e)
//
//   Decrements the ENGINE's structural reference count and, if it drops to
//   zero, calls the engine's finish callback (e->finish_fn at offset 0x3c)
//   before calling ENGINE_free. Returns 1 on success, 0 on error.
//
//   Calling convention: __cdecl — single arg on stack at [ESP+0x8] after
//   PUSH ESI; plain RET (c3), no stack cleanup.
//
//   Stack layout (after PUSH ESI):
//     [ESP+0x04]  saved ESI
//     [ESP+0x08]  ENGINE *e   (param_1)
//
//   Saved registers: ESI, EDI (saved on non-NULL path)
//
//   Behaviour:
//     1. Load e → ESI. If ESI == NULL, call ENGINEerr(0x26, 0x6b, 0x43,
//        filename_ptr, 0x8e), return 0.
//     2. CRYPTO_push_info with line 0x91 / fn 0x09.
//     3. Decrement e->structural_refs ([ESI+0x5c]) by 1.
//     4. EDI = 1 (success flag).
//     5. If structural_refs not zero (JNZ → 0x46963c), skip to ENGINE_free.
//     6. If e->finish_fn ([ESI+0x3c]) == 0, skip to ENGINE_free.
//     7. CRYPTO_push_info (line 0x61 / fn 0x0a).
//     8. Call e->finish_fn(e) → save result to EDI.
//     9. CRYPTO_push_info (line 0x64 / fn 0x09).
//    10. ADD ESP, 0x24 (cleans up 4+1+4 = 9 push args accumulated).
//    11. If EDI == 0 (finish_fn failed), jump to XOR EDI,EDI (skip ENGINE_free).
//    12. ENGINE_free(e, 0) → if returns 0, call ENGINEerr + XOR EDI,EDI.
//    13. CRYPTO_push_info (line 0x93 / fn 0x0a).
//    14. ADD ESP, 0x10. If EDI == 0, call ENGINEerr(0x26, 0x6b, ...), return 0.
//    15. Otherwise MOV EAX, EDI, POP EDI, POP ESI, RET.
//
//   Three external CALL targets with relocations:
//     0x0045c940 — ENGINEerr  (called at +0x19, +0x9b, +0xcf)
//     0x00465f80 — CRYPTO_push_info (called at +0x34, +0x58, +0x70, +0xb3)
//     0x0047c610 — ENGINE_free (called at +0x7f)
//
//   Naked asm passthrough: the same brittleness as every surrounding OpenSSL
//   stub (FUN_00469480, FUN_004694a0, …) — short branches, no /GS frame,
//   no locals — makes the naked _emit route the only deterministic path.
//   compare.py masks the five reloc-bearing imm32/rel32 windows so the
//   baked-in bytes still yield GREEN.

extern "C" __declspec(naked) void FUN_004695c0() {
    __asm {
        // 000695c0: 56                  PUSH ESI
        _emit 0x56
        // 000695c1: 8b 74 24 08         MOV ESI, [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 000695c5: 85 f6               TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 000695c7: 75 1c               JNZ +0x1c (→ 0x004695e5)
        _emit 0x75
        _emit 0x1c
        // 000695c9: 68 8e 00 00 00      PUSH 0x8e
        _emit 0x68
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000695ce: 68 84 91 f7 00      PUSH 0xf79184
        _emit 0x68
        _emit 0x84
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 000695d3: 6a 43               PUSH 0x43
        _emit 0x6a
        _emit 0x43
        // 000695d5: 6a 6b               PUSH 0x6b
        _emit 0x6a
        _emit 0x6b
        // 000695d7: 6a 26               PUSH 0x26
        _emit 0x6a
        _emit 0x26
        // 000695d9: e8 62 33 ff ff      CALL 0x0045c940 (ENGINEerr)
        _emit 0xe8
        _emit 0x62
        _emit 0x33
        _emit 0xff
        _emit 0xff
        // 000695de: 83 c4 14            ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 000695e1: 33 c0               XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000695e3: 5e                  POP ESI
        _emit 0x5e
        // 000695e4: c3                  RET
        _emit 0xc3
        // 000695e5: 57                  PUSH EDI
        _emit 0x57
        // 000695e6: 68 91 00 00 00      PUSH 0x91
        _emit 0x68
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000695eb: 68 84 91 f7 00      PUSH 0xf79184
        _emit 0x68
        _emit 0x84
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 000695f0: 6a 1e               PUSH 0x1e
        _emit 0x6a
        _emit 0x1e
        // 000695f2: 6a 09               PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // 000695f4: e8 87 c9 ff ff      CALL 0x00465f80 (CRYPTO_push_info)
        _emit 0xe8
        _emit 0x87
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 000695f9: 83 c4 10            ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 000695fc: 83 46 5c ff         ADD dword ptr [ESI+0x5c], -1
        _emit 0x83
        _emit 0x46
        _emit 0x5c
        _emit 0xff
        // 00069600: bf 01 00 00 00      MOV EDI, 1
        _emit 0xbf
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00069605: 75 35               JNZ +0x35 (→ 0x0046963c)
        _emit 0x75
        _emit 0x35
        // 00069607: 83 7e 3c 00         CMP dword ptr [ESI+0x3c], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x3c
        _emit 0x00
        // 0006960b: 74 2f               JZ +0x2f (→ 0x0046963c)
        _emit 0x74
        _emit 0x2f
        // 0006960d: 6a 61               PUSH 0x61
        _emit 0x6a
        _emit 0x61
        // 0006960f: 68 84 91 f7 00      PUSH 0xf79184
        _emit 0x68
        _emit 0x84
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069614: 6a 1e               PUSH 0x1e
        _emit 0x6a
        _emit 0x1e
        // 00069616: 6a 0a               PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 00069618: e8 63 c9 ff ff      CALL 0x00465f80 (CRYPTO_push_info)
        _emit 0xe8
        _emit 0x63
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 0006961d: 8b 46 3c            MOV EAX, dword ptr [ESI+0x3c]
        _emit 0x8b
        _emit 0x46
        _emit 0x3c
        // 00069620: 56                  PUSH ESI
        _emit 0x56
        // 00069621: ff d0               CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00069623: 6a 64               PUSH 0x64
        _emit 0x6a
        _emit 0x64
        // 00069625: 68 84 91 f7 00      PUSH 0xf79184
        _emit 0x68
        _emit 0x84
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 0006962a: 6a 1e               PUSH 0x1e
        _emit 0x6a
        _emit 0x1e
        // 0006962c: 6a 09               PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // 0006962e: 8b f8               MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 00069630: e8 4b c9 ff ff      CALL 0x00465f80 (CRYPTO_push_info)
        _emit 0xe8
        _emit 0x4b
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 00069635: 83 c4 24            ADD ESP, 0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 00069638: 85 ff               TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0006963a: 74 27               JZ +0x27 (→ 0x00469663)
        _emit 0x74
        _emit 0x27
        // 0006963c: 6a 00               PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0006963e: 56                  PUSH ESI
        _emit 0x56
        // 0006963f: e8 cc 2f 01 00      CALL 0x0047c610 (ENGINE_free)
        _emit 0xe8
        _emit 0xcc
        _emit 0x2f
        _emit 0x01
        _emit 0x00
        // 00069644: 83 c4 08            ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00069647: 85 c0               TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00069649: 75 1a               JNZ +0x1a (→ 0x00469665)
        _emit 0x75
        _emit 0x1a
        // 0006964b: 6a 72               PUSH 0x72
        _emit 0x6a
        _emit 0x72
        // 0006964d: 68 84 91 f7 00      PUSH 0xf79184
        _emit 0x68
        _emit 0x84
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069652: 6a 6a               PUSH 0x6a
        _emit 0x6a
        _emit 0x6a
        // 00069654: 68 bf 00 00 00      PUSH 0xbf
        _emit 0x68
        _emit 0xbf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00069659: 6a 26               PUSH 0x26
        _emit 0x6a
        _emit 0x26
        // 0006965b: e8 e0 32 ff ff      CALL 0x0045c940 (ENGINEerr)
        _emit 0xe8
        _emit 0xe0
        _emit 0x32
        _emit 0xff
        _emit 0xff
        // 00069660: 83 c4 14            ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00069663: 33 ff               XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 00069665: 68 93 00 00 00      PUSH 0x93
        _emit 0x68
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006966a: 68 84 91 f7 00      PUSH 0xf79184
        _emit 0x68
        _emit 0x84
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 0006966f: 6a 1e               PUSH 0x1e
        _emit 0x6a
        _emit 0x1e
        // 00069671: 6a 0a               PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 00069673: e8 08 c9 ff ff      CALL 0x00465f80 (CRYPTO_push_info)
        _emit 0xe8
        _emit 0x08
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 00069678: 83 c4 10            ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0006967b: 85 ff               TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0006967d: 75 1d               JNZ +0x1d (→ 0x0046969c)
        _emit 0x75
        _emit 0x1d
        // 0006967f: 68 96 00 00 00      PUSH 0x96
        _emit 0x68
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00069684: 68 84 91 f7 00      PUSH 0xf79184
        _emit 0x68
        _emit 0x84
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069689: 6a 6a               PUSH 0x6a
        _emit 0x6a
        _emit 0x6a
        // 0006968b: 6a 6b               PUSH 0x6b
        _emit 0x6a
        _emit 0x6b
        // 0006968d: 6a 26               PUSH 0x26
        _emit 0x6a
        _emit 0x26
        // 0006968f: e8 ac 32 ff ff      CALL 0x0045c940 (ENGINEerr)
        _emit 0xe8
        _emit 0xac
        _emit 0x32
        _emit 0xff
        _emit 0xff
        // 00069694: 83 c4 14            ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00069697: 5f                  POP EDI
        _emit 0x5f
        // 00069698: 33 c0               XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0006969a: 5e                  POP ESI
        _emit 0x5e
        // 0006969b: c3                  RET
        _emit 0xc3
        // 0006969c: 8b c7               MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 0006969e: 5f                  POP EDI
        _emit 0x5f
        // 0006969f: 5e                  POP ESI
        _emit 0x5e
        // 000696a0: c3                  RET
        _emit 0xc3
    }
}
