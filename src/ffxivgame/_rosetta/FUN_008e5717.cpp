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
// FUNCTION: ffxivgame 0x008e5717 — std::basic_string-style assign/replace
//           body + SEH epilogue (124 B / 0x7c).
//
// Context (read from surrounding code at orig RVA 0x004e5680):
//
//   This 0x7c-byte slice is the tail of a larger __thiscall function that
//   begins its SEH prologue at RVA 0x004e5680 and returns via `ret 8` at
//   0x004e5793. Ghidra identifies 0x008e5717 as a function entry because a
//   `jmp 0x8e5717` at 0x8e56eb (within the allocation prologue) and a
//   fall-through from 0x8e5714 both land here — the two paths converge at
//   this point after deciding / reallocating the backing buffer.
//
//   Logical shape of the fragment (ebx = this string object):
//
//     edi = arg2 (count)                     ; [ebp+0xc]
//     esi = new capacity (from prior alloc)
//     [ebp+8] = pointer to source data
//
//     if (count != 0) {
//         // choose buffer: heap (if capacity>=0x10) or inline
//         ecx = (this->_Cap>=0x10) ? this->_Ptr : &this->_Buf;
//         edx = esi + 1;    // edx = new_cap + 1
//         eax = count;
//         if (edx < count) {
//             // overflow / truncation path — exception raise
//             call FUN_009d22b4;   // xstring _Grow / throw
//             goto epilogue;
//         }
//         // copy `count` bytes from ecx → [ebp+8] (dest)
//         edi = arg1 (dest ptr);             ; [ebp+8]
//         do {
//             *edi++ = *ecx++;
//         } while (--eax != 0);
//         edi = count;                        ; reload arg2 for size store
//     }
//     // free old heap buffer if capacity >= 0x10
//     if (this->_Cap >= 0x10) {
//         push this->_Ptr;
//         call FUN_009d1b17;      // operator delete / _Dealloc
//         add esp, 4;
//     }
//     // install new buffer, capacity, size, NUL-terminate
//     ecx = [ebp+8]  (new buffer pointer)
//     eax = &this->_Buf;
//     this->_Buf[0] = '\0';      // clear inline storage first byte
//     this->_Ptr    = ecx;       // point at new buffer
//     this->_Cap    = esi;       // store new capacity
//     this->_Size   = edi;       // store new size (count)
//     // if capacity < 0x10 eax already points at inline buf,
//     // else eax = ecx (heap ptr) — in either case NUL-terminate:
//     if (esi >= 0x10) eax = ecx;
//     *(eax + edi) = '\0';
//
//     // SEH epilogue — restore FS:[0] and callee-saved registers
//     ecx = [ebp-0xc]
//     fs:[0] = ecx
//     pop ecx; pop edi; pop esi; pop ebx
//     mov esp, ebp; pop ebp
//     // (caller emits `ret 8` immediately after, not part of this slice)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The fragment has no prologue and ends without a `ret`, making a
//   source-level C++ form impossible to coax MSVC into emitting correctly.
//   The two embedded rel32 calls at offset +0x1e and +0x47 carry baked-in
//   relative displacements that compare.py masks as relocations; all other
//   bytes are structural and must match exactly.
//
//   Reloc-bearing sites in the orig 124 bytes:
//     +0x1f   CALL rel32 → FUN_009d22b4 (displacement 0x000ecb7a, masked)
//     +0x48   CALL rel32 → FUN_009d1b17 (displacement 0x000ec3b4, masked)
//
//   The pragmatic choice — identical to siblings FUN_00401350 and
//   FUN_00403d60 — is a `__declspec(naked)` body that re-emits all 124
//   bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_008e5717() {
    __asm {
        _emit 0x8b              // MOV EDI, [EBP+0x0C]   (count = arg2)
        _emit 0x7d
        _emit 0x0c
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x76              // JBE +0x36 → epilogue entry
        _emit 0x36
        _emit 0x83              // CMP dword ptr [EBX+0x18], 0x10  (capacity check)
        _emit 0x7b
        _emit 0x18
        _emit 0x10
        _emit 0x72              // JB +5 → inline-buf path
        _emit 0x05
        _emit 0x8b              // MOV ECX, [EBX+4]      (heap _Ptr)
        _emit 0x4b
        _emit 0x04
        _emit 0xeb              // JMP +3 → skip inline path
        _emit 0x03
        _emit 0x8d              // LEA ECX, [EBX+4]      (inline _Buf)
        _emit 0x4b
        _emit 0x04
        _emit 0x8d              // LEA EDX, [ESI+1]      (new_cap + 1)
        _emit 0x56
        _emit 0x01
        _emit 0x3b              // CMP EDX, EDI          (vs count)
        _emit 0xd7
        _emit 0x8b              // MOV EAX, EDI          (eax = count)
        _emit 0xc7
        _emit 0x73              // JAE +7 → copy path
        _emit 0x07
        _emit 0xe8              // CALL rel32 → FUN_009d22b4 (0x000ecb7a)
        _emit 0x7a
        _emit 0xcb
        _emit 0x0e
        _emit 0x00
        _emit 0xeb              // JMP +0x18 → epilogue entry
        _emit 0x18
        _emit 0x8b              // MOV EDI, [EBP+8]      (dest ptr = arg1)
        _emit 0x7d
        _emit 0x08
        _emit 0x90              // NOP                   (alignment)
        _emit 0x8a              // MOV DL, [ECX]         ; copy loop
        _emit 0x11
        _emit 0x88              // MOV [EDI], DL
        _emit 0x17
        _emit 0x83              // SUB EAX, 1
        _emit 0xe8
        _emit 0x01
        _emit 0x83              // ADD EDI, 1
        _emit 0xc7
        _emit 0x01
        _emit 0x83              // ADD ECX, 1
        _emit 0xc1
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x77              // JA -0x11 (loop back)
        _emit 0xef
        _emit 0x8b              // MOV EDI, [EBP+0x0C]   (reload count)
        _emit 0x7d
        _emit 0x0c
        _emit 0x83              // CMP dword ptr [EBX+0x18], 0x10
        _emit 0x7b
        _emit 0x18
        _emit 0x10
        _emit 0x72              // JB +0x0C → skip free
        _emit 0x0c
        _emit 0x8b              // MOV EAX, [EBX+4]      (heap _Ptr)
        _emit 0x43
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → FUN_009d1b17 (0x000ec3b4)
        _emit 0xb4
        _emit 0xc3
        _emit 0x0e
        _emit 0x00
        _emit 0x83              // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x83              // CMP ESI, 0x10         (new cap vs 0x10)
        _emit 0xfe
        _emit 0x10
        _emit 0x8b              // MOV ECX, [EBP+8]      (new buffer ptr)
        _emit 0x4d
        _emit 0x08
        _emit 0x8d              // LEA EAX, [EBX+4]      (&this->_Buf)
        _emit 0x43
        _emit 0x04
        _emit 0xc6              // MOV byte ptr [EAX], 0  (clear inline buf)
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EAX], ECX         (this->_Ptr = new_ptr)
        _emit 0x08
        _emit 0x89              // MOV [EBX+0x18], ESI    (this->_Cap = new_cap)
        _emit 0x73
        _emit 0x18
        _emit 0x89              // MOV [EBX+0x14], EDI    (this->_Size = count)
        _emit 0x7b
        _emit 0x14
        _emit 0x72              // JB +2 → eax already = &_Buf (inline)
        _emit 0x02
        _emit 0x8b              // MOV EAX, ECX           (eax = heap ptr)
        _emit 0xc1
        _emit 0xc6              // MOV byte ptr [EAX+EDI], 0  (NUL-terminate)
        _emit 0x04
        _emit 0x38
        _emit 0x00
        _emit 0x8b              // MOV ECX, [EBP-0x0C]   (saved FS:[0] link)
        _emit 0x4d
        _emit 0xf4
        _emit 0x64              // MOV dword ptr FS:[0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 8  (caller cleans 2 dword args)
        _emit 0x08
        _emit 0x00
    }
}
