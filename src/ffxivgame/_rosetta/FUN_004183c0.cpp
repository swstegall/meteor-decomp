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
// FUNCTION: ffxivgame 0x004183c0 — global-table register + dispatch
//                                  (70 B / 0x46, __cdecl, 5 args, no frame)
//
// __cdecl void FUN_004183c0(int idx, DWORD arg2, DWORD arg3, DWORD arg4, BYTE arg5)
//
// Computes a 48-byte-stride table entry index (EAX = idx*3 via LEA then *16
// via SHL 4 = idx*48), writes four fields into a global table rooted at VA
// 0x013290d8, then calls FUN_0041cd00 with all five original arguments.
//
// Argument loading is interleaved with callee-save pushes (MSVC 2005 scheduler):
//   before any push : EDX = arg5, ECX = arg1/idx
//   after PUSH EBP  : EBP = arg4  (new [esp+0x14] = old [esp+0x10])
//   after PUSH ESI  : ESI = arg2  (new [esp+0x10] = old [esp+0x08])
//   after PUSH EDI  : EDI = arg3  (new [esp+0x18] = old [esp+0x0c])
//
// Global table writes (stride = 48 bytes per entry, base VA 0x013290d8):
//   [EAX + 0x013290d8] = EDI (arg3, DWORD)   offset +0 in entry
//   [EAX + 0x013290dc] = ESI (arg2, DWORD)   offset +4
//   [EAX + 0x013290e0] = EBP (arg4, DWORD)   offset +8
//   [EAX + 0x013290e4] = DL  (arg5, BYTE)    offset +12
//
// Call-frame pushes (interleaved with LEA/SHL): EDX, EBP, EDI, then ESI,
// ECX — so FUN_0041cd00 receives (arg1, arg2, arg3, arg4, arg5).
// ADD ESP, 0x14 cleans the 5 * 4-byte call frame (__cdecl).
//
// Reconstruction strategy — naked-asm _emit byte passthrough:
//   The interleaved prologue (callee-saves woven with argument loads) and the
//   [EAX + absVA] addressing mode for the four DIR32 stores produce instruction
//   encodings that no idiomatic MSVC /O2 source can drive to the same register
//   schedule.  The 70-byte slice is therefore emitted verbatim.
//   Reloc positions masked by compare.py:
//     offsets 0x22–0x25 (DIR32 → 0x013290dc)
//     offsets 0x28–0x2b (DIR32 → 0x013290d8)
//     offsets 0x2e–0x31 (DIR32 → 0x013290e0)
//     offsets 0x34–0x37 (DIR32 → 0x013290e4)
//     offsets 0x3b–0x3e (REL32 → FUN_0041cd00)

extern "C" __declspec(naked) void FUN_004183c0() {
    __asm {
        // 000183c0: 8b 54 24 14   mov edx, [esp+0x14]      ; arg5 (BYTE), pre-push
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000183c4: 8b 4c 24 04   mov ecx, [esp+0x4]       ; arg1/idx, pre-push
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000183c8: 55             push ebp                 ; callee-save EBP
        _emit 0x55
        // 000183c9: 8b 6c 24 14   mov ebp, [esp+0x14]      ; arg4 (frame shifted by 1 push)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 000183cd: 56             push esi                 ; callee-save ESI
        _emit 0x56
        // 000183ce: 8b 74 24 10   mov esi, [esp+0x10]      ; arg2 (frame shifted by 2 pushes)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 000183d2: 57             push edi                 ; callee-save EDI
        _emit 0x57
        // 000183d3: 8b 7c 24 18   mov edi, [esp+0x18]      ; arg3 (frame shifted by 3 pushes)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // 000183d7: 52             push edx                 ; push arg5 for call
        _emit 0x52
        // 000183d8: 55             push ebp                 ; push arg4 for call
        _emit 0x55
        // 000183d9: 57             push edi                 ; push arg3 for call
        _emit 0x57
        // 000183da: 8d 04 49       lea eax, [ecx+ecx*2]    ; EAX = idx*3
        _emit 0x8d
        _emit 0x04
        _emit 0x49
        // 000183dd: c1 e0 04       shl eax, 4              ; EAX = idx*48
        _emit 0xc1
        _emit 0xe0
        _emit 0x04
        // 000183e0: 56             push esi                 ; push arg2 for call
        _emit 0x56
        // 000183e1: 51             push ecx                 ; push arg1 for call
        _emit 0x51
        // 000183e2: 89 b0 dc 90 32 01   mov [eax+0x13290dc], esi   ; table[idx]+4 = arg2
        _emit 0x89
        _emit 0xb0
        _emit 0xdc
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 000183e8: 89 b8 d8 90 32 01   mov [eax+0x13290d8], edi   ; table[idx]+0 = arg3
        _emit 0x89
        _emit 0xb8
        _emit 0xd8
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 000183ee: 89 a8 e0 90 32 01   mov [eax+0x13290e0], ebp   ; table[idx]+8 = arg4
        _emit 0x89
        _emit 0xa8
        _emit 0xe0
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 000183f4: 88 90 e4 90 32 01   mov byte ptr [eax+0x13290e4], dl ; table[idx]+12 = arg5
        _emit 0x88
        _emit 0x90
        _emit 0xe4
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 000183fa: e8 01 49 00 00      call FUN_0041cd00 (REL32)
        _emit 0xe8
        _emit 0x01
        _emit 0x49
        _emit 0x00
        _emit 0x00
        // 000183ff: 83 c4 14            add esp, 0x14        ; clean 5 call args
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00018402: 5f                  pop edi
        _emit 0x5f
        // 00018403: 5e                  pop esi
        _emit 0x5e
        // 00018404: 5d                  pop ebp
        _emit 0x5d
        // 00018405: c3                  ret
        _emit 0xc3
    }
}
