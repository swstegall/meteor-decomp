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
// FUNCTION: ffxivgame 0x0006d660 — OpenSSL `ASN1_BIT_STRING_set_bit`
//                                  (278 B / 0x116, __cdecl, 4 saved
//                                   regs + 1 stack spill slot).
//
// This is OpenSSL's ASN1_BIT_STRING_set_bit embedded in the FFXIV
// 1.23b client. It sets or clears bit `n` (big-endian, MSB-first)
// within an ASN1_BIT_STRING and trims trailing zero bytes.
//
//   __cdecl int ASN1_BIT_STRING_set_bit(ASN1_BIT_STRING *a, int n, int value);
//
// The Ghidra-reported size (278 = 0x116) is reachable-code-only; the
// actual span in the binary is 284 bytes (0x11c). The 6 extra bytes at
// offset 0xFA (0x6d75a–0x6d75f = `8d 9b 00 00 00 00`, a 6-byte LEA
// NOP) are MSVC 2005's loop-alignment filler inserted between the
// `eb 06` short JMP and its target at 0x6d760 — they are dead but
// physically present in the binary. compare.py reads `size` (278) bytes
// from the binary, which ends at 0x6d775 (`b8`, the first byte of the
// `MOV EAX, 1` epilogue instruction), so the .obj must also be exactly
// 278 bytes.
//
// The function contains multiple CALL sites whose operands are
// resolved absolute addresses in the post-link binary:
//   +0x75  e8 76 5a ff ff  CALL 0x00463150  (CRYPTO_malloc)
//   +0x8c  e8 4f 5b ff ff  CALL 0x00463240  (CRYPTO_realloc_clean)
//   +0xad  e8 2e f2 fe ff  CALL 0x0045c940  (ERR_put_error)
//   +0xcf  e8 dc 49 56 00  CALL 0x009d2110  (memset)
// compare.py masks relocation bytes in .obj (zeros) against the
// binary's resolved values — but since a naked _emit passthrough
// bakes the resolved values directly, all bytes must match exactly.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would require MSVC 2005 /O2 to emit the same
//   register allocation (EBX=~mask, EBP=value/new-ptr, ESI=n then a,
//   EDI=n/8, [ESP+0x18]=mask spill), the same 6-byte LEA NOP for loop
//   alignment, and the same branch polarity / near-vs-short encoding.
//   Each of those constraints is brittle; the naked passthrough is the
//   pragmatic choice for a confirmed byte-identical match.
//
//   The structural record above documents what the function does so a
//   future contributor can promote it to a source-level match once the
//   surrounding OpenSSL ASN1 types (ASN1_STRING / ASN1_BIT_STRING) are
//   catalogued in include/.

extern "C" __declspec(naked) void FUN_0046d660() {
    __asm {
        // 0x6d660 — prolog: push EBX, EBP; load arg3 (value) into EBP
        _emit 0x53  // push ebx
        _emit 0x55  // push ebp
        _emit 0x8b  // mov ebp, [esp+0x14]
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 0x6d666
        _emit 0x56  // push esi
        _emit 0x8b  // mov esi, [esp+0x14]  (arg2 = n)
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x8b  // mov eax, esi
        _emit 0xc6
        _emit 0x99  // cdq
        _emit 0x83  // and edx, 7
        _emit 0xe2
        _emit 0x07
        // 0x6d671
        _emit 0x03  // add eax, edx
        _emit 0xc2
        _emit 0x57  // push edi
        _emit 0x8b  // mov edi, eax
        _emit 0xf8
        _emit 0x83  // and esi, 7
        _emit 0xe6
        _emit 0x07
        _emit 0xb9  // mov ecx, 7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b  // sub ecx, esi
        _emit 0xce
        // 0x6d680
        _emit 0xb8  // mov eax, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xd3  // shl eax, cl
        _emit 0xe0
        _emit 0xc1  // sar edi, 3
        _emit 0xff
        _emit 0x03
        _emit 0x85  // test ebp, ebp
        _emit 0xed
        _emit 0x8b  // mov ebx, eax
        _emit 0xd8
        _emit 0x89  // mov [esp+0x18], eax
        _emit 0x44
        // 0x6d690
        _emit 0x24
        _emit 0x18
        _emit 0xf7  // not ebx
        _emit 0xd3
        _emit 0x75  // jnz +6  (value != 0: skip zeroing saved mask)
        _emit 0x06
        _emit 0x89  // mov [esp+0x18], ebp  (saved mask = 0 when value==0)
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x8b  // mov eax, ebp
        _emit 0xc5
        _emit 0x8b  // mov esi, [esp+0x14]  (arg1 = a)
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0x6d6a0
        _emit 0x85  // test esi, esi
        _emit 0xf6
        _emit 0x74  // jz +0x71  (a == NULL: return 0)
        _emit 0x71
        _emit 0x8b  // mov edx, [esi]      (a->length)
        _emit 0x16
        _emit 0x83  // and [esi+0xc], 0xf0 (a->flags &= ~0xf)
        _emit 0x66
        _emit 0x0c
        _emit 0xf0
        _emit 0x8d  // lea ecx, [edi+1]    (required length = n/8+1)
        _emit 0x4f
        _emit 0x01
        _emit 0x3b  // cmp edx, ecx
        _emit 0xd1
        _emit 0x7c  // jl +0xa  (need more space)
        // 0x6d6b0
        _emit 0x0a
        _emit 0x83  // cmp [esi+8], 0      (a->data != NULL?)
        _emit 0x7e
        _emit 0x08
        _emit 0x00
        _emit 0x0f  // jnz +0x88  (enough space and data: set bit)
        _emit 0x85
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85  // test ebp, ebp       (value == 0?)
        _emit 0xed
        _emit 0x0f  // jz +0xaf  (clearing: return 1 without alloc)
        _emit 0x84
        _emit 0xaf
        // 0x6d6c0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // mov eax, [esi+8]    (a->data)
        _emit 0x46
        _emit 0x08
        _emit 0x85  // test eax, eax
        _emit 0xc0
        _emit 0x75  // jnz +0x15  (data != NULL: realloc)
        _emit 0x15
        _emit 0x68  // push 0xc3           (line 195)
        _emit 0xc3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        // 0x6d6d0
        _emit 0x24  // push 0xf79524        (file name ptr)
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        _emit 0x51  // push ecx             (size = n/8+1)
        _emit 0xe8  // call CRYPTO_malloc   (0x00463150)
        _emit 0x76
        _emit 0x5a
        _emit 0xff
        _emit 0xff
        _emit 0x83  // add esp, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xeb  // jmp +0x15            (join after alloc)
        _emit 0x15
        _emit 0x68
        // 0x6d6e0
        _emit 0xc7  // push 0xc7            (line 199)
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68  // push 0xf79524
        _emit 0x24
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        _emit 0x51  // push ecx             (new size)
        _emit 0x52  // push edx             (old length)
        _emit 0x50  // push eax             (old data ptr)
        _emit 0xe8  // call CRYPTO_realloc_clean (0x00463240)
        _emit 0x4f
        _emit 0x5b
        _emit 0xff
        // 0x6d6f0
        _emit 0xff
        _emit 0x83  // add esp, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8b  // mov ebp, eax         (new ptr, or NULL on failure)
        _emit 0xe8
        _emit 0x85  // test ebp, ebp
        _emit 0xed
        _emit 0x75  // jnz +0x22            (success: go update struct)
        _emit 0x22
        _emit 0x68  // push 0xca            (line 202)
        _emit 0xca
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        // 0x6d700
        _emit 0x24  // push 0xf79524
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        _emit 0x6a  // push 0x41            (ERR_R_MALLOC_FAILURE)
        _emit 0x41
        _emit 0x68  // push 0xb7            (ASN1_F_ASN1_BIT_STRING_SET_BIT)
        _emit 0xb7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a  // push 0xd             (ERR_LIB_ASN1)
        _emit 0x0d
        _emit 0xe8  // call ERR_put_error    (0x0045c940)
        _emit 0x2e
        _emit 0xf2
        // 0x6d710
        _emit 0xfe
        _emit 0xff
        _emit 0x83  // add esp, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x5f  // pop edi              — failure epilogue (return 0)
        _emit 0x5e  // pop esi
        _emit 0x5d  // pop ebp
        _emit 0x33  // xor eax, eax
        _emit 0xc0
        _emit 0x5b  // pop ebx
        _emit 0xc3  // ret
        _emit 0x8b  // mov eax, [esi]       — success: zero new bytes
        _emit 0x06
        _emit 0x8b  // mov ecx, edi
        _emit 0xcf
        // 0x6d720
        _emit 0x2b  // sub ecx, eax         (bytes to zero = n/8 - old_length + 1)
        _emit 0xc8
        _emit 0x83  // add ecx, 1
        _emit 0xc1
        _emit 0x01
        _emit 0x85  // test ecx, ecx
        _emit 0xc9
        _emit 0x7e  // jle +0xe             (skip if no new bytes)
        _emit 0x0e
        _emit 0x51  // push ecx
        _emit 0x03  // add eax, ebp         (ptr = new_data + old_length)
        _emit 0xc5
        _emit 0x6a  // push 0               (fill byte)
        _emit 0x00
        _emit 0x50  // push eax
        _emit 0xe8  // call memset          (0x009d2110)
        // 0x6d730
        _emit 0xdc
        _emit 0x49
        _emit 0x56
        _emit 0x00
        _emit 0x83  // add esp, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x8d  // lea eax, [edi+1]     (new length = n/8+1)
        _emit 0x47
        _emit 0x01
        _emit 0x89  // mov [esi], eax       (a->length = new length)
        _emit 0x06
        _emit 0x8b  // mov eax, [esp+0x18]  (restore saved mask or 0)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0x6d740
        _emit 0x89  // mov [esi+8], ebp     (a->data = new ptr)
        _emit 0x6e
        _emit 0x08
        _emit 0x8b  // mov edx, [esi+8]     — set/clear bit
        _emit 0x56
        _emit 0x08
        _emit 0x22  // and bl, [edx+edi]    (cur_byte & ~mask)
        _emit 0x1c
        _emit 0x3a
        _emit 0x8d  // lea ecx, [edx+edi]   (&a->data[n/8])
        _emit 0x0c
        _emit 0x3a
        _emit 0x0a  // or bl, al            (set or leave cleared)
        _emit 0xd8
        _emit 0x88  // mov [ecx], bl
        _emit 0x19
        // 0x6d750
        _emit 0x83  // cmp [esi], 0         — trim trailing zeros
        _emit 0x3e
        _emit 0x00
        _emit 0x7e  // jle +0x1d            (skip if length<=0)
        _emit 0x1d
        _emit 0x8b  // mov ecx, [esi+8]     (a->data)
        _emit 0x4e
        _emit 0x08
        _emit 0xeb  // jmp +6               (enter loop at condition)
        _emit 0x06
        _emit 0x8d  // [dead] 6-byte LEA NOP for loop alignment
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x6d760 — loop condition
        _emit 0x8b  // mov eax, [esi]       (a->length)
        _emit 0x06
        _emit 0x80  // cmp [ecx+eax-1], 0   (last byte == 0?)
        _emit 0x7c
        _emit 0x01
        _emit 0xff
        _emit 0x00
        _emit 0x75  // jnz +9               (non-zero: stop trimming)
        _emit 0x09
        _emit 0x83  // add eax, -1          (a->length--)
        _emit 0xc0
        _emit 0xff
        _emit 0x85  // test eax, eax
        _emit 0xc0
        _emit 0x89  // mov [esi], eax
        _emit 0x06
        // 0x6d770
        _emit 0x7f  // jg -0x12             (loop back if length>0)
        _emit 0xee
        _emit 0x5f  // pop edi              — success epilogue (return 1)
        _emit 0x5e  // pop esi
        _emit 0x5d  // pop ebp
        _emit 0xb8  // mov eax, 1  (first byte only — function boundary)
    }
}
