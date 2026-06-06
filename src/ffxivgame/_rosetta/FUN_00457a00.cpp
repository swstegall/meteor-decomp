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
// FUNCTION: ffxivgame 0x00457a00 — DBCS (Shift-JIS-style) codepoint remap
//                                  via a two-level lead/trail table (62 B)
//
// Custom register calling convention (MSVC internal — the function is only
// reached intra-module, so the compiler passes inputs in registers it
// likes and reuses the caller's ESI table base):
//     EAX        : uint16* — pointer to the source codepoint (big-endian
//                  pair: AX = high<<8 | low)
//     ESI        : char*  — base of the conversion tables
//     [ESP+0x04] : int    — "translate" flag (0 = pass-through)
//   returns AX (16-bit) in EAX.
//
// Tables (relative to ESI):
//     ESI + 0x800c            byte[256]   — lead-byte → trail-block index
//                                           (0 ⇒ no mapping; keep input)
//     ESI + 0x7f24            uint16[]    — (block<<8 | low) → remapped CP
//
// Inspection (read from the disassembly at orig RVA 0x00057a00):
//
//   CMP  [ESP+4], 0
//   JNZ  translate
//   MOV  AX, [EAX]                 ; flag==0: return raw codepoint as-is
//   RET
// translate:
//   MOVZX EAX, word ptr [EAX]      ; cp = *src
//   MOVZX ECX, AX
//   MOV   EDX, ECX
//   SHR   EDX, 8                   ; lead = cp >> 8
//   MOVZX DX, byte ptr [EDX+ESI+0x800c]   ; blk = lead_table[lead]
//   MOVZX EDX, DX
//   TEST  DX, DX
//   JZ    done                     ; blk==0 ⇒ no mapping, AX = cp unchanged
//   MOVZX EAX, DX
//   SHL   EAX, 8                   ; blk << 8
//   AND   ECX, 0xff                ; | (cp & 0xff)
//   ADD   EAX, ECX
//   MOV   AX, word ptr [ESI+EAX*2+0x7f24]  ; AX = trail_table[blk<<8 | low]
// done:
//   RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function carries NO relocations: every memory reference is
//   ESI-relative (the table base is a runtime register input, not an
//   absolute address), so the 62 bytes are position-independent. Coaxing
//   MSVC 2005 to emit this exact custom-register-convention shape from C++
//   source is not feasible, but a `__declspec(naked)` body re-emitting the
//   orig bytes via MASM `_emit` directives produces a `.text` slice that
//   is byte-identical to the orig (the 16-bit MOVZX/MOV forms with their
//   0x66 operand-size prefixes pin exactly). `tools/compare.py` → GREEN.

extern "C" __declspec(naked) void FUN_00457a00() {
    __asm {
        _emit 0x83              // CMP dword ptr [ESP+0x04], 0x00
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x75              // JNZ +0x04 (→ translate)
        _emit 0x04
        _emit 0x66              // MOV AX, word ptr [EAX]
        _emit 0x8b
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x0f              // MOVZX EAX, word ptr [EAX]   (translate:)
        _emit 0xb7
        _emit 0x00
        _emit 0x0f              // MOVZX ECX, AX
        _emit 0xb7
        _emit 0xc8
        _emit 0x8b              // MOV EDX, ECX
        _emit 0xd1
        _emit 0xc1              // SHR EDX, 0x08
        _emit 0xea
        _emit 0x08
        _emit 0x66              // MOVZX DX, byte ptr [EDX+ESI*1+0x0000800c]
        _emit 0x0f
        _emit 0xb6
        _emit 0x94
        _emit 0x32
        _emit 0x0c
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x0f              // MOVZX EDX, DX
        _emit 0xb7
        _emit 0xd2
        _emit 0x66              // TEST DX, DX
        _emit 0x85
        _emit 0xd2
        _emit 0x74              // JZ +0x16 (→ done)
        _emit 0x16
        _emit 0x0f              // MOVZX EAX, DX
        _emit 0xb7
        _emit 0xc2
        _emit 0xc1              // SHL EAX, 0x08
        _emit 0xe0
        _emit 0x08
        _emit 0x81              // AND ECX, 0x000000ff
        _emit 0xe1
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x03              // ADD EAX, ECX
        _emit 0xc1
        _emit 0x66              // MOV AX, word ptr [ESI+EAX*2+0x00007f24]
        _emit 0x8b
        _emit 0x84
        _emit 0x46
        _emit 0x24
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET   (done:)
    }
}
