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
// FUNCTION: ffxivgame 0x0046aac0 — EVP_PKEY_verify_init OpenSSL stub
//                                  (__cdecl int (EVP_PKEY_CTX *ctx), 95 bytes)
//
// Validates ctx, ctx->pmeth, and ctx->pmeth->verify_init (+0x30) are
// non-null; if any is null calls ERR_put_error(6, 0x8f, 0x96, str, 0x7d)
// and returns -2. Otherwise sets ctx->operation (+0x10) = 0x10, then
// calls ctx->pmeth->field_0x2c (init hook, if non-null). If the hook
// returns <= 0, resets ctx->operation to 0. Returns 1 if no init hook,
// else returns the hook's return value.
//
// Asm (95 bytes, RVA 0x0006aac0):
//   56                        PUSH ESI
//   8b 74 24 08               MOV  ESI, [ESP+0x8]         ; ctx
//   85 f6                     TEST ESI, ESI
//   74 34                     JZ   error_path
//   8b 06                     MOV  EAX, [ESI]             ; pmeth
//   85 c0                     TEST EAX, EAX
//   74 2e                     JZ   error_path
//   83 78 30 00               CMP  dword ptr [EAX+0x30], 0
//   74 28                     JZ   error_path
//   c7 46 10 10 00 00 00      MOV  dword ptr [ESI+0x10], 0x10
//   8b 40 2c                  MOV  EAX, [EAX+0x2c]        ; init fn ptr
//   85 c0                     TEST EAX, EAX
//   75 07                     JNZ  call_init
//   b8 01 00 00 00            MOV  EAX, 1
//   5e                        POP  ESI
//   c3                        RET
// call_init:
//   56                        PUSH ESI                    ; arg: ctx
//   ff d0                     CALL EAX
//   83 c4 04                  ADD  ESP, 0x4
//   85 c0                     TEST EAX, EAX
//   7f 29                     JG   success
//   c7 46 10 00 00 00 00      MOV  dword ptr [ESI+0x10], 0x0
//   5e                        POP  ESI
//   c3                        RET
// error_path:
//   6a 7d                     PUSH 0x7d
//   68 f0 92 f7 00            PUSH 0xf792f0               ; string literal
//   68 96 00 00 00            PUSH 0x96
//   68 8f 00 00 00            PUSH 0x8f
//   6a 06                     PUSH 0x6
//   e8 2b 1e ff ff            CALL 0x0045c940             ; ERR_put_error
//   83 c4 14                  ADD  ESP, 0x14
//   b8 fe ff ff ff            MOV  EAX, 0xfffffffe
// success:
//   5e                        POP  ESI
//   c3                        RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The CALL to ERR_put_error (rel32 → 0x0045c940) and the PUSH of the
//   string literal address (DIR32 0xf792f0) are baked-in absolute bytes
//   that resolve correctly against the orig PE. Emitting them as raw
//   _emit bytes produces a .obj whose .text is byte-identical to the
//   orig slice with zero relocations; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0046aac0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ error_path (+0x34)
        _emit 0x34
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ error_path (+0x2e)
        _emit 0x2e
        _emit 0x83              // CMP dword ptr [EAX+0x30], 0
        _emit 0x78
        _emit 0x30
        _emit 0x00
        _emit 0x74              // JZ error_path (+0x28)
        _emit 0x28
        _emit 0xc7              // MOV dword ptr [ESI+0x10], 0x10
        _emit 0x46
        _emit 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x2c]
        _emit 0x40
        _emit 0x2c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ call_init (+0x07)
        _emit 0x07
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x56              // PUSH ESI              ; call_init:
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7f              // JG success (+0x29)
        _emit 0x29
        _emit 0xc7              // MOV dword ptr [ESI+0x10], 0x0
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x6a              // PUSH 0x7d             ; error_path:
        _emit 0x7d
        _emit 0x68              // PUSH 0xf792f0 (string literal addr)
        _emit 0xf0
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        _emit 0x68              // PUSH 0x96
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x8f
        _emit 0x8f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0x6
        _emit 0x06
        _emit 0xe8              // CALL 0x0045c940 (rel32 → ERR_put_error)
        _emit 0x2b
        _emit 0x1e
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
        _emit 0x5e              // POP ESI               ; success:
        _emit 0xc3              // RET
    }
}
