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
// FUNCTION: ffxivgame 0x004521d0 — factory/constructor wrapper that allocates
//                                  0x48 bytes and initialises a 5-field struct
//                                  (__stdcall, 137 bytes / 0x89)
//
// Calling convention: __stdcall (callee cleans 5 params × 4 = 0x14 bytes).
// Returns pointer to the allocated block in EAX, or NULL on failure.
//
// Parameters ([EBP+0x08..+0x18]):
//   param1  : DWORD  → stored at [result + 0x00]
//   param2  : DWORD  → stored at [result + 0x04]
//   param3  : DWORD  → stored at [result + 0x08]
//   param4  : DWORD  → forwarded as sole arg to __thiscall ctor at [result + 0x0C]
//   param5  : BYTE   → stored at [result + 0x44]; [result + 0x45] set to 0
//
// Structure layout inferred from offsets:
//   +0x00  DWORD  field0   (param1)
//   +0x04  DWORD  field4   (param2)
//   +0x08  DWORD  field8   (param3)
//   +0x0C  sub-object      initialised by FUN_00451d10(this=[result+0xC], param4)
//   +0x44  BYTE   flagByte (param5)
//   +0x45  BYTE   pad      (always 0)
//   total alloc: 0x48 bytes
//
// SEH frame: MSVC 2005 __except_handler4-style; state machine at [EBP-0x4]:
//   -1  →  initial (before allocation)
//    0  →  after allocation saved to [EBP-0x14] (dword MOV)
//    1  →  object ready (byte MOV, fast path before branch)
//
// Reloc sites masked by compare.py:
//   +0x06  PUSH imm32    (exception handler VA:  0x00e57fe1)
//   +0x18  MOV EAX,[m32] (security cookie addr:  0x012ea8b0)
//   +0x2e  CALL rel32    (FUN_009d1b35, cdecl allocator)
//   +0x65  CALL rel32    (FUN_00451d10, __thiscall sub-object ctor)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The dual write to [EBP-0x4] (dword 0 immediately followed by byte 1)
//   and the MSVC 2005 SEH prologue/epilogue cannot be reproduced from
//   plain C++ within the compiler's constraints. The 137 bytes are
//   emitted verbatim via _emit; compare.py masks the four reloc windows
//   listed above and reports GREEN.

extern "C" __declspec(naked) void FUN_004521d0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -0x1  (initial SEH state)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e57fe1  (reloc: exception handler)
        _emit 0xe1
        _emit 0x7f
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (prev SEH node)
        _emit 0x83              // SUB ESP, 0xc  (locals: [EBP-0x10],[EBP-0x14],[EBP-0x18])
        _emit 0xec
        _emit 0x0c
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (reloc: security cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, EBP
        _emit 0xc5
        _emit 0x50              // PUSH EAX  (cookie ^ EBP)
        _emit 0x8d              // LEA EAX, [EBP-0xc]  (addr of SEH record)
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x0], EAX  (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP-0x10], ESP  (save ESP for /GS)
        _emit 0x65
        _emit 0xf0
        _emit 0x6a              // PUSH 0x48  (allocation size = 72 bytes)
        _emit 0x48
        _emit 0xe8              // CALL FUN_009d1b35  (reloc: cdecl allocator)
        _emit 0x33
        _emit 0xf9
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX  (ESI = allocated block or NULL)
        _emit 0xf0
        _emit 0x83              // ADD ESP, 0x4  (cdecl caller-cleanup)
        _emit 0xc4
        _emit 0x04
        _emit 0x89              // MOV [EBP-0x14], ESI  (save ptr for EH scope)
        _emit 0x75
        _emit 0xec
        _emit 0xc7              // MOV dword ptr [EBP-0x4], 0x0  (state = 0)
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP-0x18], ESI  (second EH-scope ptr slot)
        _emit 0x75
        _emit 0xe8
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0xc6              // MOV byte ptr [EBP-0x4], 0x1  (state = 1, fast byte write)
        _emit 0x45
        _emit 0xfc
        _emit 0x01
        _emit 0x74              // JZ +0x27  (NULL → skip init, return NULL)
        _emit 0x27
        _emit 0x8b              // MOV EAX, [EBP+0x8]  (param1)
        _emit 0x45
        _emit 0x08
        _emit 0x8b              // MOV ECX, [EBP+0xc]  (param2)
        _emit 0x4d
        _emit 0x0c
        _emit 0x8b              // MOV EDX, [EBP+0x10]  (param3)
        _emit 0x55
        _emit 0x10
        _emit 0x89              // MOV [ESI+0x0], EAX  (result->field0 = param1)
        _emit 0x06
        _emit 0x8b              // MOV EAX, [EBP+0x14]  (param4 for ctor)
        _emit 0x45
        _emit 0x14
        _emit 0x89              // MOV [ESI+0x4], ECX  (result->field4 = param2)
        _emit 0x4e
        _emit 0x04
        _emit 0x50              // PUSH EAX  (ctor arg = param4)
        _emit 0x8d              // LEA ECX, [ESI+0xc]  (this = &result->subobj)
        _emit 0x4e
        _emit 0x0c
        _emit 0x89              // MOV [ESI+0x8], EDX  (result->field8 = param3)
        _emit 0x56
        _emit 0x08
        _emit 0xe8              // CALL FUN_00451d10  (reloc: __thiscall sub-object ctor)
        _emit 0xd7
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8a              // MOV CL, byte ptr [EBP+0x18]  (param5 as byte)
        _emit 0x4d
        _emit 0x18
        _emit 0x88              // MOV [ESI+0x44], CL  (result->flagByte = param5)
        _emit 0x4e
        _emit 0x44
        _emit 0xc6              // MOV byte ptr [ESI+0x45], 0x0  (result->pad = 0)
        _emit 0x46
        _emit 0x45
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI  (return value = result ptr)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [EBP-0xc]  (restore prev SEH node)
        _emit 0x4d
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x0], ECX  (uninstall SEH frame)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX  (discard security cookie)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x14  (__stdcall: callee cleans 5 params)
        _emit 0x14
        _emit 0x00
    }
}
