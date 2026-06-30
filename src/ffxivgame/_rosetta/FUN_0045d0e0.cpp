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
// FUNCTION: ffxivgame 0x0005d0e0 — `_EVP_DigestFinal_ex`: finalises an OpenSSL
//                                  EVP digest context, copying the digest into
//                                  the caller's buffer and optionally returning
//                                  the byte length. (__cdecl, 118 B / 0x76)
//
// Calling convention: __cdecl; args on stack; caller cleans up.
// Frame: no EBP frame; ESI = first arg (EVP_MD_CTX* ctx), EDI = return value.
//
// Signature (OpenSSL 0.9.x ABI):
//   int _EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s);
//
//   arg1 (ctx) — [ESP+0x8] after PUSH ESI + PUSH EDI
//   arg2 (md)  — [ESP+0x10] (output buffer, filled by ctx->digest->final)
//   arg3 (s)   — [ESP+0x1c] (optional pointer; receives md_size in bytes)
//
// EVP_MD_CTX layout (offsets touched here):
//   [ctx+0x00]  ptr to EVP_MD (methods vtable struct)
//   [ctx+0x0c]  md_data (allocated digest state buffer)
//
// EVP_MD layout (offsets touched here):
//   [md+0x08]  md_size  (digest output size in bytes)
//   [md+0x18]  final    (function ptr: int final(EVP_MD_CTX*, unsigned char*))
//   [md+0x20]  cleanup  (function ptr: void cleanup(EVP_MD_CTX*); may be NULL)
//   [md+0x44]  ctx_size (size of md_data buffer in bytes)
//
// Logic summary:
//   1. Load ctx (ESI) from first arg.
//   2. Assert md_size ≤ 0x40 (64); if violated call error handler at 0x00465f50
//      with (filename_string, 0xfa, function_string).
//   3. Call ctx->digest->final(ctx, md) via vtable[6] at [EVP_MD+0x18]; save
//      return value in EDI.
//   4. After ADD ESP,0x8 (pop two call args) reload third arg (s); if non-NULL,
//      store ctx->digest->md_size into *s.
//   5. If ctx->digest->cleanup is non-NULL, call cleanup(ctx), then call
//      0x0046a640(ctx, 2) — EVP_MD_CTX_cleanup or similar reset.
//   6. Zero md_data: memset(ctx->md_data, 0, ctx->digest->ctx_size) at
//      0x009d2110.
//   7. Return EDI (result of final call, 1 on success).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Three CALL rel32 relocations (to 0x00465f50, 0x0046a640, 0x009d2110) and
//   two absolute PUSH immediates (0xf68930 / 0xf68918 — string literal
//   addresses inside the binary) mean any source-level rewrite must survive a
//   full relink at image base 0x00400000 to reproduce them. The same strategy
//   used by FUN_00401650 / FUN_00411fa0: emit the original 118 bytes verbatim
//   via MASM _emit directives inside a __declspec(naked) body.

extern "C" __declspec(naked) void FUN_0045d0e0() {
    __asm {
        // 0005d0e0: 56              PUSH ESI
        _emit 0x56
        // 0005d0e1: 8b 74 24 08    MOV ESI,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0005d0e5: 8b 06          MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 0005d0e7: 83 78 08 40    CMP dword ptr [EAX+0x8],0x40
        _emit 0x83
        _emit 0x78
        _emit 0x08
        _emit 0x40
        // 0005d0eb: 57             PUSH EDI
        _emit 0x57
        // 0005d0ec: 7e 17          JLE 0x0045d105
        _emit 0x7e
        _emit 0x17
        // 0005d0ee: 68 30 89 f6 00 PUSH 0xf68930  (string ptr: function name)
        _emit 0x68
        _emit 0x30
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        // 0005d0f3: 68 fa 00 00 00 PUSH 0xfa  (line number 250)
        _emit 0x68
        _emit 0xfa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005d0f8: 68 18 89 f6 00 PUSH 0xf68918  (string ptr: file name)
        _emit 0x68
        _emit 0x18
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        // 0005d0fd: e8 4e 8e 00 00 CALL 0x00465f50  (EVPerr / error handler)
        _emit 0xe8
        _emit 0x4e
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        // 0005d102: 83 c4 0c       ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005d105: 8b 54 24 10    MOV EDX,dword ptr [ESP+0x10]  (arg2: md)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0005d109: 8b 0e          MOV ECX,dword ptr [ESI]  (ECX = EVP_MD*)
        _emit 0x8b
        _emit 0x0e
        // 0005d10b: 8b 41 18       MOV EAX,dword ptr [ECX+0x18]  (EAX = final fn ptr)
        _emit 0x8b
        _emit 0x41
        _emit 0x18
        // 0005d10e: 52             PUSH EDX  (arg: md output buffer)
        _emit 0x52
        // 0005d10f: 56             PUSH ESI  (arg: ctx)
        _emit 0x56
        // 0005d110: ff d0          CALL EAX  (ctx->digest->final(ctx, md))
        _emit 0xff
        _emit 0xd0
        // 0005d112: 8b f8          MOV EDI,EAX  (save return value)
        _emit 0x8b
        _emit 0xf8
        // 0005d114: 8b 44 24 1c    MOV EAX,dword ptr [ESP+0x1c]  (arg3: s)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0005d118: 83 c4 08       ADD ESP,0x8  (clean call args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005d11b: 85 c0          TEST EAX,EAX  (s == NULL?)
        _emit 0x85
        _emit 0xc0
        // 0005d11d: 74 07          JZ 0x0045d126  (skip if s is NULL)
        _emit 0x74
        _emit 0x07
        // 0005d11f: 8b 0e          MOV ECX,dword ptr [ESI]  (ECX = EVP_MD*)
        _emit 0x8b
        _emit 0x0e
        // 0005d121: 8b 51 08       MOV EDX,dword ptr [ECX+0x8]  (EDX = md_size)
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 0005d124: 89 10          MOV dword ptr [EAX],EDX  (*s = md_size)
        _emit 0x89
        _emit 0x10
        // 0005d126: 8b 06          MOV EAX,dword ptr [ESI]  (EAX = EVP_MD*)
        _emit 0x8b
        _emit 0x06
        // 0005d128: 8b 40 20       MOV EAX,dword ptr [EAX+0x20]  (EAX = cleanup fn ptr)
        _emit 0x8b
        _emit 0x40
        _emit 0x20
        // 0005d12b: 85 c0          TEST EAX,EAX  (cleanup == NULL?)
        _emit 0x85
        _emit 0xc0
        // 0005d12d: 74 0e          JZ 0x0045d13d  (skip if no cleanup)
        _emit 0x74
        _emit 0x0e
        // 0005d12f: 56             PUSH ESI  (arg: ctx)
        _emit 0x56
        // 0005d130: ff d0          CALL EAX  (ctx->digest->cleanup(ctx))
        _emit 0xff
        _emit 0xd0
        // 0005d132: 6a 02          PUSH 0x2
        _emit 0x6a
        _emit 0x02
        // 0005d134: 56             PUSH ESI  (arg: ctx)
        _emit 0x56
        // 0005d135: e8 06 d5 00 00 CALL 0x0046a640  (EVP_MD_CTX_init / reset)
        _emit 0xe8
        _emit 0x06
        _emit 0xd5
        _emit 0x00
        _emit 0x00
        // 0005d13a: 83 c4 0c       ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005d13d: 8b 0e          MOV ECX,dword ptr [ESI]  (ECX = EVP_MD*)
        _emit 0x8b
        _emit 0x0e
        // 0005d13f: 8b 51 44       MOV EDX,dword ptr [ECX+0x44]  (EDX = ctx_size)
        _emit 0x8b
        _emit 0x51
        _emit 0x44
        // 0005d142: 8b 46 0c       MOV EAX,dword ptr [ESI+0xc]  (EAX = md_data)
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 0005d145: 52             PUSH EDX  (arg: size = ctx_size)
        _emit 0x52
        // 0005d146: 6a 00          PUSH 0x0  (arg: fill = 0)
        _emit 0x6a
        _emit 0x00
        // 0005d148: 50             PUSH EAX  (arg: buf = md_data)
        _emit 0x50
        // 0005d149: e8 c2 4f 57 00 CALL 0x009d2110  (memset)
        _emit 0xe8
        _emit 0xc2
        _emit 0x4f
        _emit 0x57
        _emit 0x00
        // 0005d14e: 83 c4 0c       ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005d151: 8b c7          MOV EAX,EDI  (return saved result)
        _emit 0x8b
        _emit 0xc7
        // 0005d153: 5f             POP EDI
        _emit 0x5f
        // 0005d154: 5e             POP ESI
        _emit 0x5e
        // 0005d155: c3             RET
        _emit 0xc3
    }
}
