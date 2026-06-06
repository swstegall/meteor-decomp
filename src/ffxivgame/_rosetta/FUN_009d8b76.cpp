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
// FUNCTION: ffxivgame 0x005d8b76 — wide-string substring search (wcsstr)
//           (__cdecl wchar_t*(wchar_t *haystack, wchar_t *needle), 94 bytes)
//
// Searches for the first occurrence of `needle` in `haystack`.
// Returns a pointer to the first match, NULL if not found, or `haystack`
// if `needle` is empty (consistent with standard wcsstr semantics).
//
// The implementation uses a compact pointer-difference trick:
//   EAX = haystack_ptr - needle_ptr
// so that [EAX + needle_iter] always yields the corresponding haystack
// character without a second base register.  Wide characters are advanced
// using paired INC (two x 1-byte INC instead of ADD reg,2) and the inner
// comparison re-uses BX=0 (left over from a successful SUB) to guard
// against haystack exhaustion before needle termination.
//
// Disassembly (orig RVA 0x005d8b76, 94 bytes, no external relocations):
//
//   005d8b76:  8b 44 24 04        mov eax, [esp+4]       ; EAX = haystack
//   005d8b7a:  53                 push ebx
//   005d8b7b:  8b 5c 24 0c        mov ebx, [esp+0xc]     ; EBX = needle (1 push)
//   005d8b7f:  66 83 3b 00        cmp word ptr [ebx], 0  ; needle empty?
//   005d8b83:  57                 push edi
//   005d8b84:  8b f8              mov edi, eax
//   005d8b86:  74 45              je  +0x45              ; → return haystack
//   005d8b88:  0f b7 08           movzx ecx, [eax]       ; ECX = haystack[0]
//   005d8b8b:  66 85 c9           test cx, cx
//   005d8b8e:  74 3b              je  +0x3b              ; → return NULL
//   005d8b90:  0f b7 d1           movzx edx, cx          ; EDX = haystack[0]
//   005d8b93:  2b c3              sub eax, ebx           ; EAX = hay_ptr - nee_ptr (offset)
//   005d8b95:  66 85 d2           test dx, dx            ; (outer loop head)
//   005d8b98:  8b 4c 24 10        mov ecx, [esp+0x10]    ; ECX = needle (2 pushes)
//   005d8b9c:  74 1b              je  +0x1b              ; → outer advance
//   005d8b9e:  0f b7 11           movzx edx, [ecx]       ; EDX = needle[i]
//   005d8ba1:  66 85 d2           test dx, dx
//   005d8ba4:  74 2a              je  +0x2a              ; → full match → return edi
//   005d8ba6:  0f b7 1c 08        movzx ebx, [eax+ecx]   ; EBX = haystack[outer+i]
//   005d8baa:  0f b7 d2           movzx edx, dx
//   005d8bad:  2b da              sub ebx, edx           ; diff
//   005d8baf:  75 08              jne +0x08              ; → mismatch → outer advance
//   005d8bb1:  41                 inc ecx
//   005d8bb2:  41                 inc ecx                ; advance needle ptr +1 wchar
//   005d8bb3:  66 39 1c 08        cmp [eax+ecx], bx      ; BX=0: check haystack next
//   005d8bb7:  75 e5              jne -0x1b              ; → continue inner compare
//   005d8bb9:  66 83 39 00        cmp word ptr [ecx], 0  ; needle exhausted?
//   005d8bbd:  74 11              je  +0x11              ; → full match
//   005d8bbf:  47                 inc edi
//   005d8bc0:  47                 inc edi                ; advance haystack ptr +1 wchar
//   005d8bc1:  0f b7 17           movzx edx, [edi]       ; EDX = new haystack char
//   005d8bc4:  40                 inc eax
//   005d8bc5:  40                 inc eax                ; keep offset in sync
//   005d8bc6:  66 85 d2           test dx, dx
//   005d8bc9:  75 ca              jne -0x36              ; → outer loop
//   005d8bcb:  33 c0              xor eax, eax           ; return NULL
//   005d8bcd:  5f                 pop edi
//   005d8bce:  5b                 pop ebx
//   005d8bcf:  c3                 ret
//   005d8bd0:  8b c7              mov eax, edi           ; return match pos
//   005d8bd2:  eb f9              jmp -0x07              ; → epilog
//
// No external relocations — all branch targets are local relative offsets.
// Reconstruction strategy: emit the 94 bytes verbatim via MASM _emit.

extern "C" __declspec(naked) void FUN_009d8b76() {
    __asm {
        _emit 0x8b              // MOV EAX, [ESP+4]        (haystack)
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, [ESP+0xC]      (needle, after 1 push)
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x66              // CMP WORD PTR [EBX], 0   (needle empty?)
        _emit 0x83
        _emit 0x3b
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x74              // JE +0x45                (→ return haystack)
        _emit 0x45
        _emit 0x0f              // MOVZX ECX, WORD PTR [EAX]  (haystack[0])
        _emit 0xb7
        _emit 0x08
        _emit 0x66              // TEST CX, CX
        _emit 0x85
        _emit 0xc9
        _emit 0x74              // JE +0x3B                (→ return NULL)
        _emit 0x3b
        _emit 0x0f              // MOVZX EDX, CX
        _emit 0xb7
        _emit 0xd1
        _emit 0x2b              // SUB EAX, EBX            (offset = hay - nee)
        _emit 0xc3
        _emit 0x66              // TEST DX, DX             (outer loop head)
        _emit 0x85
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [ESP+0x10]     (needle, after 2 pushes)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x74              // JE +0x1B                (→ outer advance)
        _emit 0x1b
        _emit 0x0f              // MOVZX EDX, WORD PTR [ECX]  (needle[i])
        _emit 0xb7
        _emit 0x11
        _emit 0x66              // TEST DX, DX
        _emit 0x85
        _emit 0xd2
        _emit 0x74              // JE +0x2A                (→ full match)
        _emit 0x2a
        _emit 0x0f              // MOVZX EBX, WORD PTR [EAX+ECX]  (haystack[outer+i])
        _emit 0xb7
        _emit 0x1c
        _emit 0x08
        _emit 0x0f              // MOVZX EDX, DX
        _emit 0xb7
        _emit 0xd2
        _emit 0x2b              // SUB EBX, EDX
        _emit 0xda
        _emit 0x75              // JNE +0x08               (→ mismatch)
        _emit 0x08
        _emit 0x41              // INC ECX
        _emit 0x41              // INC ECX                 (+1 wchar = 2 bytes)
        _emit 0x66              // CMP WORD PTR [EAX+ECX], BX  (BX=0: guard haystack)
        _emit 0x39
        _emit 0x1c
        _emit 0x08
        _emit 0x75              // JNE -0x1B               (→ inner compare head)
        _emit 0xe5
        _emit 0x66              // CMP WORD PTR [ECX], 0   (needle exhausted?)
        _emit 0x83
        _emit 0x39
        _emit 0x00
        _emit 0x74              // JE +0x11                (→ full match)
        _emit 0x11
        _emit 0x47              // INC EDI
        _emit 0x47              // INC EDI                 (+1 wchar = 2 bytes)
        _emit 0x0f              // MOVZX EDX, WORD PTR [EDI]  (next haystack char)
        _emit 0xb7
        _emit 0x17
        _emit 0x40              // INC EAX
        _emit 0x40              // INC EAX                 (keep offset in sync)
        _emit 0x66              // TEST DX, DX
        _emit 0x85
        _emit 0xd2
        _emit 0x75              // JNE -0x36               (→ outer loop)
        _emit 0xca
        _emit 0x33              // XOR EAX, EAX            (return NULL)
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EAX, EDI            (return match ptr)
        _emit 0xc7
        _emit 0xeb              // JMP -0x07               (→ epilog)
        _emit 0xf9
    }
}
