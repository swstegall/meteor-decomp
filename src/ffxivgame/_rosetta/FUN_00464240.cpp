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
// FUNCTION: ffxivgame 0x00464240 — skip two-null-byte sentinel (44 B / 0x2C)
//
// Reads param2 ([ESP+8], signed int) as a remaining-byte count and param1
// ([ESP+4], char**) as a pointer-to-buffer-pointer. Logic:
//   1. If param2 <= 0  → return 1  (sentinel not required, vacuously ok)
//   2. If param2 <  2  → return 0  (not enough bytes to contain sentinel)
//   3. Load ptr = *param1.
//   4. If ptr[0] != 0  → return 0  (first byte non-null)
//   5. If ptr[1] != 0  → return 0  (second byte non-null)
//   6. *param1 += 2; return 1      (consumed 2-byte null / UCS-2 NUL)
//
// Consistent with consuming a 16-bit null terminator (0x0000) from a
// byte-stream, advancing the cursor on success.
//
// Calling convention: __cdecl (2 stack args; caller cleans; plain RET).
// Stack frame: none (no PUSH EBP; function uses only EAX and ECX).
// Relocations: none (all jumps are short relative; no absolute addresses).
//
// Asm (44 bytes @ orig RVA 0x00064240):
//   8b 44 24 08        MOV  EAX, [ESP+8]         ; param2
//   85 c0              TEST EAX, EAX
//   7e 1b              JLE  return_1              ; <= 0 → 1
//   83 f8 02           CMP  EAX, 2
//   7c 1c              JL   return_0              ; == 1 → 0
//   8b 4c 24 04        MOV  ECX, [ESP+4]          ; param1
//   8b 01              MOV  EAX, [ECX]            ; ptr = *param1
//   80 38 00           CMP  byte ptr [EAX], 0
//   75 11              JNZ  return_0
//   80 78 01 00        CMP  byte ptr [EAX+1], 0
//   75 0b              JNZ  return_0
//   83 c0 02           ADD  EAX, 2
//   89 01              MOV  [ECX], EAX            ; *param1 = ptr+2
// return_1:
//   b8 01 00 00 00     MOV  EAX, 1
//   c3                 RET
// return_0:
//   33 c0              XOR  EAX, EAX
//   c3                 RET

extern "C" __declspec(naked) void FUN_00464240() {
    __asm {
        mov     eax, dword ptr [esp + 0x8]
        test    eax, eax
        jle     return_1
        cmp     eax, 2
        jl      return_0
        mov     ecx, dword ptr [esp + 0x4]
        mov     eax, dword ptr [ecx]
        cmp     byte ptr [eax], 0
        jnz     return_0
        cmp     byte ptr [eax + 1], 0
        jnz     return_0
        add     eax, 2
        mov     dword ptr [ecx], eax
    return_1:
        mov     eax, 1
        ret
    return_0:
        xor     eax, eax
        ret
    }
}
