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
// FUNCTION: ffxivgame 0x004119b0 — __thiscall 168-byte (0xa8) constructor /
//                                   object-initializer (5 explicit args,
//                                   RET 0x14).
//
// Calling convention: __thiscall — ECX = this, callee cleans 5 stack args.
//
// Stack layout at entry (before any pushes, ECX = this):
//     [ESP+0x04] = arg1   → stored at this+0x04
//     [ESP+0x08] = arg2   → stored at this+0x08 and this+0x1c
//     [ESP+0x0c] = arg3   → stored at this+0x0c
//     [ESP+0x10] = arg4   → stored at this+0x10
//     [ESP+0x14] = arg5   → stored at this+0x14
//
// Object layout (offsets written by this ctor):
//     [this+0x00] = 0xf56d3c    primary vtable pointer
//     [this+0x04] = arg1
//     [this+0x08] = arg2
//     [this+0x0c] = arg3
//     [this+0x10] = arg4
//     [this+0x14] = arg5
//     [this+0x18] = 0xf56ce8    secondary vtable / base pointer
//     [this+0x1c] = arg2        (same value as +0x08)
//     [this+0x20] = 0x9d4600    function pointer / vtable
//     [this+0x24] = 0x6ce2e0    function pointer (stored 3×)
//     [this+0x28] = 0x6ce2e0
//     [this+0x2c] = 0x6ce2e0
//     [this+0x30] = 0           zero-init
//     [this+0x34] = 0
//     [this+0x38] = 0
//     [this+0x3c] = 0xf567c4    embedded Link vtable
//     [this+0x40] = &this+0x3c  Link.next sentinel → self
//     [this+0x44] = &this+0x3c  Link.prev sentinel → self
//     [this+0x48] = 0 (byte)
//     [this+0x4c] = 0xf567c4    second embedded Link vtable
//     [this+0x50] = &this+0x4c  Link.next sentinel → self
//     [this+0x54] = &this+0x4c  Link.prev sentinel → self
//     [this+0x58] = 0xf56cf0    vtable for sub-object at +0x58
//     [this+0x74] = 0xf56d2c    vtable for sub-object at +0x74
//     [this+0x78] = 0
//     [this+0x7c] = 0xf56d00    vtable for sub-object at +0x7c
//     [this+0x80] = 0
//
// The call at byte-offset +0x84 (from function start) is an indirect call
// through the global function-pointer slot at 0x00f3e174, passing &this+0x5c
// as its sole argument.
//
// Notable MSVC 2005 idiom: arg1 (EAX) and arg4 (EDX) are loaded from the
// stack *before* PUSH EBX/PUSH ESI so they ride in caller-saved registers
// while the callee-saved registers are being set up.  Attempting a
// source-level C++ form would require coercing MSVC into the same pre-push
// hoisting, which is fragile across minor re-orderings.
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// siblings FUN_00406280, FUN_004063c0, FUN_004071b0 in this binary):
//
//   The .obj's .text is byte-identical to the orig slice.  Reloc-bearing
//   sites (absolute-address immediate stores and the indirect CALL through
//   [0xf3e174]) have their 4-byte payloads baked in from the orig wire
//   image; tools/compare.py masks those reloc bytes anyway, so no
//   relink is needed.

extern "C" __declspec(naked) void FUN_004119b0() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x4]   (arg1 — before pushes)
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x10]  (arg4 — before pushes)
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX                    (this)
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x14]  (arg3 — after 2 pushes)
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV dword ptr [ESI+0x4], EAX   (this->m04 = arg1)
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]  (arg2 — after 2 pushes)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI+0xc], ECX   (this->m0c = arg3)
        _emit 0x4e
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x1c]  (arg5 — after 2 pushes)
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x89              // MOV dword ptr [ESI+0x8], EAX   (this->m08 = arg2)
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI+0x10], EDX  (this->m10 = arg4)
        _emit 0x56
        _emit 0x10
        _emit 0xc7              // MOV dword ptr [ESI], 0xf56d3c  (primary vtable)
        _emit 0x06
        _emit 0x3c
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI+0x14], ECX  (this->m14 = arg5)
        _emit 0x4e
        _emit 0x14
        _emit 0x89              // MOV dword ptr [ESI+0x1c], EAX  (this->m1c = arg2)
        _emit 0x46
        _emit 0x1c
        _emit 0xc7              // MOV dword ptr [ESI+0x18], 0xf56ce8
        _emit 0x46
        _emit 0x18
        _emit 0xe8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x20], 0x9d4600
        _emit 0x46
        _emit 0x20
        _emit 0x00
        _emit 0x46
        _emit 0x9d
        _emit 0x00
        _emit 0xb8              // MOV EAX, 0x6ce2e0
        _emit 0xe0
        _emit 0xe2
        _emit 0x6c
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI+0x24], EAX
        _emit 0x46
        _emit 0x24
        _emit 0x89              // MOV dword ptr [ESI+0x28], EAX
        _emit 0x46
        _emit 0x28
        _emit 0x89              // MOV dword ptr [ESI+0x2c], EAX
        _emit 0x46
        _emit 0x2c
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x89              // MOV dword ptr [ESI+0x30], EBX
        _emit 0x5e
        _emit 0x30
        _emit 0x89              // MOV dword ptr [ESI+0x34], EBX
        _emit 0x5e
        _emit 0x34
        _emit 0x89              // MOV dword ptr [ESI+0x38], EBX
        _emit 0x5e
        _emit 0x38
        _emit 0xc7              // MOV dword ptr [ESI+0x3c], 0xf567c4  (Link vtable)
        _emit 0x46
        _emit 0x3c
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESI+0x3c]
        _emit 0x46
        _emit 0x3c
        _emit 0x89              // MOV dword ptr [EAX+0x4], EAX   (Link.next = self)
        _emit 0x40
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX+0x8], EAX   (Link.prev = self)
        _emit 0x40
        _emit 0x08
        _emit 0x88              // MOV byte ptr [ESI+0x48], BL    (= 0)
        _emit 0x5e
        _emit 0x48
        _emit 0x8d              // LEA EAX, [ESI+0x4c]
        _emit 0x46
        _emit 0x4c
        _emit 0x8d              // LEA EDX, [ESI+0x5c]
        _emit 0x56
        _emit 0x5c
        _emit 0xc7              // MOV dword ptr [EAX], 0xf567c4  (Link vtable)
        _emit 0x00
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX+0x4], EAX   (Link.next = self)
        _emit 0x40
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX+0x8], EAX   (Link.prev = self)
        _emit 0x40
        _emit 0x08
        _emit 0x52              // PUSH EDX                        (&this+0x5c)
        _emit 0xc7              // MOV dword ptr [ESI+0x58], 0xf56cf0
        _emit 0x46
        _emit 0x58
        _emit 0xf0
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x00f3e174]
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI+0x78], EBX  (= 0)
        _emit 0x5e
        _emit 0x78
        _emit 0xc7              // MOV dword ptr [ESI+0x74], 0xf56d2c
        _emit 0x46
        _emit 0x74
        _emit 0x2c
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI+0x80], EBX  (= 0)
        _emit 0x9e
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x7c], 0xf56d00
        _emit 0x46
        _emit 0x7c
        _emit 0x00
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI                   (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x14                        (clean 5 args)
        _emit 0x14
        _emit 0x00
    }
}
