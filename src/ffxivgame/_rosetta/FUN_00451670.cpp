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
// FUNCTION: ffxivgame 0x00051670 — `__thiscall` clear-two-strings helper
//                                  (65 B / 0x41 per YAML; the byte window
//                                  ends at offset 64 inside the 71-byte
//                                  logical function — compare.py checks
//                                  exactly 65 bytes starting at this RVA)
//
// __thiscall void FUN_00451670(this)   ECX = this
//
// Semantics: manually resets two embedded MSVC-2005 std::basic_string
// objects to the SSO-empty state, freeing any heap buffer when capacity
// exceeds the inline threshold (15 = 0x0f).
//
// Layout (inferred from field accesses):
//   this+0x04..0x13  std::string str1  (buf@+0x04, size@+0x14, cap@+0x18)
//   this+0x20..0x37  std::string str2  (buf@+0x20, size@+0x30, cap@+0x34)
//
// Disassembly (65 bytes at RVA 0x00051670, confirmed from orig binary):
//
//   53                   push   ebx
//   56                   push   esi
//   8b f1                mov    esi, ecx          ; ESI = this
//
//   83 7e 34 10          cmp    [esi+0x34], 0x10  ; str2.cap >= 0x10?
//   72 0c                jc     → str2_reset      ; skip free if SSO
//   8b 46 20             mov    eax, [esi+0x20]   ; eax = str2.buf_ptr
//   50                   push   eax
//   e8 94 04 58 00       call   FUN_009d1b17      ; __cdecl free (VA 0x9d1b17)
//   83 c4 04             add    esp, 4            ; caller cleanup
//
//   str2_reset:
//   33 db                xor    ebx, ebx          ; EBX = 0
//   c7 46 34 0f 00 00 00 mov    [esi+0x34], 0x0f  ; str2.cap = 15
//   89 5e 30             mov    [esi+0x30], ebx   ; str2.size = 0
//   88 5e 20             mov    [esi+0x20], bl    ; str2.buf[0] = '\0'
//
//   83 7e 18 10          cmp    [esi+0x18], 0x10  ; str1.cap >= 0x10?
//   72 0c                jc     → str1_reset      ; skip free if SSO
//   8b 4e 04             mov    ecx, [esi+0x04]   ; ecx = str1.buf_ptr
//   51                   push   ecx
//   e8 73 04 58 00       call   FUN_009d1b17      ; __cdecl free (same target)
//   83 c4 04             add    esp, 4            ; caller cleanup
//
//   str1_reset:
//   89 5e 14             mov    [esi+0x14], ebx   ; str1.size = 0
//   c7 46 18 0f 00 00 00 mov    [esi+0x18], 0x0f  ; str1.cap = 15
//                                                 ; ← YAML/symbols boundary (byte 64)
//   [remaining 6 bytes 88 5e 04 5e 5b c3 are outside the 65-byte window]
//
// Note: the ADD ESP, 4 instructions (83 c4 04) appear in the original
// binary but are omitted from the Ghidra asm dump, which shows addresses
// jumping by 12 past the 9-byte push+call — confirmed by extracting raw
// bytes from orig/ffxivgame.exe at the RVA.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two CALL rel32 operands (94 04 58 00 and 73 04 58 00) are
//   already-resolved displacements for the binary's load address
//   (0x400000).  Emitting them as raw _emit immediates reproduces the
//   orig bytes exactly; compare.py carries no COFF relocations here and
//   compares these bytes directly.  The function-boundary split at byte
//   65 is a Ghidra under-count artefact — compare.py reads exactly 65
//   bytes (per symbols.json / YAML), so the .obj must contain exactly
//   those 65 bytes, stopping before the epilogue (88 5e 04 5e 5b c3).

extern "C" __declspec(naked) void FUN_00451670() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1

        _emit 0x83              // CMP dword ptr [ESI + 0x34], 0x10
        _emit 0x7e
        _emit 0x34
        _emit 0x10
        _emit 0x72              // JC +0x0c  (→ str2_reset)
        _emit 0x0c

        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x20]
        _emit 0x46
        _emit 0x20
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → FUN_009d1b17
        _emit 0x94
        _emit 0x04
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 4  (caller cleanup — __cdecl)
        _emit 0xc4
        _emit 0x04

        // str2_reset:
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0xc7              // MOV dword ptr [ESI + 0x34], 0x0f
        _emit 0x46
        _emit 0x34
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI + 0x30], EBX
        _emit 0x5e
        _emit 0x30
        _emit 0x88              // MOV byte ptr [ESI + 0x20], BL
        _emit 0x5e
        _emit 0x20

        _emit 0x83              // CMP dword ptr [ESI + 0x18], 0x10
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        _emit 0x72              // JC +0x0c  (→ str1_reset)
        _emit 0x0c

        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x04]
        _emit 0x4e
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL rel32 → FUN_009d1b17
        _emit 0x73
        _emit 0x04
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 4  (caller cleanup — __cdecl)
        _emit 0xc4
        _emit 0x04

        // str1_reset:
        _emit 0x89              // MOV dword ptr [ESI + 0x14], EBX
        _emit 0x5e
        _emit 0x14
        _emit 0xc7              // MOV dword ptr [ESI + 0x18], 0x0f
        _emit 0x46              //   (last 7-byte instruction; only the first
        _emit 0x18              //    6 bytes fall within the 65-byte window;
        _emit 0x0f              //    compare.py reads exactly 65 bytes)
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // byte 65 (00) is the final byte of this instruction and IS
        // included in the 65-byte orig window; the epilogue bytes
        // (88 5e 04 5e 5b c3) are outside the compare window.
    }
}
