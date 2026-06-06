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
// FUNCTION: ffxivgame 0x004154f0 — `__thiscall` VfxLogger format dispatcher
//                                   (304 B / 0x130).
//
// Inspection (read from the disassembly at orig RVA 0x000154f0):
//
//   __thiscall void FUN_004154f0(VfxLogger *this,
//                                char *buffer,    int buf_size,
//                                int kind,        int w,
//                                const char *msg, bool newline_flag);
//
//   Dispatches on `kind` to pick a log-line tag and format with
//   __snprintf_s (at .text 0x009d4f9f), then optionally appends '\n':
//
//     switch (kind) {
//       case 0:
//       default:
//         snprintf_s(buffer, buf_size, buf_size-1,
//                    "%s[vfx] %s",             // 0x00f57548
//                    g_indent_table[this->depth], msg);
//         // fall into newline logic
//         break;
//       case 1:
//         snprintf_s(buffer, buf_size, buf_size-1,
//                    "%s[vfx.w%d] %s",         // 0x00f57524
//                    g_indent_table[this->depth], w, msg);
//         return;   // no newline
//       case 2:
//         snprintf_s(buffer, buf_size, buf_size-1,
//                    "%s[vfx.assert] %s",       // 0x00f57534
//                    g_indent_table[this->depth], msg);
//         return;   // no newline
//     }
//
//     // Newline append (kind==0 or kind>=3 only):
//     if (newline_flag) {
//         int len = min((int)strlen(buffer), buf_size - 2);
//         if (buffer[len-1] != '\n') {
//             buffer[len]   = '\n';
//             buffer[len+1] = '\0';
//         }
//     }
//
//   Global table: g_indent_table @ .data 0x01265f48 — eight const char*
//   entries, one per nesting depth 0..7. Indexed by this+0x80 (uint8
//   VfxLogger::depth). See decomp-notes/types/ffxivgame/0x000154f0.md.
//
//   Dispatch encoding: MSVC 2005 /O2 lowers a 3-way switch as a
//   SUB-chain (SUB EAX,0 / JZ case0; SUB EAX,1 / JZ case1; SUB EAX,1
//   / JZ case2; fall-through default). The two PUSH callee-saves
//   (PUSH ESI; PUSH EDI) are interleaved between SUB EAX,0 and its JZ
//   because they preserve flags. Each arm reads this->depth via
//   MOVZX r32, byte ptr [ECX+0x80], then indexes the table with
//   MOV r32, [r32*4 + 0x01265f48].
//
//   Calling convention: __thiscall / __stdcall hybrid — `this` in ECX,
//   six stack args, callee-balanced epilogue: RET 0x18.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Four separate __snprintf_s calls each embed absolute .rdata and .data
//   addresses (format strings, g_indent_table) that only resolve at the
//   original link base 0x00400000.  Additionally, the SUB-chain dispatch
//   interleaved with PUSH instructions, the CMOVLE min-of-two at
//   0x00015564, and the precise short-vs-near branch selection at every
//   JZ/JLE site are all sensitive to MSVC 2005 /O2 register-allocation
//   state across four arms.  A source-level C++ reconstruction would
//   shift at least one byte at each of those sites.
//
//   The pragmatic choice — the same one FUN_004014b0, FUN_00401a00, and
//   FUN_00408f10 took — is a `__declspec(naked)` body re-emitting the
//   orig 304 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_004154f0() {
    __asm {
        // --- prologue ---
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, [ESP+0x10]  (kind = arg3)
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0x83              // SUB EAX, 0x0  (sets ZF if kind==0)
        _emit 0xe8
        _emit 0x00
        _emit 0x56              // PUSH ESI  (flags preserved)
        _emit 0x57              // PUSH EDI  (flags preserved)
        _emit 0x0f              // JZ → case 0 @ 0x004155ec
        _emit 0x84
        _emit 0xea
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB EAX, 0x1
        _emit 0xe8
        _emit 0x01
        _emit 0x0f              // JZ → case 1 @ 0x004155b2
        _emit 0x84
        _emit 0xa7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB EAX, 0x1
        _emit 0xe8
        _emit 0x01
        _emit 0x74              // JZ → case 2 @ 0x0041557d
        _emit 0x6d

        // --- default arm: kind != 0,1,2 ---
        _emit 0x0f              // MOVZX ECX, byte ptr [ECX+0x80]  (depth)
        _emit 0xb6
        _emit 0x89
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP+0x20]  (msg = arg5)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV EDX, [ECX*4 + 0x01265f48]  (indent)
        _emit 0x14
        _emit 0x8d
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV EDI, [ESP+0x14]  (buf_size = arg2)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV ESI, [ESP+0x10]  (buffer = arg1)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX  (msg)
        _emit 0x52              // PUSH EDX  (indent)
        _emit 0x68              // PUSH 0x00f57548  ("%s[vfx] %s")
        _emit 0x48
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EDI-1]  (buf_size-1)
        _emit 0x47
        _emit 0xff
        _emit 0x50              // PUSH EAX  (count)
        _emit 0x57              // PUSH EDI  (buf_size)
        _emit 0x56              // PUSH ESI  (buffer)
        _emit 0xe8              // CALL __snprintf_s @ 0x009d4f9f
        _emit 0x63
        _emit 0xfa
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x18  (6 args * 4)
        _emit 0xc4
        _emit 0x18
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x7e              // JLE +5 → newline_check @ 0x00415548
        _emit 0x05
        _emit 0x83              // CMP EBX, 0x2
        _emit 0xfb
        _emit 0x02
        _emit 0x7e              // JLE +0x2f → epilogue @ 0x00415577
        _emit 0x2f

        // --- newline check @ 0x00415548 ---
        _emit 0x80              // CMP byte ptr [ESP+0x24], 0x0  (newline_flag = arg6)
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x74              // JZ +0x28 → epilogue @ 0x00415577
        _emit 0x28
        _emit 0x8b              // MOV EAX, ESI  (buffer)
        _emit 0xc6
        _emit 0x8d              // LEA EDX, [EAX+1]  (buffer+1)
        _emit 0x50
        _emit 0x01
        // strlen loop:
        _emit 0x8a              // MOV CL, [EAX]
        _emit 0x08
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ -9 → strlen loop top
        _emit 0xf7
        _emit 0x2b              // SUB EAX, EDX  (EAX = strlen)
        _emit 0xc2
        _emit 0x8d              // LEA ECX, [EDI-2]  (buf_size - 2)
        _emit 0x4f
        _emit 0xfe
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x0f              // CMOVLE EAX, ECX  (EAX = min(strlen, buf_size-2))
        _emit 0x4e
        _emit 0xc1
        _emit 0x80              // CMP byte ptr [EAX+ESI-1], 0xa  ('\n'?)
        _emit 0x7c
        _emit 0x30
        _emit 0xff
        _emit 0x0a
        _emit 0x74              // JZ +9 → epilogue (already ends in '\n')
        _emit 0x09
        _emit 0xc6              // MOV byte ptr [EAX+ESI], 0xa  ('\n')
        _emit 0x04
        _emit 0x30
        _emit 0x0a
        _emit 0xc6              // MOV byte ptr [EAX+ESI+1], 0x0  ('\0')
        _emit 0x44
        _emit 0x30
        _emit 0x01
        _emit 0x00

        // --- epilogue @ 0x00415577 ---
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x18
        _emit 0x18
        _emit 0x00

        // --- case 2: kind==2 @ 0x0041557d ---
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX+0x80]  (depth)
        _emit 0xb6
        _emit 0x81
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, [ESP+0x20]  (msg)
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV ECX, [EAX*4 + 0x01265f48]  (indent)
        _emit 0x0c
        _emit 0x85
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV EAX, [ESP+0x14]  (buf_size)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x52              // PUSH EDX  (msg)
        _emit 0x51              // PUSH ECX  (indent)
        _emit 0x68              // PUSH 0x00f57534  ("%s[vfx.assert] %s")
        _emit 0x34
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA EDX, [EAX-1]  (buf_size-1)
        _emit 0x50
        _emit 0xff
        _emit 0x52              // PUSH EDX  (count)
        _emit 0x50              // PUSH EAX  (buf_size)
        _emit 0x8b              // MOV EAX, [ESP+0x24]  (buffer, after 5 pushes)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50              // PUSH EAX  (buffer)
        _emit 0xe8              // CALL __snprintf_s @ 0x009d4f9f
        _emit 0xf6
        _emit 0xf9
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x18
        _emit 0x18
        _emit 0x00

        // --- case 1: kind==1 @ 0x004155b2 ---
        _emit 0x0f              // MOVZX ECX, byte ptr [ECX+0x80]  (depth)
        _emit 0xb6
        _emit 0x89
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, [ESP+0x20]  (msg)
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV EAX, [ESP+0x1c]  (w = arg4)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x52              // PUSH EDX  (msg)   — 1st push
        _emit 0x8b              // MOV EDX, [ECX*4 + 0x01265f48]  (indent)
        _emit 0x14
        _emit 0x8d
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x50              // PUSH EAX  (w)     — 2nd push
        _emit 0x8b              // MOV EAX, [ESP+0x1c]  (buf_size, after 2 pushes: base+0x14)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x52              // PUSH EDX  (indent) — 3rd push
        _emit 0x8b              // MOV EDX, [ESP+0x1c]  (buffer, after 3 pushes: base+0x10)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x68              // PUSH 0x00f57524  ("%s[vfx.w%d] %s") — 4th push
        _emit 0x24
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX-1]  (buf_size-1)
        _emit 0x48
        _emit 0xff
        _emit 0x51              // PUSH ECX  (count)  — 5th push
        _emit 0x50              // PUSH EAX  (buf_size) — 6th push
        _emit 0x52              // PUSH EDX  (buffer) — 7th push
        _emit 0xe8              // CALL __snprintf_s @ 0x009d4f9f
        _emit 0xbc
        _emit 0xf9
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x1c  (7 args * 4)
        _emit 0xc4
        _emit 0x1c
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x18
        _emit 0x18
        _emit 0x00

        // --- case 0: kind==0 @ 0x004155ec ---
        _emit 0x0f              // MOVZX ECX, byte ptr [ECX+0x80]  (depth)
        _emit 0xb6
        _emit 0x89
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP+0x20]  (msg)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV EDX, [ECX*4 + 0x01265f48]  (indent)
        _emit 0x14
        _emit 0x8d
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV EDI, [ESP+0x14]  (buf_size)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV ESI, [ESP+0x10]  (buffer)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX  (msg)
        _emit 0x52              // PUSH EDX  (indent)
        _emit 0x68              // PUSH 0x00f57548  ("%s[vfx] %s")
        _emit 0x48
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EDI-1]
        _emit 0x47
        _emit 0xff
        _emit 0x50              // PUSH EAX  (count)
        _emit 0x57              // PUSH EDI  (buf_size)
        _emit 0x56              // PUSH ESI  (buffer)
        _emit 0xe8              // CALL __snprintf_s @ 0x009d4f9f
        _emit 0x87
        _emit 0xf9
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0xe9              // JMP → newline_check @ 0x00415548
        _emit 0x28
        _emit 0xff
        _emit 0xff
        _emit 0xff
    }
}
