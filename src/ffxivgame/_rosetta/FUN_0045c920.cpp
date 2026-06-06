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
// FUNCTION: ffxivgame 0x0005c920 — ERR_load_strings init trampoline (21 B)
//
// Asm (21 bytes @ orig RVA 0x0005c920):
//   53                    PUSH EBX
//   e8 fa fe ff ff        CALL 0x0045c820  ; _ERR_load_ERR_strings — init ERR subsystem
//   8b 4c 24 0c           MOV  ECX, [ESP+0xc]  ; arg2 (str array) → ECX (this for FUN_0045c240)
//   8b 5c 24 08           MOV  EBX, [ESP+0x8]  ; arg1 (lib) → EBX  (custom register param)
//   e8 0d f9 ff ff        CALL 0x0045c240  ; FUN_0045c240 — iterate and register strings
//   5b                    POP  EBX
//   c3                    RET
//
// Calling convention: outer function is __cdecl (RET without N).
//   arg1 (int lib)              at [ESP+4] on entry → [ESP+8] after PUSH EBX
//   arg2 (ERR_STRING_DATA* str) at [ESP+8] on entry → [ESP+0xc] after PUSH EBX
//
// FUN_0045c240 (RVA 0x0005c240) uses a non-standard calling convention:
//   ECX = pointer to the ERR_STRING_DATA array  (iterated like a thiscall this)
//   EBX = lib number (low byte ORed into the first entry's error field if non-zero)
//
// _ERR_load_ERR_strings (RVA 0x0005c820) takes no arguments and is the
// OpenSSL ERR_load_ERR_strings() initialiser that registers the subsystem.
//
// Reconstruction strategy — naked-asm with named symbol calls:
//   The two CALL instructions produce e8 + rel32 relocations in the .obj;
//   compare.py masks those 4-byte reloc windows, so symbol references
//   compile to the correct opcode byte (0xe8) at the right offset while the
//   operand words are treated as wildcards. All other bytes are fixed and
//   must match byte-for-byte.

extern "C" void ERR_load_ERR_strings(void);
extern "C" void FUN_0045c240(void);

extern "C" __declspec(naked) void FUN_0045c920() {
    __asm {
        push ebx
        call ERR_load_ERR_strings
        mov ecx, [esp+0ch]
        mov ebx, [esp+08h]
        call FUN_0045c240
        pop ebx
        ret
    }
}
