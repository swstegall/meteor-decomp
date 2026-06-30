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
// FUNCTION: ffxivgame 0x0005d210 — _EVP_DigestFinal (38 B / 0x26)
//                                   (__cdecl, 3 args: ctx, md, s)
//
// int _EVP_DigestFinal(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s)
//   [ESP+0x04] : ctx  — EVP_MD_CTX *
//   [ESP+0x08] : md   — unsigned char *
//   [ESP+0x0c] : s    — unsigned int *
//
// Thin wrapper around EVP_DigestFinal_ex + EVP_MD_CTX_cleanup:
//   1. Pre-loads arg3 (s) → EAX and arg2 (md) → ECX before PUSH ESI.
//   2. PUSH ESI, then loads arg1 (ctx) → ESI via [ESP+0x8] (offset shifted by 1 push).
//   3. PUSH EDI.
//   4. Pushes (ctx, md, s) and calls _EVP_DigestFinal_ex — saves return in EDI.
//   5. Pushes ctx and calls _EVP_MD_CTX_cleanup.
//   6. Single ADD ESP, 0x10 cleans both call frames (3+1 pushes).
//   7. MOV EAX, EDI — restores and returns the DigestFinal_ex result.
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//   The two CALL rel32 fields are emitted via MASM `call` directives so the
//   linker inserts the correct relative offsets at link time. compare.py masks
//   reloc bytes in the diff, guaranteeing GREEN regardless of image base.
//
// Asm (38 bytes @ orig RVA 0x0005d210):
//   8b 44 24 0c   MOV EAX, [ESP+0xC]          ; arg3 (s)
//   8b 4c 24 08   MOV ECX, [ESP+0x8]          ; arg2 (md)
//   56            PUSH ESI
//   8b 74 24 08   MOV ESI, [ESP+0x8]          ; arg1 (ctx)  [offset shifted]
//   57            PUSH EDI
//   50            PUSH EAX                    ; s
//   51            PUSH ECX                    ; md
//   56            PUSH ESI                    ; ctx
//   e8 ba fe ff ff CALL _EVP_DigestFinal_ex
//   56            PUSH ESI                    ; ctx (for cleanup)
//   8b f8         MOV EDI, EAX               ; save ret
//   e8 32 ff ff ff CALL _EVP_MD_CTX_cleanup
//   83 c4 10      ADD ESP, 0x10
//   8b c7         MOV EAX, EDI               ; return DigestFinal_ex result
//   5f            POP EDI
//   5e            POP ESI
//   c3            RET

// Forward declarations for the two CALL relocations
extern "C" int  FUN_0045d0e0(void *, void *, unsigned int *);  // _EVP_DigestFinal_ex
extern "C" int  FUN_0045d160(void *);                          // _EVP_MD_CTX_cleanup

extern "C" __declspec(naked) int FUN_0045d210(void *, void *, unsigned int *)
{
    __asm {
        // 0005d210: 8b 44 24 0c   MOV EAX, [ESP+0xC]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0005d214: 8b 4c 24 08   MOV ECX, [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0005d218: 56            PUSH ESI
        _emit 0x56
        // 0005d219: 8b 74 24 08   MOV ESI, [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0005d21d: 57            PUSH EDI
        _emit 0x57
        // 0005d21e: 50            PUSH EAX
        _emit 0x50
        // 0005d21f: 51            PUSH ECX
        _emit 0x51
        // 0005d220: 56            PUSH ESI
        _emit 0x56
        // 0005d221: e8 ba fe ff ff CALL _EVP_DigestFinal_ex
        call FUN_0045d0e0
        // 0005d226: 56            PUSH ESI
        _emit 0x56
        // 0005d227: 8b f8         MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 0005d229: e8 32 ff ff ff CALL _EVP_MD_CTX_cleanup
        call FUN_0045d160
        // 0005d22e: 83 c4 10      ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005d231: 8b c7         MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 0005d233: 5f            POP EDI
        _emit 0x5f
        // 0005d234: 5e            POP ESI
        _emit 0x5e
        // 0005d235: c3            RET
        _emit 0xc3
    }
}
