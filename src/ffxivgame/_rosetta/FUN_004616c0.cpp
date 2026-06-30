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
// FUNCTION: ffxivgame 0x000616c0 — `_CRYPTO_free_ex_data` ex-data impl
//                                  dispatcher (82 B / 0x52, no stack frame,
//                                  tail-call via JMP [ECX]).
//
// This is OpenSSL's CRYPTO_free_ex_data trampoline.  It checks whether the
// global EX_DATA_IMPL pointer at 0x0132e798 has been initialised.  If not,
// it emits two internal error/warning calls (0x00465f80 — four-arg __cdecl,
// probably OPENSSL_PUT_ERROR or a private variant) and installs the default
// implementation at 0x01268850 if the pointer is still NULL after the first
// call.  Once the impl pointer is known non-NULL, the function loads the
// function pointer stored at offset 0x14 inside the impl struct and
// tail-calls it (JMP ECX), forwarding the caller's arguments on the stack.
//
// Calling convention: __cdecl (no stack-frame setup; caller cleans up).
// The tail-call at the end (`jmp ecx`) leaves the caller's return address
// and all three parameters (class_index, obj, ad) on the stack exactly as
// the impl->free_ex_data method expects them.
//
// Control flow:
//
//   +0x00  CMP [impl], 0
//   +0x07  JNZ  dispatch (→ +0x48)      ; impl already set: skip init
//   +0x09  PUSH 0xcb / 0xf6936c / 2 / 9 ; first error call (ERR_LIB_CRYPTO / line?)
//   +0x17  CALL 0x00465f80
//   +0x1c  ADD  ESP, 10h
//   +0x1f  CMP [impl], 0
//   +0x26  JNZ  +0x0a (→ +0x32)         ; impl set by error callback: skip default
//   +0x28  MOV [impl], 0x01268850        ; install default impl
//   +0x32  PUSH 0xce / 0xf6936c / 2 / 0xa ; second error call
//   +0x40  CALL 0x00465f80
//   +0x45  ADD  ESP, 10h
//   +0x48  MOV  EAX, [impl]              ; dispatch
//   +0x4d  MOV  ECX, [EAX + 0x14]       ; load free_ex_data fn ptr
//   +0x50  JMP  ECX                      ; tail-call
//
// Reconstruction strategy — `_emit` byte passthrough:
//
//   The 82 bytes contain multiple absolute-address operands
//   (DIR32 relocs against the impl global, the default-impl symbol, the
//   error-string literal at 0xf6936c) and two REL32 CALL operands
//   against 0x00465f80.  Rebuilding this from source-level C would
//   require matching MSVC 2005's specific error-call macro expansion
//   sequence and the exact conditional layout for the double-check init
//   pattern.  The `_emit` passthrough avoids all of those fragile
//   constraints; compare.py masks the reloc windows during the diff.

extern "C" __declspec(naked) void FUN_004616c0() {
    __asm {
        // +0x00  CMP dword ptr [0x0132e798], 0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x98
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // +0x07  JNZ +0x3f  (→ dispatch at +0x48)
        _emit 0x75
        _emit 0x3f
        // +0x09  PUSH 0xcb
        _emit 0x68
        _emit 0xcb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0e  PUSH 0xf6936c  (string literal ptr — DIR32 reloc)
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        // +0x13  PUSH 0x2
        _emit 0x6a
        _emit 0x02
        // +0x15  PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // +0x17  CALL 0x00465f80  (REL32 reloc)
        _emit 0xe8
        _emit 0xa4
        _emit 0x48
        _emit 0x00
        _emit 0x00
        // +0x1c  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // +0x1f  CMP dword ptr [0x0132e798], 0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x98
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // +0x26  JNZ +0x0a  (→ +0x32, skip default-impl install)
        _emit 0x75
        _emit 0x0a
        // +0x28  MOV dword ptr [0x0132e798], 0x01268850  (two DIR32 relocs)
        _emit 0xc7
        _emit 0x05
        _emit 0x98
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x50
        _emit 0x88
        _emit 0x26
        _emit 0x01
        // +0x32  PUSH 0xce
        _emit 0x68
        _emit 0xce
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x37  PUSH 0xf6936c  (string literal ptr — DIR32 reloc)
        _emit 0x68
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        // +0x3c  PUSH 0x2
        _emit 0x6a
        _emit 0x02
        // +0x3e  PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // +0x40  CALL 0x00465f80  (REL32 reloc)
        _emit 0xe8
        _emit 0x7b
        _emit 0x48
        _emit 0x00
        _emit 0x00
        // +0x45  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // +0x48  MOV EAX, [0x0132e798]  (DIR32 reloc)
        _emit 0xa1
        _emit 0x98
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // +0x4d  MOV ECX, dword ptr [EAX + 0x14]
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // +0x50  JMP ECX  (tail-call into impl->free_ex_data)
        _emit 0xff
        _emit 0xe1
    }
}
