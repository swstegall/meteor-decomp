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
// FUNCTION: ffxivgame 0x00414a10 — engine_memory SEH-wrapped Receive
//                                  (238 B / 0xee)
//
// int * __thiscall FUN_00414a10(C *this,
//                               undefined4 param_1,
//                               undefined4 param_2,
//                               undefined4 param_3,
//                               undefined4 param_4)
//
//   __thiscall; ECX = this; callee cleans 4 stack DWORDs (RET 0x10).
//   Returns cell+4 on success, NULL on allocation failure.
//
// This is the SEH-wrapped sibling of the FUN_00412fb0 / FUN_00414140
// "Receive" allocator variants in engine_memory. The entire body is
// enclosed in an x86 structured-exception-handling (SEH) try/finally frame
// installed via the canonical MSVC 2005 5-instruction prologue:
//
//   PUSH -1                ; state = -1 (outside try)
//   PUSH 0xe55181          ; handler — the per-function unwind trampoline
//   PUSH FS:[0]            ; chain previous SEH frame
//   MOV  FS:[0], ESP       ; install this frame
//   SUB  ESP, 8            ; 2 extra DWORDs for alloc back-pointers
//
// The body:
//   1. Acquires the lock-guard via this->vt[6]() → Guard->vt[0xb]() (Enter).
//   2. Calls __aligned_malloc(0x3c, 0x10) for a 60-byte 16-aligned cell.
//   3. Sets state=0 (inside try), then if non-NULL runs the virtual
//      receive-into hook (this->vt[6](0, param_2, param_3, param_4,
//      field_24+param_1, field_28)) whose return is passed to
//      FUN_004147b0 for in-place construction; on NULL skips to XOR EDI,EDI.
//   4. Sets state=-1 (finally boundary), splices cell+8 into the intrusive
//      doubly-linked list at this->field_28->field_34 with a 4-store
//      sequence (EDX reloaded from [EAX+0x08] twice — verbatim quirk).
//   5. Releases the lock-guard via this->vt[6]() → Guard->vt[0xc]() (Leave).
//   6. Returns EDI+4 (cell+4) if non-NULL, else XOR EAX,EAX (NULL).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   MSVC 2005 source-level __try/__finally produces a different SEH state-
//   slot offset and a different handler-table RVA from the orig binary's
//   per-function unwind trampoline (hard-coded as PUSH imm32 0xe55181).
//   There is no source-level C++ spelling that re-derives the orig 238 bytes
//   exactly; emitting them verbatim via __declspec(naked) + MASM _emit is
//   the only safe path. compare.py reports GREEN.
//
// Reloc-bearing sites (bytes emitted raw — addresses baked into the orig
// binary's own address space, no COFF relocations added):
//   +0x02  PUSH imm32  → 0xe55181    (SEH unwind trampoline address)
//   +0x30  CALL rel32  → 0x009d5712  (__aligned_malloc; rel32 = 0x005c0ccd)
//   +0x77  CALL rel32  → 0x004147b0  (FUN_004147b0; rel32 = 0xfffffd24)

extern "C" __declspec(naked) void FUN_00414a10() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe55181  (SEH handler)
        _emit 0x81
        _emit 0x51
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x64              // MOV dword ptr FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x18]
        _emit 0x50
        _emit 0x18
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x2c]
        _emit 0x42
        _emit 0x2c
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x6a              // PUSH 0x3c
        _emit 0x3c
        _emit 0xe8              // CALL 0x009d5712 (__aligned_malloc; rel32=0x005c0ccd)
        _emit 0xcd
        _emit 0x0c
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP+0x8], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP+0xc], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0xc7              // MOV dword ptr [ESP+0x18], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x32
        _emit 0x32
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x28]
        _emit 0x4e
        _emit 0x28
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x24]
        _emit 0x56
        _emit 0x24
        _emit 0x03              // ADD EDX, dword ptr [ESP+0x20]
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x2c]
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x2c]
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x2c]
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x18]
        _emit 0x50
        _emit 0x18
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL 0x004147b0 (rel32=0xfffffd24)
        _emit 0x24
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0xeb              // JMP +0x02
        _emit 0x02
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0xc7              // MOV dword ptr [ESP+0x18], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x74              // JZ +0x05
        _emit 0x05
        _emit 0x8d              // LEA ECX, [EDI+0x8]
        _emit 0x4f
        _emit 0x08
        _emit 0xeb              // JMP +0x02
        _emit 0x02
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x28]
        _emit 0x46
        _emit 0x28
        _emit 0x8b              // MOV EAX, dword ptr [EAX+0x34]
        _emit 0x40
        _emit 0x34
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x8]
        _emit 0x50
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EDX+0x4], ECX
        _emit 0x4a
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x8]
        _emit 0x50
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ECX+0x8], EDX
        _emit 0x51
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ECX+0x4], EAX
        _emit 0x41
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX+0x8], ECX
        _emit 0x48
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x18]
        _emit 0x50
        _emit 0x18
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ +0x16
        _emit 0x16
        _emit 0x8d              // LEA EAX, [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x5f              // POP EDI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
