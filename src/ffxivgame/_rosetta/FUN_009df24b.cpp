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
// FUNCTION: ffxivgame 0x005df24b — 2-arg __stdcall thiscall dispatch stub (25 B)
//
// __stdcall void FUN_009df24b(int a, int b):
//   Sets up args (a, b) on the stack, loads a global object pointer
//   [0x01363fc4], resolves a method via FUN_009df187 (the dynamic
//   function resolver at 0x5df187), and dispatches via __thiscall
//   (ECX = *global, stack = a, b).
//
// Calling convention: __stdcall (callee cleans 2 args via RET 8).
// Frame: none (no prologue / epilogue; raw ESP-relative addressing).
//
// Asm shape (25 bytes):
//
//   005df24b:  ff 74 24 08     PUSH DWORD PTR [ESP+8]   ; push arg2 (b)
//   005df24f:  ff 74 24 08     PUSH DWORD PTR [ESP+8]   ; push arg1 (a) [now at ESP+8]
//   005df253:  ff 35 c4 3f 36 01  PUSH DWORD PTR [g_dispatch_obj]  ; DIR32 reloc
//   005df259:  e8 29 ff ff ff  CALL FUN_009df187         ; REL32 reloc — cdecl resolver
//   005df25e:  59              POP ECX                   ; ECX = *g_dispatch_obj (this)
//   005df25f:  ff d0           CALL EAX                  ; __thiscall dispatch
//   005df261:  c2 08 00        RET 8                     ; stdcall: clean 2 args

// g_dispatch_obj at VA 0x01363fc4 (.data): global object pointer used as
// the 'this' receiver and resolver key for the dynamic method dispatch.
extern "C" int g_dispatch_obj;

// FUN_009df187 at VA 0x009df187: dynamic function resolver — takes the
// object value and returns a method pointer in EAX via __cdecl.
extern "C" void FUN_009df187();

extern "C" __declspec(naked) void FUN_009df24b() {
    __asm {
        push    dword ptr [esp+8]
        push    dword ptr [esp+8]
        push    dword ptr [g_dispatch_obj]
        call    FUN_009df187
        pop     ecx
        call    eax
        ret     8
    }
}
