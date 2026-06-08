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
// FUNCTION: ffxivgame 0x00438ab0 — __thiscall wrapper that constructs a
//                                  temporary 5-field stack object (with vtable
//                                  0x00f649b0) from four caller-supplied args
//                                  and this->field_4, then dispatches through
//                                  FUN_00435cf0 (62 B / 0x3e).
//
// Calling convention: __thiscall — ECX = this, four DWORD stack args,
//   callee cleans 0x10 via `RET 0x10`.
//
// Stack layout (offsets relative to entry ESP; after `SUB ESP, 0x14`):
//   [ESP+0x18]  arg1   (1st caller arg)
//   [ESP+0x1c]  arg2   (2nd caller arg)
//   [ESP+0x20]  arg3   (3rd caller arg)
//   [ESP+0x24]  arg4   (4th caller arg)
//
// Local struct assembled at [ESP+0x0..0x10] (5 DWORDs):
//   [ESP+0x00]  vtable pointer → 0x00f649b0
//   [ESP+0x04]  arg1
//   [ESP+0x08]  arg2
//   [ESP+0x0c]  arg4
//   [ESP+0x10]  arg3
//
// Then:
//   PUSH this->field_4          ; stack arg to FUN_00435cf0
//   LEA  ECX, [ESP+0x4]         ; ECX → &local_struct (after push, struct at ESP+4)
//   MOV  [ESP+0x4], 0x00f649b0  ; overwrite vtable slot in new ESP-relative terms
//   MOV  [ESP+0x14], EDX        ; arg3 into struct field_0x10 (new ESP-relative)
//   CALL FUN_00435cf0            ; __thiscall, 1 stack arg, callee does RET 4
//   ADD  ESP, 0x14
//   RET  0x10
//
// Note: FUN_00435cf0 is a virtual-call dispatch + assert-fail reporter wrapper.
//   ECX points to the local struct; the single stack arg is the outer this->field_4
//   (a Obj* / handle passed along for the virtual dispatch chain).
//
// Why naked asm: the instruction sequence involves a hard-coded vtable immediate
//   (0x00f649b0) written into a stack slot, a bespoke ESP-relative struct layout
//   with a non-intuitive arg reorder (arg3 and arg4 swap positions in the struct),
//   and a PUSH before the LEA+MOV that shifts all subsequent [ESP+N] offsets. No
//   C++ source form reliably round-trips to this exact encoding under MSVC 2005 /O2.
//
// Reloc-bearing sites: the CALL FUN_00435cf0 at +0x33 is REL32 (masked by
//   compare.py via the COFF relocation table). The MOV [ESP+0x4], 0x00f649b0
//   immediate at +0x27 is a raw literal — emitted verbatim, matches the orig.
//
// Asm (62 bytes):
//   83 ec 14                     SUB  ESP, 0x14
//   8b 44 24 18                  MOV  EAX, [ESP+0x18]
//   8b 54 24 1c                  MOV  EDX, [ESP+0x1c]
//   89 44 24 04                  MOV  [ESP+0x04], EAX
//   8b 44 24 24                  MOV  EAX, [ESP+0x24]
//   89 44 24 0c                  MOV  [ESP+0x0c], EAX
//   8b 41 04                     MOV  EAX, [ECX+0x4]
//   89 54 24 08                  MOV  [ESP+0x08], EDX
//   8b 54 24 20                  MOV  EDX, [ESP+0x20]
//   50                           PUSH EAX
//   8d 4c 24 04                  LEA  ECX, [ESP+0x4]
//   c7 44 24 04 b0 49 f6 00      MOV  dword ptr [ESP+0x4], 0x00f649b0
//   89 54 24 14                  MOV  [ESP+0x14], EDX
//   e8 08 d2 ff ff               CALL FUN_00435cf0
//   83 c4 14                     ADD  ESP, 0x14
//   c2 10 00                     RET  0x10

extern "C" void FUN_00435cf0();

extern "C" __declspec(naked) void FUN_00438ab0() {
    __asm {
        sub     esp, 0x14
        mov     eax, dword ptr [esp + 0x18]
        mov     edx, dword ptr [esp + 0x1c]
        mov     dword ptr [esp + 0x4], eax
        mov     eax, dword ptr [esp + 0x24]
        mov     dword ptr [esp + 0xc], eax
        mov     eax, dword ptr [ecx + 0x4]
        mov     dword ptr [esp + 0x8], edx
        mov     edx, dword ptr [esp + 0x20]
        push    eax
        lea     ecx, [esp + 0x4]
        mov     dword ptr [esp + 0x4], 0x00f649b0
        mov     dword ptr [esp + 0x14], edx
        call    FUN_00435cf0
        add     esp, 0x14
        ret     0x10
    }
}
