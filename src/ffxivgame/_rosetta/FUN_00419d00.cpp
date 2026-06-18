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
// FUNCTION: ffxivgame 0x00419d00 — __thiscall loop: call vfunc0(0) on each
//           element of a 0x70-byte-stride array; if element[+0x60] != NULL
//           call vtable[10] on this->other; then free the array. (84 B)
//
// Calling convention: __thiscall (ECX = this).
//
// Frame layout (no frame pointer — /Oy):
//   saved EBX (counter)
//   saved ESI (this)
//   saved EDI (byte offset)  — pushed only inside the branch taken
//
// this layout (read from asm):
//   [this+0x24]  — pointer to array of 0x70-byte objects
//   [this+0x28]  — element count (unsigned)
//   [this+0x2c]  — pointer to secondary object (dispatched via vtable[10])
//
// Array element layout:
//   [elem+0x00]  — vtable pointer  (virtual dispatch: vtable[0] called with arg=0)
//   [elem+0x60]  — nullable pointer used as arg to this->other->vtable[10]
//
// Post-loop:
//   The array pointer itself is reloaded from [old_this+0x24].  If non-null,
//   the DWORD at [ptr - 4] is loaded into ECX (a header word before the
//   allocation) and passed as part of the free call to FUN_0040df70.
//
// Reconstruction strategy:
//   The function body is pure register-indirect calls (CALL EAX) and one
//   CALL rel32 to FUN_0040df70.  The naked __asm form with mnemonics
//   reproduces the exact byte sequence; compare.py masks only the single
//   rel32 relocation window.
//
//   Note the 2-byte MOV EDI,EDI (8b ff) immediately before the loop top —
//   a classic MSVC hot-patch NOP pair inserted after the XOR EDI,EDI that
//   initialises the byte-offset counter.

extern "C" {

// Direct call target (CALL rel32 — one relocation masked by compare.py).
void FUN_0040df70();

__declspec(naked) void FUN_00419d00() {
    __asm {
        push    ebx
        push    esi
        mov     esi, ecx                        // ESI = this
        xor     ebx, ebx                        // EBX = 0 (element counter)
        cmp     dword ptr [esi + 0x28], ebx     // count == 0?
        jbe     post_loop                        // skip loop if count <= 0
        push    edi
        xor     edi, edi                        // EDI = 0 (byte offset into array)
        mov     edi, edi                        // 2-byte hot-patch NOP (8b ff)
    loop_top:
        mov     eax, dword ptr [esi + 0x24]     // EAX = array base
        mov     edx, dword ptr [eax + edi]      // EDX = array[edi] = element vtable ptr
        lea     ecx, [eax + edi]                // ECX = &array[edi] = 'this' for vfunc0
        mov     eax, dword ptr [edx]            // EAX = vtable[0]
        push    0                               // arg = 0
        call    eax                             // element->vfunc0(0)
        mov     ecx, dword ptr [esi + 0x24]     // reload array base
        mov     eax, dword ptr [ecx + edi + 0x60] // EAX = element->field60
        test    eax, eax
        jz      loop_inc                        // skip if field60 == NULL
        mov     ecx, dword ptr [esi + 0x2c]     // ECX = this->other
        mov     edx, dword ptr [ecx]            // EDX = other->vtable
        push    eax                             // arg = field60
        mov     eax, dword ptr [edx + 0x28]     // EAX = vtable[10]
        call    eax                             // other->vfunc10(field60)
    loop_inc:
        add     ebx, 1
        add     edi, 0x70                       // advance to next element (stride 0x70)
        cmp     ebx, dword ptr [esi + 0x28]
        jc      loop_top
        pop     edi
    post_loop:
        mov     esi, dword ptr [esi + 0x24]     // ESI = array ptr
        test    esi, esi
        jz      done
        mov     ecx, dword ptr [esi - 4]        // ECX = header word before allocation
        push    esi
        call    FUN_0040df70
    done:
        pop     esi
        pop     ebx
        ret
    }
}

}  // extern "C"
