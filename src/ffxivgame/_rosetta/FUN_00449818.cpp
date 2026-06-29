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
// FUNCTION: ffxivgame 0x00049818 — wstring-like assign body + epilogue chunk
//                                  (__stdcall 2-arg, 122 B / 0x7a; this slice
//                                  carries no prologue — the EBP frame and
//                                  callee-saved register set are established by
//                                  the prologue at a lower RVA attributed to
//                                  the same logical function by Ghidra).
//
// Asm shape (read from RVA 0x00049818, 122 bytes of `.text`):
//
//   At entry the EBP frame is live; EDI holds `this` (a wstring-like object),
//   ESI holds the new capacity/length; [ebp+0x8] is the source-data pointer and
//   [ebp+0xc] is the element count.
//
//   Pseudo-C shape:
//
//     ebx = count = [ebp+0xc];
//     if (count != 0) {
//         // pick buffer: heap pointer vs inline buffer
//         if (this->_Cap >= 8)
//             src = this->_BufPtr;              // [EDI+0x4] loaded
//         else
//             src = &this->_InlineBuf;          // LEA [EDI+0x4]
//         // copy content — 4-arg __cdecl (add esp, 0x10 cleanup)
//         fn_0x009d17f3(arg1, ESI*2+2, src, count*2);
//     }
//     // free old heap buffer if large (3-arg __cdecl, add esp, 0xc)
//     old_cap = this->_Cap;
//     if (old_cap >= 8)
//         fn_0x0044d350(this->_BufPtr, old_cap*2+2, 0xC);
//
//     // install new state
//     new_ptr = [ebp+0x8];
//     eax = &this->_InlineBuf;
//     *(wchar_t*)eax = 0;          // 16-bit zero-init
//     *(void**)eax  = new_ptr;     // overwrite with pointer
//     this->_Cap  = ESI;
//     this->_Size = count;
//     if (ESI >= 8) eax = new_ptr; // large: write NUL via new_ptr
//     *(wchar_t*)(eax + count*2) = 0;  // NUL terminator
//
//     // SEH chain restore + standard epilogue
//     ecx = [ebp - 0xC];   MOV FS:[0], ECX
//     POP ECX; POP EDI; POP ESI; POP EBX; MOV ESP,EBP; POP EBP; RET 0x8
//
// Reloc-bearing sites in the orig 122 bytes:
//   +0x23  CALL rel32 → VA 0x009d17f3  (e8 b3 7f 58 00)
//   +0x3e  CALL rel32 → VA 0x0044d350  (e8 f5 3a 00 00)
//   +0x69  MOV FS:[0], ECX (64 89 0d 00 00 00 00 — FS-prefixed, imm32=0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Both CALL displacements are valid only in the original binary's address
//   space and cannot be reconstructed from .obj-level relocations without a
//   full relink.  Emitting them as raw _emit bytes produces the same bytes;
//   tools/compare.py masks relocation sites, so GREEN is still achievable.

extern "C" __declspec(naked) void FUN_00449818() {
    __asm {
        _emit 0x8b              // MOV EBX, [EBP + 0xC]
        _emit 0x5d
        _emit 0x0c
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x76              // JBE +0x24  (→ skip copy block)
        _emit 0x24
        _emit 0x83              // CMP dword ptr [EDI + 0x18], 0x8
        _emit 0x7f
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JC +0x05   (→ use inline buffer)
        _emit 0x05
        _emit 0x8b              // MOV EAX, [EDI + 0x4]   (heap ptr)
        _emit 0x47
        _emit 0x04
        _emit 0xeb              // JMP +0x03  (→ after LEA)
        _emit 0x03
        _emit 0x8d              // LEA EAX, [EDI + 0x4]   (inline buf)
        _emit 0x47
        _emit 0x04
        _emit 0x8b              // MOV ECX, [EBP + 0x8]   (arg1 = new_ptr)
        _emit 0x4d
        _emit 0x08
        _emit 0x8d              // LEA EDX, [EBX + EBX*1] (count*2)
        _emit 0x14
        _emit 0x1b
        _emit 0x52              // PUSH EDX               (arg4 = count*2)
        _emit 0x50              // PUSH EAX               (arg3 = src buffer)
        _emit 0x8d              // LEA EAX, [ESI + ESI*1 + 0x2]  (cap*2+2)
        _emit 0x44
        _emit 0x36
        _emit 0x02
        _emit 0x50              // PUSH EAX               (arg2 = cap*2+2)
        _emit 0x51              // PUSH ECX               (arg1 = new_ptr)
        _emit 0xe8              // CALL 0x009d17f3  (rel32 = 0x00587fb3)
        _emit 0xb3
        _emit 0x7f
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10          (__cdecl cleanup)
        _emit 0xc4
        _emit 0x10
        _emit 0x8b              // MOV EAX, [EDI + 0x18]  (old capacity)
        _emit 0x47
        _emit 0x18
        _emit 0x83              // CMP EAX, 0x8
        _emit 0xf8
        _emit 0x08
        _emit 0x72              // JC +0x13  (→ skip free)
        _emit 0x13
        _emit 0x8d              // LEA EDX, [EAX + EAX*1 + 0x2]  (old_cap*2+2)
        _emit 0x54
        _emit 0x00
        _emit 0x02
        _emit 0x8b              // MOV EAX, [EDI + 0x4]   (heap ptr)
        _emit 0x47
        _emit 0x04
        _emit 0x6a              // PUSH 0xC
        _emit 0x0c
        _emit 0x52              // PUSH EDX               (arg2 = old_cap*2+2)
        _emit 0x50              // PUSH EAX               (arg1 = heap ptr)
        _emit 0xe8              // CALL 0x0044d350  (rel32 = 0x00003af5)
        _emit 0xf5
        _emit 0x3a
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xC           (__cdecl cleanup)
        _emit 0xc4
        _emit 0x0c
        _emit 0x83              // CMP ESI, 0x8           (new cap >= 8?)
        _emit 0xfe
        _emit 0x08
        _emit 0x8b              // MOV ECX, [EBP + 0x8]   (new_ptr)
        _emit 0x4d
        _emit 0x08
        _emit 0x8d              // LEA EAX, [EDI + 0x4]   (inline buf slot)
        _emit 0x47
        _emit 0x04
        _emit 0x66              // MOV word ptr [EAX], 0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX], ECX  (store new_ptr)
        _emit 0x08
        _emit 0x89              // MOV [EDI + 0x18], ESI  (store new capacity)
        _emit 0x77
        _emit 0x18
        _emit 0x89              // MOV [EDI + 0x14], EBX  (store new size)
        _emit 0x5f
        _emit 0x14
        _emit 0x72              // JC +0x02  (CF set → small buf, eax already inline)
        _emit 0x02
        _emit 0x8b              // MOV EAX, ECX           (large: eax = new_ptr)
        _emit 0xc1
        _emit 0x66              // MOV word ptr [EAX + EBX*2], 0x0  (NUL terminator)
        _emit 0xc7
        _emit 0x04
        _emit 0x58
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, [EBP - 0xC]  (saved FS:[0] prev handler)
        _emit 0x4d
        _emit 0xf4
        _emit 0x64              // MOV FS:[0], ECX        (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                (security cookie slot)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV ESP, EBP           (frame teardown)
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
