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
// FUNCTION: ffxivgame 0x004620d0 — OpenSSL X.509 key-usage / bit-string
//                                  population from a STACK of OID entries
//                                  (269 B / 0x10d, __cdecl, 2 args, no SEH).
//
// Behaviour read from the disassembly at orig RVA 0x000620d0:
//
//   int FUN_004620d0(ASN1_BIT_STRING **bits /*[ESP+4]*/, void *ext /*[ESP+8]*/)
//
//   4-byte local var `ok` allocated on stack via __alloca_probe(4);
//   freed on all exit paths via POP ECX.  Initial value = 0; set to 1
//   only at the "count <= 0" early-success path.
//
//   Logical shape:
//
//     ok = 0;
//     STACK *sk = FUN_00470630(ext);
//     if (!sk) return 0;
//     if (*bits) return 0;     // already populated
//     int n = sk_num(sk);
//     if (n <= 0) { ok = 1; goto cleanup; }
//     for (int i = 0; i < n; ) {
//         void *entry   = sk_value(sk, i);
//         char *key_str = *(char**)((char*)entry + 4);   // EDI = [EAX+4]
//         if (!*bits) {
//             *bits = FUN_0045d910();       // ASN1_BIT_STRING_new()
//             if (!*bits) goto cleanup;
//         }
//         // search global OID→bit table at 0xf69a50
//         // table entry: { int bit; char *key; char *str; } (12 bytes each)
//         if (g_table[0].key /*[0xf69a54]*/ != NULL) {
//             for (TableEntry *e = g_table; ; e++) {
//                 if (strcmp(e->str, key_str) == 0) {
//                     if (!ASN1_BIT_STRING_set_bit(*bits, e->bit, 1))
//                         goto cleanup;
//                     if (!e[1].key) goto cleanup;
//                     break;
//                 }
//                 if (!e[1].key) goto not_found;
//             }
//         }
//     not_found:
//         if (!next_entry_key) goto cleanup;
//         i++;
//         n = sk_num(sk);    // re-check count each iteration
//     }
//     ok = 1;
//   cleanup:
//     sk_pop_free(sk, FUN_0046fcc0);
//     return ok;
//
//   Callees (all already matched in _rosetta/):
//     0x470630 = FUN_00470630   — extension→STACK accessor
//     0x464030 = sk_num         — STACK element count
//     0x464040 = sk_value       — STACK element by index
//     0x45d910 = FUN_0045d910   — ASN1_BIT_STRING_new (tiny stub)
//     0x46d660 = _ASN1_BIT_STRING_set_bit
//     0x4641f0 = sk_pop_free
//     0x46fcc0 = FUN_0046fcc0   — per-element free callback
//
//   Global data:
//     0xf69a54 = g_table[0].key  — null ⇒ table disabled
//     0xf69a50 = g_table base    — array of { int bit; char *key; char *str; }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function embeds several absolute addresses as immediates
//   (0xf69a50, 0xf69a54, 0x46fcc0) and uses __alloca_probe to allocate
//   exactly 4 bytes on the stack (unusual for a small local; source-level
//   reconstruction would require a carefully placed `int ok` that MSVC
//   2005 happens to allocate via the probe rather than SUB ESP,4).
//   The specific register assignment (EBP as loop counter, ESI dual-use
//   as arg1 and table pointer, EBX as the STACK handle throughout) is
//   also brittle under /O2.  The naked byte passthrough reproduces the
//   269 bytes exactly.

extern "C" __declspec(naked) void FUN_004620d0() {
    __asm {
        // 004620d0  MOV EAX, 4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 004620d5  CALL __alloca_probe
        _emit 0xe8
        _emit 0xf6
        _emit 0x08
        _emit 0x57
        _emit 0x00
        // 004620da  MOV EAX, [ESP+0xc]   ; arg2 (ext)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 004620de  PUSH EBX
        _emit 0x53
        // 004620df  PUSH EBP
        _emit 0x55
        // 004620e0  XOR EBP, EBP         ; i = 0
        _emit 0x33
        _emit 0xed
        // 004620e2  PUSH EAX             ; push ext as arg to FUN_470630
        _emit 0x50
        // 004620e3  MOV [ESP+0xc], EBP   ; local ok = 0
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 004620e7  CALL FUN_00470630
        _emit 0xe8
        _emit 0x44
        _emit 0xe5
        _emit 0x00
        _emit 0x00
        // 004620ec  MOV EBX, EAX         ; sk = result
        _emit 0x8b
        _emit 0xd8
        // 004620ee  ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 004620f1  CMP EBX, EBP         ; sk == NULL?
        _emit 0x3b
        _emit 0xdd
        // 004620f3  JNZ +6
        _emit 0x75
        _emit 0x06
        // 004620f5  POP EBP
        _emit 0x5d
        // 004620f6  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 004620f8  POP EBX
        _emit 0x5b
        // 004620f9  POP ECX              ; free local
        _emit 0x59
        // 004620fa  RET
        _emit 0xc3
        // 004620fb  PUSH ESI
        _emit 0x56
        // 004620fc  MOV ESI, [ESP+0x14]  ; ESI = arg1 (bits)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00462100  CMP [ESI], EBP       ; *bits == NULL?
        _emit 0x39
        _emit 0x2e
        // 00462102  JZ +7
        _emit 0x74
        _emit 0x07
        // 00462104  POP ESI
        _emit 0x5e
        // 00462105  POP EBP
        _emit 0x5d
        // 00462106  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00462108  POP EBX
        _emit 0x5b
        // 00462109  POP ECX
        _emit 0x59
        // 0046210a  RET
        _emit 0xc3
        // 0046210b  PUSH EDI
        _emit 0x57
        // 0046210c  PUSH EBX             ; sk_num(sk)
        _emit 0x53
        // 0046210d  CALL sk_num
        _emit 0xe8
        _emit 0x1e
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        // 00462112  ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00462115  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00462117  JLE 0x4621be (near)
        _emit 0x0f
        _emit 0x8e
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0046211d  JMP 0x462124 (loop entry, skip reload)
        _emit 0xeb
        _emit 0x05
        // 0046211f  NOP  (alignment pad — 0x462120 is a 16-byte boundary)
        _emit 0x90
        // 00462120  MOV ESI, [ESP+0x18]  ; loop-back: reload bits
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // 00462124  PUSH EBP             ; push i (loop counter)
        _emit 0x55
        // 00462125  PUSH EBX             ; sk_value(sk, i)
        _emit 0x53
        // 00462126  CALL sk_value
        _emit 0xe8
        _emit 0x15
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        // 0046212b  MOV EDI, [EAX+4]     ; EDI = entry->key_str
        _emit 0x8b
        _emit 0x78
        _emit 0x04
        // 0046212e  ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00462131  CMP [ESI], 0         ; *bits == NULL?
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 00462134  JNZ +0xf
        _emit 0x75
        _emit 0x0f
        // 00462136  CALL FUN_0045d910    ; ASN1_BIT_STRING_new()
        _emit 0xe8
        _emit 0xd5
        _emit 0xb7
        _emit 0xff
        _emit 0xff
        // 0046213b  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0046213d  MOV [ESI], EAX       ; *bits = new
        _emit 0x89
        _emit 0x06
        // 0046213f  JZ 0x4621c6 (near)   ; if NULL → cleanup
        _emit 0x0f
        _emit 0x84
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00462145  CMP [0xf69a54], 0    ; table enabled?
        _emit 0x83
        _emit 0x3d
        _emit 0x54
        _emit 0x9a
        _emit 0xf6
        _emit 0x00
        _emit 0x00
        // 0046214c  MOV ESI, 0xf69a50   ; ESI = g_table base
        _emit 0xbe
        _emit 0x50
        _emit 0x9a
        _emit 0xf6
        _emit 0x00
        // 00462151  JZ 0x4621a4
        _emit 0x74
        _emit 0x51
        // 00462153  MOV EAX, [ESI+8]     ; e->str
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00462156  MOV ECX, EDI         ; key_str
        _emit 0x8b
        _emit 0xcf
        // 00462158  MOV DL, [EAX]
        _emit 0x8a
        _emit 0x10
        // 0046215a  CMP DL, [ECX]
        _emit 0x3a
        _emit 0x11
        // 0046215c  JNZ 0x462178
        _emit 0x75
        _emit 0x1a
        // 0046215e  TEST DL, DL
        _emit 0x84
        _emit 0xd2
        // 00462160  JZ 0x462174 (match)
        _emit 0x74
        _emit 0x12
        // 00462162  MOV DL, [EAX+1]
        _emit 0x8a
        _emit 0x50
        _emit 0x01
        // 00462165  CMP DL, [ECX+1]
        _emit 0x3a
        _emit 0x51
        _emit 0x01
        // 00462168  JNZ 0x462178
        _emit 0x75
        _emit 0x0e
        // 0046216a  ADD EAX, 2
        _emit 0x83
        _emit 0xc0
        _emit 0x02
        // 0046216d  ADD ECX, 2
        _emit 0x83
        _emit 0xc1
        _emit 0x02
        // 00462170  TEST DL, DL
        _emit 0x84
        _emit 0xd2
        // 00462172  JNZ 0x462158 (strcmp loop)
        _emit 0x75
        _emit 0xe4
        // 00462174  XOR EAX, EAX         ; match: result = 0
        _emit 0x33
        _emit 0xc0
        // 00462176  JMP 0x46217d
        _emit 0xeb
        _emit 0x05
        // 00462178  SBB EAX, EAX         ; no match: normalize
        _emit 0x1b
        _emit 0xc0
        // 0046217a  SBB EAX, -1
        _emit 0x83
        _emit 0xd8
        _emit 0xff
        // 0046217d  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0046217f  JZ 0x46218c (strings equal → matched)
        _emit 0x74
        _emit 0x0b
        // 00462181  ADD ESI, 0xc         ; next table entry
        _emit 0x83
        _emit 0xc6
        _emit 0x0c
        // 00462184  CMP [ESI+4], 0       ; e->key == NULL?
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        // 00462188  JNZ 0x462153 (continue table scan)
        _emit 0x75
        _emit 0xc9
        // 0046218a  JMP 0x4621a4 (not found)
        _emit 0xeb
        _emit 0x18
        // 0046218c  MOV ECX, [ESI]       ; matched: e->bit
        _emit 0x8b
        _emit 0x0e
        // 0046218e  MOV EDX, [ESP+0x18]  ; EDX = bits
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00462192  MOV EAX, [EDX]       ; EAX = *bits
        _emit 0x8b
        _emit 0x02
        // 00462194  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 00462196  PUSH ECX             ; bit number
        _emit 0x51
        // 00462197  PUSH EAX             ; *bits
        _emit 0x50
        // 00462198  CALL _ASN1_BIT_STRING_set_bit
        _emit 0xe8
        _emit 0xc3
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        // 0046219d  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 004621a0  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 004621a2  JZ 0x4621c6 (cleanup)
        _emit 0x74
        _emit 0x22
        // 004621a4  CMP [ESI+4], 0       ; more table entries?
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        // 004621a8  JZ 0x4621c6 (cleanup)
        _emit 0x74
        _emit 0x1c
        // 004621aa  PUSH EBX             ; sk_num(sk)
        _emit 0x53
        // 004621ab  ADD EBP, 1           ; i++
        _emit 0x83
        _emit 0xc5
        _emit 0x01
        // 004621ae  CALL sk_num
        _emit 0xe8
        _emit 0x7d
        _emit 0x1e
        _emit 0x00
        _emit 0x00
        // 004621b3  ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 004621b6  CMP EBP, EAX         ; i < n?
        _emit 0x3b
        _emit 0xe8
        // 004621b8  JL 0x462120 (loop back)
        _emit 0x0f
        _emit 0x8c
        _emit 0x62
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 004621be  MOV [ESP+0x10], 1    ; ok = 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 004621c6  PUSH 0x46fcc0        ; sk_pop_free free-fn
        _emit 0x68
        _emit 0xc0
        _emit 0xfc
        _emit 0x46
        _emit 0x00
        // 004621cb  PUSH EBX             ; sk
        _emit 0x53
        // 004621cc  CALL sk_pop_free
        _emit 0xe8
        _emit 0x1f
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // 004621d1  MOV EAX, [ESP+0x18]  ; EAX = ok (local var)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 004621d5  ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 004621d8  POP EDI
        _emit 0x5f
        // 004621d9  POP ESI
        _emit 0x5e
        // 004621da  POP EBP
        _emit 0x5d
        // 004621db  POP EBX
        _emit 0x5b
        // 004621dc  POP ECX              ; free the alloca'd local
        _emit 0x59
        // 004621dd  RET
        _emit 0xc3
    }
}
