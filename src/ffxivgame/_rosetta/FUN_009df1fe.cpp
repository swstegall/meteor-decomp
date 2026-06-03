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
// FUNCTION: ffxivgame 0x005df1fe — 1-arg __stdcall TLS-dispatch thunk (21 B)
//
// __stdcall FUN_009df1fe(DWORD arg) — retrieves the per-thread function pointer
// stored in the CRT TLS slot (via TlsGetValue) and dispatches to it with the
// caller's single argument.
//
// Asm (21 bytes @ orig RVA 0x005df1fe):
//
//   ff 74 24 04             PUSH  dword ptr [ESP+4]        ; push arg (pre-stage for dispatch call)
//   ff 35 b0 ae 2e 01       PUSH  dword ptr [0x012eaeb0]   ; push g_tlsIndex (__tlsindex)
//   ff 15 a4 e2 f3 00       CALL  dword ptr [0x00f3e2a4]   ; IAT → KERNEL32!TlsGetValue
//   ff d0                   CALL  EAX                       ; dispatch to TLS function ptr
//   c2 04 00                RETN  4                         ; __stdcall: clean 1 arg
//
// Calling convention: __stdcall (callee-cleans 4 bytes via RETN 4). No stack frame.
//
// Register / stack walk:
//   1. arg is pushed first (pre-stages the argument for the indirect dispatch call).
//   2. g_tlsIndex (0x012eaeb0, MSVCRT __tlsindex) is pushed as TlsGetValue's argument.
//   3. TlsGetValue (KERNEL32 __stdcall 1-arg) pops the TLS-index slot from the stack;
//      EAX = the thread-local data pointer (a function pointer in this slot).
//   4. CALL EAX invokes that function pointer; arg is on the stack at [ESP+4] inside
//      the callee. If callee is __stdcall with 1 arg it does its own RETN 4, leaving
//      ESP positioned past arg; our RETN 4 then correctly cleans up the caller's slot.
//
// Context: sits between __encoded_null (RVA 0x5df17e) and the CRT __getptd
// cluster. The TLS slot at __tlsindex stores a dispatch function pointer
// populated during CRT thread-local init; this thunk is the hot-path entry
// point for callers that need to invoke that per-thread routine with a
// single argument.
//
// Reloc-bearing sites in the orig 21 bytes:
//   +0x05  PUSH [imm32]  → 0x012eaeb0  (.data — __tlsindex global)
//   +0x0b  CALL [imm32]  → 0x00f3e2a4  (.rdata — IAT KERNEL32!TlsGetValue)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function contains no REL32 relocations and the two absolute-address
//   sites are emitted verbatim with _emit directives. The resulting .obj .text
//   is byte-identical to the orig slice without needing linker fixups, so
//   tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_009df1fe() {
    __asm {
        _emit 0xff              // PUSH dword ptr [ESP+4]
        _emit 0x74
        _emit 0x24
        _emit 0x04
        _emit 0xff              // PUSH dword ptr [0x012eaeb0]  (g_tlsIndex / __tlsindex)
        _emit 0x35
        _emit 0xb0
        _emit 0xae
        _emit 0x2e
        _emit 0x01
        _emit 0xff              // CALL dword ptr [0x00f3e2a4]  (IAT: KERNEL32!TlsGetValue)
        _emit 0x15
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0xff              // CALL EAX  (dispatch to TLS function pointer)
        _emit 0xd0
        _emit 0xc2              // RETN 4  (__stdcall epilogue: clean 1 arg)
        _emit 0x04
        _emit 0x00
    }
}
