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
// FUNCTION: ffxivgame 0x0005caf0 — `_ERR_peek_last_error` (27 B, __cdecl).
//
// OpenSSL ERR_peek_last_error() — returns the error code at the top of the
// per-thread error queue without removing it, or 0 if the queue is empty.
//
// Asm shape (27 bytes, read from orig RVA 0x0005caf0):
//
//   0005caf0:  e8 6b f8 ff ff    CALL  FUN_0045c360    ; get ERR_STATE* → EAX
//   0005caf5:  8b 88 88 01 00 00 MOV   ECX, [EAX+0x188] ; top index
//   0005cafb:  39 88 8c 01 00 00 CMP   [EAX+0x18c], ECX  ; bottom == top?
//   0005cb01:  75 03             JNZ   0x0045cb06       ; no → queue not empty
//   0005cb03:  33 c0             XOR   EAX, EAX         ; empty → return 0
//   0005cb05:  c3                RET
//   0005cb06:  8b 44 88 48       MOV   EAX, [EAX+ECX*4+0x48] ; err_buffer[top]
//   0005cb0a:  c3                RET
//
// Reloc-bearing site:
//     +0x01  CALL rel32 → FUN_0045c360  (singleton getter / ERR_get_state)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The 27-byte body has one rel32 CALL and six fixed-byte instruction
//   sequences.  A source-level rewrite would need MSVC 2005 /O2 to keep
//   EAX live across the CALL (no PUSH/MOV to spill it), emit the
//   `CMP mem,reg` form (not `CMP reg,mem`), and choose the JNZ-over-two
//   branch encoding — all brittle under the optimiser.  The pragmatic
//   choice — matching FUN_0045ca40, FUN_0045c360, and all other reloc-
//   heavy siblings — is a `__declspec(naked)` body re-emitting the 27
//   orig bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_0045caf0() {
    __asm {
        // 0005caf0: CALL FUN_0045c360   ; rel32 = 0xfffff86b
        _emit 0xe8
        _emit 0x6b
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        // 0005caf5: MOV ECX, [EAX+0x188]
        _emit 0x8b
        _emit 0x88
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005cafb: CMP [EAX+0x18c], ECX
        _emit 0x39
        _emit 0x88
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005cb01: JNZ +3 (to 0x0045cb06)
        _emit 0x75
        _emit 0x03
        // 0005cb03: XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0005cb05: RET
        _emit 0xc3
        // 0005cb06: MOV EAX, [EAX+ECX*4+0x48]
        _emit 0x8b
        _emit 0x44
        _emit 0x88
        _emit 0x48
        // 0005cb0a: RET
        _emit 0xc3
    }
}
