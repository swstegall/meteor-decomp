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
// FUNCTION: ffxivgame 0x0005a510 — SSO string data-pointer extractor
//                                   forwarding to FUN_0045a480 (69 B / 0x45)
//
// Signature: __cdecl void* FUN_0045a510(void* arg1, SSO_string* str)
//   arg1 — destination / context object (returned unchanged)
//   str  — SSO basic_string (layout matches FUN_00403d10):
//            +0x04  union { char inline_buf[16]; char *heap_ptr; }
//            +0x14  size_t size
//            +0x18  size_t capacity  (0xf = inline, >=0x10 = heap)
//
// Body:
//   ECX = str->size  ([str+0x14])
//   if (str->capacity >= 0x10)            // heap-allocated
//       return FUN_0045a480(arg1, str->heap_ptr, size);   // [str+4]
//   else                                  // inline buffer
//       return FUN_0045a480(arg1, &str->inline_buf, size); // str+4
//
// Frame notes:
//   PUSH ECX at entry is MSVC /O2's "sub esp,4" substitution — allocates a
//   4-byte local slot on the stack.  The slot is zeroed by
//   MOV dword ptr [ESP], 0x0 immediately before the branch and popped
//   via POP ECX in both epilogues.  No frame pointer (/Oy).
//
// Asm (69 bytes @ orig RVA 0x0005a510):
//   51                       PUSH ECX                   ; allocate local slot
//   8b 44 24 0c              MOV EAX,[ESP+0xc]          ; arg2 (str)
//   83 78 18 10              CMP [EAX+0x18],0x10        ; capacity vs 16
//   8b 48 14                 MOV ECX,[EAX+0x14]         ; ECX = size
//   c7 04 24 00 00 00 00     MOV dword ptr [ESP],0x0    ; local = 0
//   72 18                    JB  inline_path
//   ; heap path:
//   8b 40 04                 MOV EAX,[EAX+0x4]          ; heap_ptr
//   56                       PUSH ESI
//   8b 74 24 0c              MOV ESI,[ESP+0xc]          ; arg1
//   51                       PUSH ECX                   ; push size
//   50                       PUSH EAX                   ; push heap_ptr
//   56                       PUSH ESI                   ; push arg1
//   e8 4b ff ff ff           CALL FUN_0045a480          ; (reloc)
//   83 c4 0c                 ADD ESP,0xc
//   8b c6                    MOV EAX,ESI                ; return arg1
//   5e                       POP ESI
//   59                       POP ECX                    ; pop local slot
//   c3                       RET
//   ; inline path:
//   56                       PUSH ESI
//   8b 74 24 0c              MOV ESI,[ESP+0xc]          ; arg1
//   51                       PUSH ECX                   ; push size
//   83 c0 04                 ADD EAX,0x4                ; &inline_buf = str+4
//   50                       PUSH EAX                   ; push &inline_buf
//   56                       PUSH ESI                   ; push arg1
//   e8 33 ff ff ff           CALL FUN_0045a480          ; (reloc)
//   83 c4 0c                 ADD ESP,0xc
//   8b c6                    MOV EAX,ESI                ; return arg1
//   5e                       POP ESI
//   59                       POP ECX                    ; pop local slot
//   c3                       RET

extern "C" void FUN_0045a480();

extern "C" __declspec(naked) void * __cdecl FUN_0045a510(void *, void *) {
    __asm {
        // 0005a510: 51
        push    ecx
        // 0005a511: 8b 44 24 0c
        mov     eax, dword ptr [esp+0xc]
        // 0005a515: 83 78 18 10
        cmp     dword ptr [eax+0x18], 0x10
        // 0005a519: 8b 48 14
        mov     ecx, dword ptr [eax+0x14]
        // 0005a51c: c7 04 24 00 00 00 00
        mov     dword ptr [esp], 0
        // 0005a523: 72 18
        jb      inline_path
        // 0005a525: 8b 40 04
        mov     eax, dword ptr [eax+0x4]
        // 0005a528: 56
        push    esi
        // 0005a529: 8b 74 24 0c
        mov     esi, dword ptr [esp+0xc]
        // 0005a52d: 51
        push    ecx
        // 0005a52e: 50
        push    eax
        // 0005a52f: 56
        push    esi
        // 0005a530: e8 RR RR RR RR  (reloc)
        call    FUN_0045a480
        // 0005a535: 83 c4 0c
        add     esp, 0xc
        // 0005a538: 8b c6
        mov     eax, esi
        // 0005a53a: 5e
        pop     esi
        // 0005a53b: 59
        pop     ecx
        // 0005a53c: c3
        ret
    inline_path:
        // 0005a53d: 56
        push    esi
        // 0005a53e: 8b 74 24 0c
        mov     esi, dword ptr [esp+0xc]
        // 0005a542: 51
        push    ecx
        // 0005a543: 83 c0 04
        add     eax, 4
        // 0005a546: 50
        push    eax
        // 0005a547: 56
        push    esi
        // 0005a548: e8 RR RR RR RR  (reloc)
        call    FUN_0045a480
        // 0005a54d: 83 c4 0c
        add     esp, 0xc
        // 0005a550: 8b c6
        mov     eax, esi
        // 0005a552: 5e
        pop     esi
        // 0005a553: 59
        pop     ecx
        // 0005a554: c3
        ret
    }
}
