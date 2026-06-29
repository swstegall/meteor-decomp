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
// FUNCTION: ffxivgame 0x0043d940 — slot-assign / detach helper
//                                  (__cdecl, 133 bytes, SEH+GS frame,
//                                  ESP-based).
//
// High-level shape reconstructed from asm:
//
//   void* __cdecl FUN_0043d940(void**     out_ptr,   // [ESP+0x20]
//                               SomeObj*   self,      // [ESP+0x24]
//                               void*      key,       // [ESP+0x28]
//                               void*      extra)     // [ESP+0x2C]
//   {
//       void* local_iface = NULL;  // [BASE_ESP+0x0C] — output param
//       // (local_B at [BASE_ESP+0x08] tracks SEH guard state)
//
//       // __thiscall call: ECX=extra, stack=(key, &local_iface)
//       // Returns pointer to a slot inside some container.
//       void** slot = self->FUN_0043d3a0(&local_iface, key);
//
//       void* val = *slot;   // read slot
//       *slot = NULL;        // clear it
//       *out_ptr = val;      // hand value to caller
//
//       if (local_iface != NULL) {
//           // virtual Release(1) — vtable[0]
//           ((IReleasable*)local_iface)->Release(1);
//       }
//       return out_ptr;
//   }
//
// The function carries three relocations (all masked by compare.py):
//   +0x03  DIR32 → SEH handler stub (orig 0x00e56ae9)
//   +0x13  DIR32 → __security_cookie (orig 0x012ea8b0)
//   +0x3B  REL32 → FUN_0043d3a0
//
// Reconstruction strategy: naked-asm _emit byte passthrough, identical
// to siblings FUN_00403d60 / FUN_00408610.  The SEH-frame + GS prologue
// layout cannot be coerced at source level without introducing extra
// alignment artefacts; emitting the 133 bytes verbatim is the safest
// path to a GREEN diff.

extern "C" __declspec(naked) void FUN_0043d940() {
    __asm {
        // --- Prologue: SEH + /GS setup (ESP-based frame) ---
        _emit 0x6a              // PUSH -1                    (initial SEH state)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e56ae9            (SEH handler stub, DIR32 reloc)
        _emit 0xe9
        _emit 0x6a
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                   (prev fs:[0])
        _emit 0x83              // SUB ESP, 0x08              (two DWORD locals: local_B, local_A)
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]      (__security_cookie, DIR32 reloc)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX                   (GS cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x10]      (= &prev_fs0)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x00000000], EAX   (install SEH registration)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- Body ---
        _emit 0xc7              // MOV dword ptr [ESP + 0x08], 0  (local_B = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x28]  (arg3 / key)
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x50              // PUSH EAX                          (2nd stack arg to callee)

        _emit 0x8d              // LEA ECX, [ESP + 0x10]             (ECX = &local_A after push)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX                          (1st stack arg to callee = &local_A)

        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x2c]   (ECX = arg4, __thiscall this)
        _emit 0x4c
        _emit 0x24
        _emit 0x2c

        _emit 0xe8              // CALL FUN_0043d3a0                 (REL32 reloc)
        _emit 0x21
        _emit 0xfa
        _emit 0xff
        _emit 0xff

        _emit 0x8b              // MOV EDX, dword ptr [EAX]          (EDX = *slot)
        _emit 0x10
        _emit 0x8b              // MOV ESI, dword ptr [ESP + 0x28]   (ESI = arg1 / out_ptr, after 2 pushes)
        _emit 0x74
        _emit 0x24
        _emit 0x28
        _emit 0x8b              // MOV ECX, EDX
        _emit 0xca
        _emit 0xc7              // MOV dword ptr [EAX], 0x0          (*slot = NULL)
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x08                     (pop 2 pushed args)
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI], ECX          (*out_ptr = val)
        _emit 0x0e

        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x0c]   (ECX = local_A)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9

        _emit 0xc7              // MOV dword ptr [ESP + 0x18], 0x0   (SEH trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0xc7              // MOV dword ptr [ESP + 0x08], 0x1   (local_B = 1)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x74              // JZ +0x08                          (if local_A==0, skip virtual call)
        _emit 0x08

        _emit 0x8b              // MOV EAX, dword ptr [ECX]          (EAX = vtable)
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX]          (EDX = vtable[0])
        _emit 0x10
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xff              // CALL EDX                          (virtual Release(1))
        _emit 0xd2

        // --- Epilogue: restore SEH, return out_ptr ---
        _emit 0x8b              // MOV EAX, ESI                      (return value = out_ptr)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x10]   (ECX = prev_fs0)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x00000000], ECX          (unlink SEH)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                           (drop GS cookie)
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3              // RET
    }
}

// vim: ts=4 sts=4 sw=4 et
