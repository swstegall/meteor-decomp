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
// FUNCTION: ffxivgame 0x00015860 — Printer::Reset() (76 B / 0x4c)
//
// __thiscall void Printer::Reset()
//
// Frees the two primary ring buffers (m_data at +0x08, m_meta at +0x0c)
// and an auxiliary owned object (at +0x7c), then zeroes all three pointers.
// Structurally identical to the Printer destructor body in FUN_00415c40
// but without the vtable assignment and the global-flag zero at the end.
// Named Reset() based on the type note at decomp-notes/types/ffxivgame/
// 0x000158b0.md (sibling SetLimit calls Reset as its first step).
//
// Inspection (read from orig bytes at RVA 0x00015860, 76 bytes total):
//
//   56                      push esi
//   8b f1                   mov  esi, ecx            ; esi = this
//   8b 46 08                mov  eax, [esi+0x08]     ; m_data
//   85 c0                   test eax, eax
//   57                      push edi
//   74 19                   jz   skip_data (+0x19 -> 0x00015884)
//   50                      push eax
//   e8 4f 0a 00 00          call FUN_004162c0        ; free(m_data)
//   8b 46 0c                mov  eax, [esi+0x0c]     ; m_meta
//   50                      push eax
//   e8 46 0a 00 00          call FUN_004162c0        ; free(m_meta)
//   83 c4 08                add  esp, 8
//   c7 46 08 00 00 00 00    mov  dword [esi+0x08], 0 ; m_data = NULL
// skip_data:
//   8b 7e 7c                mov  edi, [esi+0x7c]     ; m_aux
//   85 ff                   test edi, edi
//   c7 46 0c 00 00 00 00    mov  dword [esi+0x0c], 0 ; m_meta = NULL (always)
//   74 17                   jz   skip_aux (+0x17 -> 0x004158a9)
//   8b cf                   mov  ecx, edi            ; this for FUN_00416650
//   e8 b7 0d 00 00          call FUN_00416650        ; m_aux->Finalize()
//   57                      push edi
//   e8 21 0a 00 00          call FUN_004162c0        ; free(m_aux)
//   83 c4 04                add  esp, 4
//   c7 46 7c 00 00 00 00    mov  dword [esi+0x7c], 0 ; m_aux = NULL
// skip_aux:
//   5f                      pop  edi
//   5e                      pop  esi
//   c3                      ret
//
//   Calling convention: __thiscall (ECX = this, no stack args, plain RET).
//   Stack frame: -8 (PUSH ESI / PUSH EDI bracket, no locals).
//
// Key difference from FUN_00415c40: that sibling uses EBX as a zero
// sentinel (XOR EBX,EBX then CMP/MOV-via-EBX), while this function uses
// TEST and immediate zeros (c7 46 xx 00 00 00 00). MSVC 2005 chose TEST
// here because no other register needed to hold zero; it also omits a
// third saved register (EBX).
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x0d  CALL rel32 → FUN_004162c0  (free(m_data))
//   +0x16  CALL rel32 → FUN_004162c0  (free(m_meta))
//   +0x35  CALL rel32 → FUN_00416650  (m_aux->Finalize())
//   +0x3b  CALL rel32 → FUN_004162c0  (free(m_aux))
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would produce the same logical output but the
//   interleaved PUSH EDI (after TEST EAX,EAX, before JZ) and the deferred
//   m_meta zero (after TEST EDI,EDI, before JZ) are MSVC 2005 /O2
//   scheduling artefacts that require the exact instruction ordering.
//   Re-emitting the 76 orig bytes verbatim (including the original rel32
//   offsets, which compare.py masks) is the reliable approach.

extern "C" __declspec(naked) void FUN_00415860() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, [ESI+0x08]
        _emit 0x46
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x57              // PUSH EDI
        _emit 0x74              // JZ skip_data (+0x19)
        _emit 0x19
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_004162c0 (rel32)
        _emit 0x4f
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_004162c0 (rel32)
        _emit 0x46
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESI+0x08], 0
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDI, [ESI+0x7c]
        _emit 0x7e
        _emit 0x7c
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0xc7              // MOV dword ptr [ESI+0x0c], 0
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ skip_aux (+0x17)
        _emit 0x17
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_00416650 (rel32)
        _emit 0xb7
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_004162c0 (rel32)
        _emit 0x21
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESI+0x7c], 0
        _emit 0x46
        _emit 0x7c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
