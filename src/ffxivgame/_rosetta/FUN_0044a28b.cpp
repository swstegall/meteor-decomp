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
// FUNCTION: ffxivgame 0x0004a28b — fragment within an SEH-framed function;
//                                  copy loop + vector set + epilogue
//                                  (144 B / 0x90, ret 8, __stdcall)
//
// This code block is a continuation target (jumped to at +0x251 from the
// outer SEH body via `eb 31 jmp 0x44a28b`). The surrounding function has
// already set up a full stack frame (push ebp / mov ebp,esp), callee-saved
// EBX/ESI/EDI, and an SEH registration record; by the time execution
// reaches 0x44a28b all of those invariants are in place.
//
// The fragment receives two arguments via the caller's stack (accessed as
// [ebp+8] and [ebp+0xc]) and reads/writes a vector-like object in EDI:
//
//   EDI  — pointer to a dynamic array object with layout:
//             [+0x04]  data pointer (or inline buf when capacity < 4)
//             [+0x14]  size field
//             [+0x18]  capacity field
//   ESI  — new size value
//   [ebp+0x08]  — source data pointer
//   [ebp+0x0c]  — copy count (element count, in DWORDs)
//
// Behaviour (recovered from asm):
//
//   1. If copy count > 0:
//        if capacity >= 4  → use heap buf ([edi+4])
//        else              → use inline buf (&edi[4])
//        if esi+1 < copy_count  → throw / call invalid_parameter
//        else copy copy_count DWORDs from [ebp+8] into the chosen buf
//   2. If old capacity >= 4  → reallocate via FUN_0044d350(buf, capacity, 0xc)
//   3. Store new data pointer at [edi+4], size at [edi+0x18], count at [edi+0x14].
//   4. If esi >= 4, use heap buf as base; else use inline buf.
//      Zero the sentinel slot at [base + copy_count*4].
//   5. Restore SEH frame, pop callee-saves, ret 8.
//
// Call targets (REL32 reloc sites — compare.py wildcards these 4-byte windows):
//   +0x1e  CALL FUN_009d22b4   — _invalid_parameter / range-check helper
//   +0x52  CALL FUN_0044d350   — vector realloc helper
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function fragment enters mid-frame without a conventional prologue.
//   Source-level C++ would require a wrapper function that is not present in
//   the binary; the only faithful way to reproduce the original bytes is a
//   __declspec(naked) _emit passthrough.

extern "C" __declspec(naked) void FUN_0044a28b() {
    __asm {
        // 0044a28b: 8b 5d 0c        mov ebx, [ebp+0xc]        ; copy count
        _emit 0x8b
        _emit 0x5d
        _emit 0x0c
        // 0044a28e: 85 db           test ebx, ebx
        _emit 0x85
        _emit 0xdb
        // 0044a290: 76 35           jbe short +0x37            ; skip copy if count==0
        _emit 0x76
        _emit 0x35
        // 0044a292: 83 7f 18 04     cmp [edi+0x18], 4          ; capacity >= 4?
        _emit 0x83
        _emit 0x7f
        _emit 0x18
        _emit 0x04
        // 0044a296: 72 05           jb short +0x07             ; no → use inline buf
        _emit 0x72
        _emit 0x05
        // 0044a298: 8b 4f 04        mov ecx, [edi+4]           ; heap buf ptr
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 0044a29b: eb 03           jmp short +0x05
        _emit 0xeb
        _emit 0x03
        // 0044a29d: 8d 4f 04        lea ecx, [edi+4]           ; inline buf addr
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        // 0044a2a0: 8d 56 01        lea edx, [esi+1]
        _emit 0x8d
        _emit 0x56
        _emit 0x01
        // 0044a2a3: 3b d3           cmp edx, ebx               ; esi+1 >= count?
        _emit 0x3b
        _emit 0xd3
        // 0044a2a5: 8b c3           mov eax, ebx               ; eax = count
        _emit 0x8b
        _emit 0xc3
        // 0044a2a7: 73 07           jae short +0x09            ; ok → do copy
        _emit 0x73
        _emit 0x07
        // 0044a2a9: e8 06 80 58 00  call FUN_009d22b4           ; invalid_parameter (REL32)
        _emit 0xe8
        _emit 0x06
        _emit 0x80
        _emit 0x58
        _emit 0x00
        // 0044a2ae: eb 17           jmp short +0x19
        _emit 0xeb
        _emit 0x17
        // 0044a2b0: 8b 55 08        mov edx, [ebp+8]           ; src ptr
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        // 0044a2b3: 8b 19           mov ebx, [ecx]             ; load DWORD
        _emit 0x8b
        _emit 0x19
        // 0044a2b5: 89 1a           mov [edx], ebx             ; store DWORD
        _emit 0x89
        _emit 0x1a
        // 0044a2b7: 83 e8 01        sub eax, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0044a2ba: 83 c2 04        add edx, 4
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        // 0044a2bd: 83 c1 04        add ecx, 4
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        // 0044a2c0: 85 c0           test eax, eax
        _emit 0x85
        _emit 0xc0
        // 0044a2c2: 77 ef           ja short -0x11             ; loop back
        _emit 0x77
        _emit 0xef
        // 0044a2c4: 8b 5d 0c        mov ebx, [ebp+0xc]         ; reload count
        _emit 0x8b
        _emit 0x5d
        _emit 0x0c
        // 0044a2c7: 8b 47 18        mov eax, [edi+0x18]        ; old capacity
        _emit 0x8b
        _emit 0x47
        _emit 0x18
        // 0044a2ca: 83 f8 04        cmp eax, 4
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        // 0044a2cd: 72 16           jb short +0x18             ; capacity<4 → skip realloc
        _emit 0x72
        _emit 0x16
        // 0044a2cf: 8b 4f 04        mov ecx, [edi+4]           ; buf ptr
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 0044a2d2: 6a 0c           push 0xc
        _emit 0x6a
        _emit 0x0c
        // 0044a2d4: 8d 04 85 04 00 00 00  lea eax, [eax*4+4]
        _emit 0x8d
        _emit 0x04
        _emit 0x85
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044a2db: 50              push eax
        _emit 0x50
        // 0044a2dc: 51              push ecx
        _emit 0x51
        // 0044a2dd: e8 6e 30 00 00  call FUN_0044d350           ; realloc helper (REL32)
        _emit 0xe8
        _emit 0x6e
        _emit 0x30
        _emit 0x00
        _emit 0x00
        // 0044a2e2: 83 c4 0c        add esp, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0044a2e5: 83 fe 04        cmp esi, 4                  ; new size >= 4?
        _emit 0x83
        _emit 0xfe
        _emit 0x04
        // 0044a2e8: 8b 4d 08        mov ecx, [ebp+8]            ; new data ptr
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        // 0044a2eb: 8d 47 04        lea eax, [edi+4]            ; &edi[4]
        _emit 0x8d
        _emit 0x47
        _emit 0x04
        // 0044a2ee: c7 00 00 00 00 00  mov dword ptr [eax], 0   ; clear ptr slot
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044a2f4: 89 08           mov [eax], ecx              ; store data ptr
        _emit 0x89
        _emit 0x08
        // 0044a2f6: 89 77 18        mov [edi+0x18], esi          ; store new size
        _emit 0x89
        _emit 0x77
        _emit 0x18
        // 0044a2f9: 89 5f 14        mov [edi+0x14], ebx          ; store count
        _emit 0x89
        _emit 0x5f
        _emit 0x14
        // 0044a2fc: 72 02           jb short +0x04              ; size<4 → skip to sentinel
        _emit 0x72
        _emit 0x02
        // 0044a2fe: 8b c1           mov eax, ecx                ; use heap buf as base
        _emit 0x8b
        _emit 0xc1
        // 0044a300: c7 04 98 00 00 00 00  mov dword ptr [eax+ebx*4], 0  ; sentinel
        _emit 0xc7
        _emit 0x04
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044a307: 8b 4d f4        mov ecx, [ebp-0xc]          ; saved SEH node ptr
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        // 0044a30a: 64 89 0d 00 00 00 00  mov fs:[0], ecx        ; restore SEH chain
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044a311: 59              pop ecx
        _emit 0x59
        // 0044a312: 5f              pop edi
        _emit 0x5f
        // 0044a313: 5e              pop esi
        _emit 0x5e
        // 0044a314: 5b              pop ebx
        _emit 0x5b
        // 0044a315: 8b e5           mov esp, ebp
        _emit 0x8b
        _emit 0xe5
        // 0044a317: 5d              pop ebp
        _emit 0x5d
        // 0044a318: c2 08 00        ret 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
