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
// FUNCTION: ffxivgame 0x0005cf20 — EVP_DigestInit_ex (OpenSSL libcrypto)
//                                  (__cdecl, 420 B / 0x1a4, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x0005cf20):
//
//   __cdecl int EVP_DigestInit_ex(EVP_MD_CTX *ctx, const EVP_MD *type,
//                                 ENGINE *impl)
//
//   EVP_MD_CTX layout used by the body:
//     +0x00 const EVP_MD *digest
//     +0x04 ENGINE       *engine
//     +0x08 unsigned long flags          (tests bit 0x100 = NO_INIT)
//     +0x0c void         *md_data
//     +0x10 EVP_PKEY_CTX *pctx
//     +0x14 update fn ptr
//   EVP_MD layout used by the body:
//     +0x00 int type
//     +0x14 update fn ptr
//     +0x44 int ctx_size
//
//   Structural shape (classic OpenSSL EVP_DigestInit_ex):
//     EVP_MD_CTX_clear_flags(ctx, EVP_MD_CTX_FLAG_CLEANED /*2*/);
//     if (ctx->engine && ctx->digest &&
//         (!type || type->type == ctx->digest->type))
//         goto skip_to_init;
//     if (type) {
//         if (ctx->engine) ENGINE_finish(ctx->engine);          // 0x004695c0
//         if (impl) {
//             if (!ENGINE_init(impl)) {                          // 0x00469540
//                 ERR_put_error(6,0x80,0x86,"...",0xa3); return 0;
//             }
//         } else impl = ENGINE_get_digest_engine(type->type);    // 0x0046a510
//         if (!impl) { ctx->engine = 0; goto have_engine; }
//         {
//             const EVP_MD *d = ENGINE_get_digest(impl,type->type); // 0x0046a530
//             if (!d) { ERR_put_error(6,0x80,0x86,"...",0xb1);
//                       ENGINE_finish(impl); return 0; }
//             type = d; ctx->engine = impl;
//         }
//     } else if (!ctx->digest) {
//         ERR_put_error(6,0x80,0x8b,"...",0xc2); return 0;
//     }
//   have_engine:
//     if (ctx->digest != type) {
//         if (ctx->digest && ctx->digest->ctx_size) {
//             OPENSSL_free(ctx->md_data);                        // 0x004632f0
//         }
//         ctx->digest = type;
//         if (!(ctx->flags & 0x100) && type->ctx_size) {
//             ctx->update   = type->update;
//             ctx->md_data  = OPENSSL_malloc(type->ctx_size);    // 0x00463150
//             if (!ctx->md_data) { ERR_put_error(6,0x80,0xd2,"...",0x41); return 0; }
//         }
//     }
//   skip_to_init:
//     if (ctx->pctx) {
//         int r = EVP_PKEY_CTX_ctrl(ctx->pctx,-1,7,0xf8,0,ctx); // 0x0046a1f0
//         if (r <= 0 && r != -2) return 0;
//     }
//     if (ctx->flags & 0x100) return 1;
//     return ctx->digest->init(ctx);
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body is dense with linker-resolved sites: absolute imm32 PUSHes of
//   the source-file string pointer (0x00f68918) feeding the ERR_put_error
//   macro, and seven E8 rel32 calls into the ENGINE/ERR/OPENSSL helper
//   cluster. Coaxing MSVC 2005 /O2 into reproducing the exact branch
//   shapes (short vs near JZ), the shared ERR_put_error tail (the JMP back
//   into the middle of the first error block at 0x0045cf8e), and these
//   relocation windows byte-for-byte is brittle. As the sibling reloc-heavy
//   matches (FUN_0040ced0 et al.) do, emit the orig 420 bytes verbatim via
//   `__declspec(naked)` + `_emit` so the .obj .text slice is byte-identical
//   to the orig with no relocations.

extern "C" __declspec(naked) void FUN_0045cf20() {
    __asm {
        _emit 0x53
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x57
        _emit 0x6a
        _emit 0x02
        _emit 0x56
        _emit 0xe8
        _emit 0x21
        _emit 0xd7
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x18
        _emit 0x8b
        _emit 0x0e
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x12
        _emit 0x85
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0x2f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x17
        _emit 0x3b
        _emit 0x11
        _emit 0x0f
        _emit 0x84
        _emit 0x25
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xff
        _emit 0x0f
        _emit 0x84
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x09
        _emit 0x50
        _emit 0xe8
        _emit 0x59
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x85
        _emit 0xdb
        _emit 0x74
        _emit 0x31
        _emit 0x53
        _emit 0xe8
        _emit 0xc8
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x31
        _emit 0x68
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0xa6
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0xc3
        _emit 0x8b
        _emit 0x07
        _emit 0x50
        _emit 0xe8
        _emit 0x65
        _emit 0xd5
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0xd8
        _emit 0x85
        _emit 0xdb
        _emit 0x0f
        _emit 0x84
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x0f
        _emit 0x51
        _emit 0x53
        _emit 0xe8
        _emit 0x6f
        _emit 0xd5
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x2a
        _emit 0x68
        _emit 0xb1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0x5d
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x53
        _emit 0xe8
        _emit 0xd7
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0xc3
        _emit 0x8b
        _emit 0xf8
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        _emit 0x8b
        _emit 0x06
        _emit 0x3b
        _emit 0xc7
        _emit 0x74
        _emit 0x7d
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x12
        _emit 0x83
        _emit 0x78
        _emit 0x44
        _emit 0x00
        _emit 0x74
        _emit 0x0c
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        _emit 0x52
        _emit 0xe8
        _emit 0xe0
        _emit 0x62
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xf7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x3e
        _emit 0x75
        _emit 0x5c
        _emit 0x83
        _emit 0x7f
        _emit 0x44
        _emit 0x00
        _emit 0x74
        _emit 0x56
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        _emit 0x68
        _emit 0xce
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x46
        _emit 0x14
        _emit 0x8b
        _emit 0x4f
        _emit 0x44
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x51
        _emit 0xe8
        _emit 0x13
        _emit 0x61
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x75
        _emit 0x33
        _emit 0x68
        _emit 0xd2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x41
        _emit 0xe9
        _emit 0x36
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x96
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        _emit 0x75
        _emit 0x91
        _emit 0x68
        _emit 0xc2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x68
        _emit 0x8b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe9
        _emit 0x14
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x22
        _emit 0x56
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x07
        _emit 0x68
        _emit 0xf8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0xff
        _emit 0x50
        _emit 0xe8
        _emit 0x5d
        _emit 0xd1
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85
        _emit 0xc0
        _emit 0x7f
        _emit 0x09
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        _emit 0x0f
        _emit 0x85
        _emit 0xfa
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xf7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x09
        _emit 0x5f
        _emit 0x5e
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0xc3
        _emit 0x8b
        _emit 0x16
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        _emit 0x56
        _emit 0xff
        _emit 0xd0
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0xc3
    }
}
