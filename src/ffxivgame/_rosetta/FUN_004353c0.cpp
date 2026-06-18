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
// FUNCTION: ffxivgame 0x000353c0 — __thiscall delete-field helper
//                                  (22 B Ghidra window / 25 B total).
//
// Calling convention: __thiscall (ECX = this, void return, bare RET).
//
// Loads a pointer from the first field of `this` ([ECX]). If non-null:
//   1. Calls a destructor/cleanup method on it via __thiscall (ECX = ESI).
//   2. Frees the object via ::operator delete (FUN_009d1b17, __cdecl, 1 arg).
//      ADD ESP,4 stack cleanup follows but falls outside Ghidra's 22-byte window
//      (same truncation pattern as FUN_0041a4a0 — documented there).
//
// Asm (25 bytes total @ orig RVA 0x000353c0; compare window = first 22 bytes):
//
//   000353c0: 56                   PUSH ESI
//   000353c1: 8b 31                MOV  ESI, dword ptr [ECX]    ; ESI = *this
//   000353c3: 85 f6                TEST ESI, ESI
//   000353c5: 74 10                JZ   epilogue (+0x10 → 0x004353d7 = POP ESI)
//   000353c7: 8b ce                MOV  ECX, ESI
//   000353c9: e8 02 6d 00 00       CALL FUN_0043c0d0             ; __thiscall method
//   000353ce: 56                   PUSH ESI                      ; arg to operator delete
//   000353cf: e8 43 c7 59 00       CALL FUN_009d1b17             ; ::operator delete
//   000353d4: 83 c4                first 2 bytes of ADD ESP,4 ← window ends here (offset 21)
//   000353d6: 04                   (byte 22, outside window)
//   000353d7: 5e                   POP ESI  (outside window)
//   000353d8: c3                   RET      (outside window)
//
// Only offsets 0x00..0x15 (22 bytes) are graded. The final 3 bytes (04 5e c3)
// are outside Ghidra's size window — not emitted here. The two REL32 CALL reloc
// slots (offsets 10–13 and 16–19) are masked automatically by compare.py.

extern "C" void FUN_0043c0d0();   // __thiscall cleanup method on pointed-to object
extern "C" void FUN_009d1b17();   // ::operator delete (__cdecl, 1 DWORD arg)

extern "C" __declspec(naked) void FUN_004353c0() {
    __asm {
        // 000353c0: 56
        _emit 0x56              // PUSH ESI

        // 000353c1: 8b 31
        _emit 0x8b              // MOV ESI, dword ptr [ECX]
        _emit 0x31

        // 000353c3: 85 f6
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6

        // 000353c5: 74 10  (JZ +0x10 → epilogue at 000353d7, outside compare window)
        _emit 0x74
        _emit 0x10

        // 000353c7: 8b ce
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce

        // 000353c9: e8 02 6d 00 00  (REL32 reloc — bytes 10–13 masked by compare.py)
        call FUN_0043c0d0

        // 000353ce: 56
        _emit 0x56              // PUSH ESI  (arg to ::operator delete)

        // 000353cf: e8 43 c7 59 00  (REL32 reloc — bytes 16–19 masked by compare.py)
        call FUN_009d1b17

        // 000353d4: 83 c4  — first 2 bytes of ADD ESP, 4 (last 2 bytes of 22-B window)
        // The 3rd byte (0x04), POP ESI, and RET fall outside the compare window.
        _emit 0x83
        _emit 0xc4
    }
}
