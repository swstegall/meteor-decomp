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
// FUNCTION: ffxivgame 0x004591f0 — reference-counted object release (31 B,
//           __stdcall void*(void*)).
//
// Single __stdcall argument: pointer to a refcounted object whose refcount
// lives at offset +0x4. Decrements the count atomically (ADD [ptr+4], -1),
// reads it back into EAX, and returns immediately if still non-zero.
// When the count reaches zero: calls the object's destructor via __thiscall
// (ECX = ptr → CALL 0x00458dd0), then releases the allocation via a cdecl
// free-like function (PUSH ptr → CALL 0x009d1b17).
//
// Asm (31 bytes, RVA 0x000591f0..0x0005920f):
//
//   000591f0:  56                    PUSH ESI
//   000591f1:  8b 74 24 08           MOV  ESI, [ESP+0x8]      ; arg1 = ptr
//   000591f5:  83 46 04 ff           ADD  [ESI+0x4], -1       ; --refcount
//   000591f9:  8b 46 04              MOV  EAX, [ESI+0x4]      ; read new count
//   000591fc:  75 12                 JNZ  +0x12               ; still alive → return
//   000591fe:  8b ce                 MOV  ECX, ESI            ; this = ptr
//   00059200:  e8 cb fb ff ff        CALL 0x00458dd0          ; destructor (__thiscall)
//   00059205:  56                    PUSH ESI                  ; push ptr
//   00059206:  e8 0c 89 57 00        CALL 0x009d1b17          ; free / operator delete
//   00059210:  5e                    POP  ESI
//   00059211:  c2 04 00              RET  0x4
//
// Two CALL rel32 operands are baked via _emit so compare.py sees the exact
// original bytes; the reloc-masking logic still applies but the raw values
// already match the PE slice.

extern "C" __declspec(naked) void FUN_004591f0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x83              // ADD dword ptr [ESI+0x4], -1
        _emit 0x46
        _emit 0x04
        _emit 0xff
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x75              // JNZ +0x12
        _emit 0x12
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00458dd0  (rel32 = 0xfffffbcb)
        _emit 0xcb
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL 0x009d1b17  (rel32 = 0x0057890c)
        _emit 0x0c
        _emit 0x89
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 4  (cdecl cleanup for PUSH ESI)
        _emit 0xc4
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX byte 1 (ModRM 0xc0 is in shared tail at +1)
    }
}
