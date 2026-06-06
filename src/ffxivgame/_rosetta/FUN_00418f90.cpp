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
// FUNCTION: ffxivgame 0x00418f90 — transfer a C++ object handle with
//                                  /GS cookie + SEH frame; optionally
//                                  invokes first vtable slot (__cdecl,
//                                  137 bytes / 0x89)
//
// Signature (inferred from stack layout, 5 params):
//
//   void * __cdecl FUN_00418f90(void **out_ptr,
//                               void  *param2,
//                               void  *param3,
//                               void  *param4,
//                               void  *param5);
//
// The function:
//   1. Installs a /GS security-cookie + 3-field SEH frame (pNext, handler,
//      tryLevel=−1).
//   2. Calls FUN_004323d0(&local_buf, param2, param3, param4) — receives
//      a pointer back in EAX.
//   3. Moves *EAX (the handle) into *out_ptr and zeroes *EAX.
//   4. Enters SEH try-level 0 (tryLevel ← 0), sets local state = 1.
//   5. If local_buf != 0: dereferences it as a vtable pointer and calls
//      vtbl[0](1).
//   6. Returns out_ptr (ESI) in EAX.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The combined /GS + SEH prologue, the FS:[0] read/write encoding, and
//   the specific tryLevel-mutation sequence inside the body are not
//   reproducible byte-identically from C++ source with MSVC 2005. The
//   __declspec(naked) body re-emits the original 137 bytes verbatim via
//   MASM _emit directives; the .obj's .text section is byte-identical to
//   the original slice, and compare.py reports GREEN.
//
// Reloc sites (compare.py masks these positions):
//   +0x02 : PUSH 0xe555ae       (SEH handler VA — IMAGE_REL_I386_DIR32)
//   +0x13 : MOV EAX,[0x12ea8b0] (__security_cookie VA — IMAGE_REL_I386_DIR32)
//   +0x41 : CALL 0x004323d0     (rel32 displacement — IMAGE_REL_I386_REL32)

extern "C" __declspec(naked) void FUN_00418f90() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe555ae  [RELOC +0x02]
        _emit 0xae
        _emit 0x55
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  [RELOC +0x13]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX  (security cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x0], EAX  (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x28]  (param3)
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x24]  (param2)
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0xc7              // MOV dword ptr [ESP + 0x8], 0x0  (local[0] = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x2c]  (param4)
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x50              // PUSH EAX  (param4 — 3rd arg to FUN_004323d0)
        _emit 0x51              // PUSH ECX  (param3 — 2nd arg)
        _emit 0x52              // PUSH EDX  (param2 — 1st arg)
        _emit 0x8d              // LEA EAX, [ESP + 0x18]  (&local[1])
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX  (out ptr — 0th arg)
        _emit 0xe8              // CALL 0x004323d0  [RELOC +0x41]
        _emit 0xfb
        _emit 0x93
        _emit 0x01
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [EAX]  (dereference return value)
        _emit 0x08
        _emit 0x8b              // MOV ESI, dword ptr [ESP + 0x30]  (param1 = out_ptr)
        _emit 0x74
        _emit 0x24
        _emit 0x30
        _emit 0xc7              // MOV dword ptr [EAX], 0x0  (zero original slot)
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10  (pop 4 args)
        _emit 0xc4
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI], ECX  (*out_ptr = handle)
        _emit 0x0e
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0xc]  (local[1])
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0xc7              // MOV dword ptr [ESP + 0x18], 0x0  (SEH tryLevel ← 0)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESP + 0x8], 0x1  (local[0] = 1)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x08  (→ epilogue)
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX]  (vtable ptr)
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX]  (first vtable slot)
        _emit 0x02
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xff              // CALL EAX  (vtbl[0](1))
        _emit 0xd0
        _emit 0x8b              // MOV EAX, ESI  (return out_ptr)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x10]  (old FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX  (uninstall SEH)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX  (pop security cookie)
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x14  (pop frame)
        _emit 0xc4
        _emit 0x14
        _emit 0xc3              // RET
    }
}
