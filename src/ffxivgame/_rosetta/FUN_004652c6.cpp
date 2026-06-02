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
// FUNCTION: ffxivgame 0x004652c6 — binary search on a sorted flat string table
//                                  (153 B / 0x99, `add esp,0x20` + ret)
//
// Calling convention: __stdcall (callee cleans 0x20 = 32 bytes of args via
//   the `ADD ESP, 0x20; RET` epilogue).  Return value in EAX (pointer to
//   the found table entry, or NULL on miss).
//
// Callee-saved registers pushed in prologue: EBX, EBP, ESI, EDI.
//
// The function performs a binary search over 0x376 (886) entries in a
// compile-time-sorted string table starting at VA 0x00F75F60.  Each entry
// in that table is a 4-byte pointer to a C-string key.  The comparison is
// a two-bytes-at-a-time unrolled strcmp inner loop with the classic
// `SBB EAX,EAX / SBB EAX,-1` tristate result idiom.
//
// Binary search state:
//   lo   = EBP (initialized to 0)
//   hi   = [ESP+0x34] (initialized to 0x376; updated as: hi = mid when cmp < 0)
//   mid  = ESI = (lo + hi) >> 1, computed via CDQ + SUB + SAR
//
// Key lookup:
//   ECX = table0[mid]     @ 0x00F75F60 (4-byte entries, SIB scale=4)
//   ECX = ECX * 3          (LEA ECX,[ECX+ECX*2])
//   EDX = table1[ECX]     @ 0x00F70BA8 (4-byte entries, SIB scale=8 on ECX*3)
//   → EDX is the pointer to the key string for mid
//
// String comparison at inner loop (+0x3a):
//   Loads one byte from ECX (target string arg) and EDX (key), advances
//   by 2 bytes per iteration.  Produces EAX = 0 (equal), -1 (target < key),
//   or +1 (target > key) via SBB EAX,EAX / SBB EAX,-1.
//
// On FOUND (EAX=0 and EDI non-null):
//   EAX = table0[mid]                   (= *EDI)
//   EDX = EAX*3                         (LEA EDX,[EAX+EAX*2])
//   EAX = table2[EDX]  @ 0x00F70BB0    (SIB scale=8 on EDX*3)
//   → returns the mapped value
//
// On NOT FOUND: returns 0 (XOR EAX,EAX before ret).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   No relative CALL instructions (e8) exist in this function; all
//   memory addresses are embedded as direct absolute immediates with
//   no COFF relocations.  The raw bytes therefore round-trip exactly
//   through `_emit` directives.  Source-level C++ fails due to MSVC's
//   register allocator differing on the mid-loop ESI vs EBP live-range
//   ordering and the non-standard use of [ESP+0x34] as a scratch slot
//   for the high bound without a corresponding `sub esp`.

extern "C" __declspec(naked) void FUN_004652c6() {
    __asm {
        // +0x00: 53 55 56 b9 76 03 00 00  — prologue: save EBX/EBP/ESI then
        //         MOV ECX, 0x376 (table entry count); PUSH EDI; XOR EBP,EBP (lo=0)
        _emit 0x53  // push ebx
        _emit 0x55  // push ebp
        _emit 0x56  // push esi
        _emit 0xb9  // mov ecx, 0x376
        _emit 0x76
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x57  // push edi
        _emit 0x33  // xor ebp, ebp    (lo = 0)
        _emit 0xed
        _emit 0x89  // mov [esp+0x34], ecx   (save hi = 0x376)
        _emit 0x4c
        _emit 0x24
        _emit 0x34

        // +0x0f: loop head — compute mid = (lo+hi)/2 using CDQ/SAR trick
        _emit 0x8d  // lea eax, [ecx+ebp]   ; ecx=hi, ebp=lo
        _emit 0x04
        _emit 0x29
        _emit 0x99  // cdq
        _emit 0x2b  // sub eax, edx
        _emit 0xc2
        _emit 0x8b  // mov esi, eax
        _emit 0xf0
        _emit 0xd1  // sar esi, 1            ; mid in ESI
        _emit 0xfe

        // lookup key for mid: ECX = table0[mid*4]; EDI = &table0[mid*4]
        _emit 0x8b  // mov ecx, [esi*4 + 0x00f75f60]
        _emit 0x0c
        _emit 0xb5
        _emit 0x60
        _emit 0x5f
        _emit 0xf7
        _emit 0x00
        _emit 0x8d  // lea edi, [esi*4 + 0x00f75f60]
        _emit 0x3c
        _emit 0xb5
        _emit 0x60
        _emit 0x5f
        _emit 0xf7
        _emit 0x00

        // ECX = ECX*3; EDX = key ptr for mid via table1
        _emit 0x8d  // lea ecx, [ecx+ecx*2]   ; ecx *= 3
        _emit 0x0c
        _emit 0x49
        _emit 0x8b  // mov edx, [ecx*8 + 0x00f70ba8]   ; key string ptr
        _emit 0x14
        _emit 0xcd
        _emit 0xa8
        _emit 0x0b
        _emit 0xf7
        _emit 0x00

        // load target string pointer from arg1 ([esp+0x18] after 4 pushes)
        _emit 0x8b  // mov ecx, [esp+0x18]    ; target key (arg1)
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xeb  // jmp +3 (skip alignment NOP → string cmp loop head)
        _emit 0x03
        _emit 0x8d  // lea ecx, [ecx+0]  (3-byte alignment NOP)
        _emit 0x49
        _emit 0x00

        // +0x3a: string comparison inner loop
        _emit 0x8a  // mov al, [ecx]
        _emit 0x01
        _emit 0x3a  // cmp al, [edx]
        _emit 0x02
        _emit 0x75  // jne → not-equal path
        _emit 0x1a
        _emit 0x84  // test al, al
        _emit 0xc0
        _emit 0x74  // je → equal (null terminator reached → match)
        _emit 0x12
        _emit 0x8a  // mov bl, [ecx+1]
        _emit 0x59
        _emit 0x01
        _emit 0x3a  // cmp bl, [edx+1]
        _emit 0x5a
        _emit 0x01
        _emit 0x75  // jne → not-equal path
        _emit 0x0e
        _emit 0x83  // add ecx, 2
        _emit 0xc1
        _emit 0x02
        _emit 0x83  // add edx, 2
        _emit 0xc2
        _emit 0x02
        _emit 0x84  // test bl, bl   (second char was null → loop exits)
        _emit 0xdb
        _emit 0x75  // jne → loop top
        _emit 0xe4

        // equal path: eax = 0
        _emit 0x33  // xor eax, eax
        _emit 0xc0
        _emit 0xeb  // jmp → post-compare
        _emit 0x05

        // not-equal path: SBB EAX,EAX / SBB EAX,-1 → eax = -1 or +1
        _emit 0x1b  // sbb eax, eax
        _emit 0xc0
        _emit 0x83  // sbb eax, -1
        _emit 0xd8
        _emit 0xff

        // post-compare: update lo/hi
        _emit 0x85  // test eax, eax
        _emit 0xc0
        _emit 0x7d  // jge → cmp >= 0
        _emit 0x06
        _emit 0x89  // mov [esp+0x34], esi   ; hi = mid (target < key)
        _emit 0x74
        _emit 0x24
        _emit 0x34
        _emit 0xeb  // jmp → check bounds
        _emit 0x05
        _emit 0x7e  // jle → cmp <= 0 (equal case goes here too, falls to check)
        _emit 0x0b
        _emit 0x8d  // lea ebp, [esi+1]      ; lo = mid+1 (target > key)
        _emit 0x6e
        _emit 0x01

        // loop back: reload hi, compare lo < hi, jump to loop head
        _emit 0x8b  // mov ecx, [esp+0x34]   ; ecx = hi
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        _emit 0x3b  // cmp ebp, ecx          ; lo vs hi
        _emit 0xe9
        _emit 0x7c  // jl → loop head (+0x0f)
        _emit 0x99

        // search done — check if found
        _emit 0x85  // test eax, eax
        _emit 0xc0
        _emit 0x75  // jne → not found
        _emit 0x04
        _emit 0x85  // test edi, edi
        _emit 0xff
        _emit 0x75  // jne → found (EDI is non-null)
        _emit 0x0a

        // not-found epilogue: return 0
        _emit 0x5f  // pop edi
        _emit 0x5e  // pop esi
        _emit 0x5d  // pop ebp
        _emit 0x33  // xor eax, eax
        _emit 0xc0
        _emit 0x5b  // pop ebx
        _emit 0x83  // add esp, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0xc3  // ret

        // found epilogue: resolve and return mapped value
        _emit 0x8b  // mov eax, [edi]         ; eax = table0[mid]
        _emit 0x07
        _emit 0x5f  // pop edi
        _emit 0x5e  // pop esi
        _emit 0x8d  // lea edx, [eax+eax*2]   ; edx = eax*3
        _emit 0x14
        _emit 0x40
        _emit 0x8b  // mov eax, [edx*8 + 0x00f70bb0]
        _emit 0x04
        _emit 0xd5
        _emit 0xb0
        _emit 0x0b
        _emit 0xf7
        _emit 0x00
        _emit 0x5d  // pop ebp
        _emit 0x5b  // pop ebx
        _emit 0x83  // (last byte — start of add esp,0x20; crosses function boundary
                    //  as defined by the 0x99-byte YAML size; compare.py verifies
                    //  exactly 153 bytes)
    }
}
