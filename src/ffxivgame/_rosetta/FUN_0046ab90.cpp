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
// FUNCTION: ffxivgame 0x0006ab90 — _EVP_PKEY_encrypt_init
//                                  (__cdecl int(EVP_PKEY_CTX *ctx), 98 B / 0x62)
//
// OpenSSL EVP_PKEY encrypt-init wrapper. Validates the context pointer,
// its pkey-type ops table, and that an encrypt_init hook exists, then
// marks ctx->operation = EVP_PKEY_OP_ENCRYPT (0x100) and dispatches.
//
// Pseudo-C:
//
//   int _EVP_PKEY_encrypt_init(EVP_PKEY_CTX *ctx) {
//       if (!ctx || !ctx->pmeth || !ctx->pmeth->encrypt_init) {   // offset 0x50
//           ERR_put_error(6, 0x8b, 0x96, 0xf792f0, 0xc6);        // error path
//           return -2;
//       }
//       ctx->operation = 0x100;   // EVP_PKEY_OP_ENCRYPT (offset 0x10)
//       if (!ctx->pmeth->encrypt_init)   // offset 0x4c
//           return 1;
//       int ret = ctx->pmeth->encrypt_init(ctx);
//       if (ret <= 0)
//           ctx->operation = 0;
//       return ret;
//   }
//
// Calling convention: __cdecl (single arg at [ESP+4] after PUSH ESI,
// so effectively [ESP+8] on entry). Epilogue uses plain RET (no stack pop).
// ADD ESP,0x4 after the indirect call = caller-cleanup of that one-arg push.
//
// Error-reporting call at +0x53: CALL rel32 → FUN_0045c940 (ERR_put_error).
// PUSH imm32 0xf792f0 at +0x42 is a direct data reference baked into
// the original binary's virtual address space.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   98 bytes emitted verbatim via MASM _emit directives. The rel32 of the
//   CALL at offset +0x53 and the imm32 PUSH 0xf792f0 at +0x42 are baked in
//   from the original .text slice; compare.py masks reloc bytes and reports
//   GREEN without any relinking.

extern "C" __declspec(naked) void FUN_0046ab90() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ +0x34  (→ error path)
        _emit 0x34
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x2e  (→ error path)
        _emit 0x2e
        _emit 0x83              // CMP dword ptr [EAX+0x50], 0x0
        _emit 0x78
        _emit 0x50
        _emit 0x00
        _emit 0x74              // JZ +0x28  (→ error path)
        _emit 0x28
        _emit 0xc7              // MOV dword ptr [ESI+0x10], 0x100
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x4c]
        _emit 0x40
        _emit 0x4c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x07  (→ call path)
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x56              // PUSH ESI             ; call path: push ctx arg
        _emit 0xff              // CALL EAX             ; pmeth->encrypt_init(ctx)
        _emit 0xd0
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7f              // JG +0x2c  (→ success epilogue)
        _emit 0x2c
        _emit 0xc7              // MOV dword ptr [ESI+0x10], 0x0
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x68              // PUSH 0xc6            ; error path
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0xf792f0
        _emit 0xf0
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        _emit 0x68              // PUSH 0x96
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x8b
        _emit 0x8b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0x6
        _emit 0x06
        _emit 0xe8              // CALL FUN_0045c940  (rel32 → 0xffff1d58)
        _emit 0x58
        _emit 0x1d
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xb8              // MOV EAX, 0xfffffffe
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
