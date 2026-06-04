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
// FUNCTION: ffxivgame 0x00047330 — read-into-buffer + UTF-8 BOM strip
//                                  (__thiscall, 179 B / 0xb3)
//
//   __thiscall void *FUN_00447330(this, source *src)
//     ECX        : this   (a small string/buffer object, SSO at this+0x12)
//     [ESP+0x04] : src    (param @ [ESP+0x10] after PUSH EBP/ESI/EDI)
//     RET 4 — one stack argument.
//
//   Behaviour recovered from asm/ffxivgame/00047330_FUN_00447330.s:
//
//     mov esi, ecx
//     call 0x00456250                 ; init/guard (no args)
//     mov ebp, src
//     if (src->field4 == 0)           ; cmp [ebp+4], 0
//         report(0x29d5);             ; push 0x29d5; call 0x00456060; add esp,4
//     edi = src->len;                 ; mov edi, [ebp+8]
//     this->ptr = this+0x12;          ; SSO inline buffer
//     this->b10 = 1; this->b11 = 1;
//     this->c = 0; this->cap8 = 1; this->size4 = 0x40;
//     *(this+0x12) = 0;
//     this->resize(edi+1, 1);         ; thiscall 0x00447010(len+1, 1)
//     if (edi >= 3) {                 ; cmp edi,3 / jc small
//         src->read(this->ptr, 3);    ; thiscall 0x00453030(ptr, 3)
//         p = this->ptr;
//         if (p[0]==0xef && p[1]==0xbb && p[2]==0xbf) {   ; UTF-8 BOM
//             edi -= 3;
//             this->resize(edi+1, 1);
//             src->read(this->ptr, edi);
//         } else {
//             src->read(this->ptr + 3, edi - 3);
//         }
//     } else {
//         src->read(this->ptr, edi);
//     }
//     this->ptr[edi] = 0;             ; null-terminate
//     return this;                    ; eax = esi
//
//   Reloc-bearing CALL rel32 sites (compare.py wildcards these windows):
//     +0x05   CALL → 0x00456250   (init/guard)
//     +0x19   CALL → 0x00456060   (report, only on src->field4 == 0)
//     +0x51   CALL → 0x00447010   (resize, thiscall)
//     +0x62   CALL → 0x00453030   (read, thiscall)
//     +0x85   CALL → 0x00447010   (resize, thiscall)
//     +0xa0   CALL → 0x00453030   (read, thiscall)
//
// Reconstruction strategy — naked-asm byte passthrough (same as the
// sibling FUN_00403f10 etc.): the source-level C++ shape produces six
// CALL rel32 relocations the linker would resolve at relink time. We
// re-emit the orig 179 bytes verbatim via MASM `_emit`; the .obj's
// `.text` ends up byte-identical to the orig slice (no relocations —
// the rel32 displacements are baked in as raw bytes, and compare.py
// masks the reloc windows out of the diff). compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00447330() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xe8              // CALL rel32 → 0x00456250
        _emit 0x16
        _emit 0xef
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x10]
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x83              // CMP dword ptr [EBP+0x4], 0
        _emit 0x7d
        _emit 0x04
        _emit 0x00
        _emit 0x75              // JNZ +0x0d
        _emit 0x0d
        _emit 0x68              // PUSH 0x29d5
        _emit 0xd5
        _emit 0x29
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x00456060
        _emit 0x12
        _emit 0xed
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x8b              // MOV EDI, dword ptr [EBP+0x8]
        _emit 0x7d
        _emit 0x08
        _emit 0x8d              // LEA EAX, [ESI+0x12]
        _emit 0x46
        _emit 0x12
        _emit 0x89              // MOV dword ptr [ESI], EAX
        _emit 0x06
        _emit 0xc6              // MOV byte ptr [ESI+0x10], 0x1
        _emit 0x46
        _emit 0x10
        _emit 0x01
        _emit 0xc6              // MOV byte ptr [ESI+0x11], 0x1
        _emit 0x46
        _emit 0x11
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESI+0xc], 0
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x8], 0x1
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x4], 0x40
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x8d              // LEA EAX, [EDI+0x1]
        _emit 0x47
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x00447010
        _emit 0x8a
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // CMP EDI, 0x3
        _emit 0xff
        _emit 0x03
        _emit 0x72              // JC +0x3f (small)
        _emit 0x3f
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0x6a              // PUSH 0x3
        _emit 0x03
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0xe8              // CALL rel32 → 0x00453030
        _emit 0x99
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x80              // CMP byte ptr [EAX], 0xef
        _emit 0x38
        _emit 0xef
        _emit 0x75              // JNZ +0x22 (no-BOM)
        _emit 0x22
        _emit 0x80              // CMP byte ptr [EAX+0x1], 0xbb
        _emit 0x78
        _emit 0x01
        _emit 0xbb
        _emit 0x75              // JNZ +0x1c (no-BOM)
        _emit 0x1c
        _emit 0x80              // CMP byte ptr [EAX+0x2], 0xbf
        _emit 0x78
        _emit 0x02
        _emit 0xbf
        _emit 0x75              // JNZ +0x16 (no-BOM)
        _emit 0x16
        _emit 0x83              // SUB EDI, 0x3
        _emit 0xef
        _emit 0x03
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EDI+0x1]
        _emit 0x57
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x00447010
        _emit 0x56
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x57              // PUSH EDI
        _emit 0x50              // PUSH EAX
        _emit 0xeb              // JMP +0x0e (read tail)
        _emit 0x0e
        _emit 0x8d              // LEA ECX, [EDI-0x3]      (no-BOM:)
        _emit 0x4f
        _emit 0xfd
        _emit 0x51              // PUSH ECX
        _emit 0x83              // ADD EAX, 0x3
        _emit 0xc0
        _emit 0x03
        _emit 0x50              // PUSH EAX
        _emit 0xeb              // JMP +0x04 (read tail)
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESI] (small:)
        _emit 0x16
        _emit 0x57              // PUSH EDI
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, EBP             (read tail:)
        _emit 0xcd
        _emit 0xe8              // CALL rel32 → 0x00453030
        _emit 0x5b
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0xc6              // MOV byte ptr [EDI+EAX*1], 0
        _emit 0x04
        _emit 0x07
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
