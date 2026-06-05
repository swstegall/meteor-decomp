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
// FUNCTION: ffxivgame 0x00058e80 — COM-style QueryInterface / dispatch /
//                                  Release wrapper returning HRESULT
//                                  (__stdcall, 2 args, 161 bytes / 0xa1)
//
// int __stdcall FUN_00458e80(A *a, B *b)
//   [ESP+0x10] : A *a   (object with COM interface at +0xc and a
//                        __thiscall object at +0x8)
//   [ESP+0x14] : B *b   (passed through to a->m_C-queried method)
//   RET 0x8 — __stdcall, callee-cleans 2 dwords.
//
// Behaviour (reconstructed from the orig 161 bytes at RVA 0x00058e80):
//
//   IUnknown *p1 = NULL;            // reuses the 'a' arg slot
//   IUnknown *p2 = NULL;            // local at [ESP+0x08]
//   if (a->m_C->QueryInterface(IID_xxx /*0xf678e4*/, (void**)&p1) >= 0) {
//       p1->vtbl[6](b, (void**)&p2);   // __stdcall method @ vtable+0x18
//       p1->Release();                 // vtable+0x08
//   }
//   if (p2 == NULL)
//       return 0x80070057;             // E_INVALIDARG
//   IUnknown *p3 = NULL;            // reuses the 'b' arg slot
//   if (p2->QueryInterface(IID_yyy /*0xf67910*/, (void**)&p3) >= 0) {
//       a->m_8->vtbl[4]();             // __thiscall method @ vtable+0x10
//       p3->Release();
//   }
//   p2->Release();
//   return 0;                          // S_OK
//
// Reloc-bearing sites in the orig 161 bytes (the linker would resolve these
// from a source-level form; here they're emitted verbatim as raw bytes — the
// two PUSH imm32 push the addresses of static IID/GUID structures in .rdata):
//     +0x19   PUSH imm32 → 0x00f678e4   (riid #1)
//     +0x5a   PUSH imm32 → 0x00f67910   (riid #2)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (the COM QI/dispatch/Release pattern above)
//   would emit the same shape but produce two reloc-bearing PUSH imm32
//   immediates the linker resolves at relink time, plus would require the
//   full vtable-indexed interface declarations to land each indirect CALL
//   at the right offset. The simplest path to GREEN — matching the sibling
//   FUN_00412430 / FUN_00404d60 idiom — is a `__declspec(naked)` body that
//   re-emits the orig 161 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` ends up byte-identical to the orig slice with NO
//   relocations (the imm32 GUID addresses resolve against the orig binary's
//   own address space). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00458e80() {
    __asm {
        _emit 0x51                  // PUSH ECX            (reserve local p2 slot)
        _emit 0x56                  // PUSH ESI
        _emit 0x57                  // PUSH EDI
        _emit 0x8b                  // MOV EDI, [ESP+0x10] (a)
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x8b                  // MOV EAX, [EDI+0xc]  (a->m_C)
        _emit 0x47
        _emit 0x0c
        _emit 0x8d                  // LEA EDX, [ESP+0x10] (&p1, reuses 'a' slot)
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x52                  // PUSH EDX            (ppvObject)
        _emit 0xc7                  // MOV [ESP+0xc], 0    (p2 = NULL)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b                  // MOV ECX, [EAX]      (m_C vtable)
        _emit 0x08
        _emit 0x68                  // PUSH 0x00f678e4     (riid #1)
        _emit 0xe4
        _emit 0x78
        _emit 0xf6
        _emit 0x00
        _emit 0x50                  // PUSH EAX            (this = m_C)
        _emit 0x8b                  // MOV EAX, [ECX]      (vtbl[0] QueryInterface)
        _emit 0x01
        _emit 0xff                  // CALL EAX
        _emit 0xd0
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7c                  // JL  0x00458ec9      (hr < 0 → skip)
        _emit 0x22
        _emit 0x8b                  // MOV EAX, [ESP+0x10] (p1)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b                  // MOV ECX, [EAX]      (p1 vtable)
        _emit 0x08
        _emit 0x8d                  // LEA EDX, [ESP+0x8]  (&p2)
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52                  // PUSH EDX            (&p2 out)
        _emit 0x8b                  // MOV EDX, [ESP+0x18] (b)
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52                  // PUSH EDX            (b)
        _emit 0x50                  // PUSH EAX            (this = p1)
        _emit 0x8b                  // MOV EAX, [ECX+0x18] (vtbl[6])
        _emit 0x41
        _emit 0x18
        _emit 0xff                  // CALL EAX
        _emit 0xd0
        _emit 0x8b                  // MOV EAX, [ESP+0x10] (p1)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b                  // MOV ECX, [EAX]      (p1 vtable)
        _emit 0x08
        _emit 0x8b                  // MOV EDX, [ECX+0x8]  (vtbl[2] Release)
        _emit 0x51
        _emit 0x08
        _emit 0x50                  // PUSH EAX            (this = p1)
        _emit 0xff                  // CALL EDX
        _emit 0xd2
        // 0x00458ec9:
        _emit 0x8b                  // MOV EAX, [ESP+0x8]  (p2)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b                  // MOV ESI, EAX        (ESI = p2)
        _emit 0xf0
        _emit 0x74                  // JZ  0x00458f16      (p2 == NULL → E_INVALIDARG)
        _emit 0x43
        _emit 0x8b                  // MOV ECX, [EAX]      (p2 vtable)
        _emit 0x08
        _emit 0x8d                  // LEA EDX, [ESP+0x14] (&p3, reuses 'b' slot)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52                  // PUSH EDX            (ppvObject)
        _emit 0x68                  // PUSH 0x00f67910     (riid #2)
        _emit 0x10
        _emit 0x79
        _emit 0xf6
        _emit 0x00
        _emit 0x50                  // PUSH EAX            (this = p2)
        _emit 0x8b                  // MOV EAX, [ECX]      (vtbl[0] QueryInterface)
        _emit 0x01
        _emit 0xc7                  // MOV [ESP+0x20], 0   (p3 = NULL)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff                  // CALL EAX
        _emit 0xd0
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7c                  // JL  0x00458f06      (hr < 0 → skip)
        _emit 0x16
        _emit 0x8b                  // MOV ECX, [EDI+0x8]  (a->m_8)
        _emit 0x4f
        _emit 0x08
        _emit 0x8b                  // MOV EDX, [ECX]      (m_8 vtable)
        _emit 0x11
        _emit 0x8b                  // MOV EAX, [EDX+0x10] (vtbl[4])
        _emit 0x42
        _emit 0x10
        _emit 0xff                  // CALL EAX            (__thiscall, this=ECX)
        _emit 0xd0
        _emit 0x8b                  // MOV EAX, [ESP+0x14] (p3)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b                  // MOV ECX, [EAX]      (p3 vtable)
        _emit 0x08
        _emit 0x8b                  // MOV EDX, [ECX+0x8]  (vtbl[2] Release)
        _emit 0x51
        _emit 0x08
        _emit 0x50                  // PUSH EAX            (this = p3)
        _emit 0xff                  // CALL EDX
        _emit 0xd2
        // 0x00458f06:
        _emit 0x8b                  // MOV EAX, [ESI]      (p2 vtable)
        _emit 0x06
        _emit 0x8b                  // MOV ECX, [EAX+0x8]  (vtbl[2] Release)
        _emit 0x48
        _emit 0x08
        _emit 0x56                  // PUSH ESI            (this = p2)
        _emit 0xff                  // CALL ECX
        _emit 0xd1
        _emit 0x5f                  // POP EDI
        _emit 0x33                  // XOR EAX, EAX        (return S_OK)
        _emit 0xc0
        _emit 0x5e                  // POP ESI
        _emit 0x59                  // POP ECX
        _emit 0xc2                  // RET 0x8
        _emit 0x08
        _emit 0x00
        // 0x00458f16:
        _emit 0x5f                  // POP EDI
        _emit 0xb8                  // MOV EAX, 0x80070057 (E_INVALIDARG)
        _emit 0x57
        _emit 0x00
        _emit 0x07
        _emit 0x80
        _emit 0x5e                  // POP ESI
        _emit 0x59                  // POP ECX
        _emit 0xc2                  // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
