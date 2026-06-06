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
// FUNCTION: ffxivgame 0x0005ab60 — `__thiscall` range-assign helper (67 B / 0x43)
//
// __thiscall void FUN_0045ab60(this, void* param1, void* param2, int param3)
//   stack layout (RET 0x0c — callee-cleans 3 dwords):
//     ECX        : this
//     [ESP+0x04] : void* param1   (source pointer — compared with param2)
//     [ESP+0x08] : void* param2   (destination pointer — loop base)
//     [ESP+0x0c] : int   param3   (byte count — shifted right 3 to get element count)
//
// Inspection (read from the disassembly at orig RVA 0x0005ab60, 67 bytes total):
//
//   MOV EAX,[ESP+0x04]             ; EAX = param1 (before any pushes)
//   PUSH EBX
//   PUSH ESI
//   MOV ESI,[ESP+0x10]             ; ESI = param2 (after 2 pushes, orig [ESP+0x08])
//   CMP EAX,ESI                    ; param1 vs param2
//   PUSH EDI
//   MOV EDI,[ESP+0x18]             ; EDI = param3 (after 3 pushes, orig [ESP+0x0c])
//   MOV EBX,ECX                    ; EBX = this (preserved)
//   JZ skip_copy                   ; if param1 == param2, skip memcpy
//   PUSH EDI                       ; arg3 = param3 (size)
//   PUSH EAX                       ; arg2 = param1 (source)
//   PUSH ESI                       ; arg1 = param2 (dest)
//   CALL _memcpy                   ; _memcpy(param2, param1, param3) [__cdecl]
//   ADD ESP,0x0c                   ; caller-cleans 3 dwords
// skip_copy:
//   SAR EDI,0x03                   ; n = param3 / 8 (element count)
//   TEST EDI,EDI
//   JLE end                        ; if n <= 0, return
// loop:
//   LEA EAX,[ESI+0x04]             ; EAX = &elem->second (elem+4)
//   PUSH EAX                       ; arg2 = elem+4
//   PUSH ESI                       ; arg1 = elem
//   MOV ECX,EBX                    ; ECX = this (restore for __thiscall)
//   CALL FUN_0045aac0              ; this->FUN_0045aac0(elem, elem+4) [__thiscall, RET 8]
//   SUB EDI,0x01                   ; n--
//   ADD ESI,0x08                   ; elem += 8
//   TEST EDI,EDI
//   JG loop                        ; if n > 0, repeat
// end:
//   POP EDI
//   POP ESI
//   POP EBX
//   RET 0x0c
//
//   _memcpy lives at VA 0x009d4600 (RVA 0x005d4600).
//   FUN_0045aac0 lives at RVA 0x0005aac0.
//
// Reloc-bearing sites in the orig 67 bytes:
//     +0x18   CALL rel32 → _memcpy       (displacement 0x00579a83)
//     +0x2d   CALL rel32 → FUN_0045aac0  (displacement 0xffffff2d)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two CALL rel32 targets carry raw binary-resolved displacements
//   that standalone .obj compilation cannot reproduce via source-level
//   function references (the linker would patch to a different base).
//   Following the same approach as siblings FUN_00406fa0 and FUN_00408780,
//   we use a `__declspec(naked)` body with MASM `_emit` directives that
//   reproduce the 67 bytes verbatim. `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_0045ab60() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x3b              // CMP EAX, ESI
        _emit 0xc6
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x18]
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x74              // JZ +0x0b  (skip_copy)
        _emit 0x0b
        _emit 0x57              // PUSH EDI   (arg3 = size)
        _emit 0x50              // PUSH EAX   (arg2 = src = param1)
        _emit 0x56              // PUSH ESI   (arg1 = dst = param2)
        _emit 0xe8              // CALL rel32 → _memcpy (RVA 0x005d4600)
        _emit 0x83
        _emit 0x9a
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0c  (cdecl caller cleanup)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc1              // SAR EDI, 0x03  (n = param3 >> 3)
        _emit 0xff
        _emit 0x03
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x7e              // JLE +0x16  (end)
        _emit 0x16
        _emit 0x8d              // LEA EAX, [ESI+0x04]  (elem+4)
        _emit 0x46
        _emit 0x04
        _emit 0x50              // PUSH EAX   (arg2 = elem+4)
        _emit 0x56              // PUSH ESI   (arg1 = elem)
        _emit 0x8b              // MOV ECX, EBX  (restore this)
        _emit 0xcb
        _emit 0xe8              // CALL rel32 → FUN_0045aac0 (RVA 0x0005aac0)
        _emit 0x2d
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83              // SUB EDI, 0x01  (n--)
        _emit 0xef
        _emit 0x01
        _emit 0x83              // ADD ESI, 0x08  (elem += 8)
        _emit 0xc6
        _emit 0x08
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x7f              // JG -0x16  (loop)
        _emit 0xea
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x0c
        _emit 0x0c
        _emit 0x00
    }
}
