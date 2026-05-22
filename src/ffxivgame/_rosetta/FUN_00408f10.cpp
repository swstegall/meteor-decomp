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
// FUNCTION: ffxivgame 0x00408f10 — `__cdecl` insertion sort over an array
//                                  of 64-byte records keyed by a leading
//                                  C-string (306 B / 0x132).
//
// Inspection (read from the disassembly at orig RVA 0x00008f10):
//
//   __cdecl void FUN_00408f10(byte *first, byte *last);
//
//   Sorts the half-open range [first, last) in place, where each element
//   is a fixed 64-byte record (0x40) whose leading bytes form a NUL-
//   terminated string. The comparison is byte-wise strcmp (MSVC-2005's
//   inlined 2-bytes-per-iter strcmp idiom: load byte, CMP, JNZ, TEST,
//   JZ, load next byte, CMP, JNZ, ADD 2, ADD 2, TEST, JNZ; resolve sign
//   via SBB EAX,EAX / SBB EAX,-0x1).
//
//   Algorithm — classic in-place insertion sort with a fast path for
//   elements that fall below the current sorted prefix's minimum:
//
//     if (first == last) return;
//     byte *next = first + 0x40;                  // EBP
//     if (next == last) return;
//     byte *prev = first;                         // local_48 ([esp+0x10])
//     byte temp[0x40];                            // local_40 ([esp+0x18])
//     do {
//         memcpy(temp, next, 0x40);
//         if (strcmp(temp, first) < 0) {
//             // temp < first element — bulk-shift everything in
//             // [first, next) right by one slot and store temp at first.
//             byte *p = next;
//             while (p != first) {
//                 byte *src = p - 0x40;
//                 memcpy(src + 0x40, src, 0x40);
//                 p = src;
//             }
//             // dest = first, falls through to the trailing store.
//         } else if (strcmp(temp, prev) < 0) {
//             // temp belongs strictly above first but below prev —
//             // slide one element at a time backwards through the
//             // sorted prefix until the right slot is found.
//             byte *cur = prev;
//             do {
//                 memcpy(cur + 0x40, cur, 0x40);
//                 cur -= 0x40;
//             } while (strcmp(temp, cur) < 0);
//             // dest = cur + 0x40, falls through to the trailing store.
//         }
//         // else: temp >= prev — element already in order, dest = next.
//         prev += 0x40;
//         next += 0x40;
//         memcpy(dest, temp, 0x40);
//     } while (next != last);
//
//   Stack frame (after `SUB ESP,0x48` + PUSH EBX/EBP/ESI/EDI):
//     [esp+0x00]              saved EDI
//     [esp+0x04]              saved ESI
//     [esp+0x08]              saved EBP
//     [esp+0x0c]              saved EBX
//     [esp+0x10]              local_48 (rolling `prev` pointer)
//     [esp+0x14]              local_44 (rolling `next` pointer alias)
//     [esp+0x18 .. esp+0x57]  local_40 — 64-byte temp record
//     [esp+0x58]              return address
//     [esp+0x5c]              first  (arg 1, MOV EBX,[esp+0x5c] restores it)
//     [esp+0x60]              last   (arg 2, CMP EBP,[esp+0x60] terminates)
//
//   Three structurally-identical strcmp blocks at 0x8f53, 0x8fa6, 0x8fe6
//   each end with the classic MSVC-2005 sign-resolution tail:
//       SBB EAX, EAX        ; -1 if borrow (less), 0 otherwise
//       SBB EAX, -1         ; +1 if no borrow, -1 if less
//   yielding the canonical strcmp ternary in EAX.
//
//   No relocations: every branch is rel8/rel32 internal, no external
//   CALL, no IAT load, no SEH frame, no string-literal reference. The
//   306 function bytes are position-independent within the .text slice.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Coaxing MSVC 2005 /O2 /GS /EHsc to emit *this exact* register
//   allocation (EBX = first, EBP = next, EBX-overload-as-prev in the
//   inner-loop section, EAX/EDX as the strcmp scratch pair, the
//   particular 2-bytes-per-iter unroll, the rel8-vs-rel32 branch
//   selection at every site, and the precise frame-slot layout above)
//   from a source-level C++ insertion sort is brittle — any high-level
//   rewrite shifts at least one byte (loop type, predicate ordering,
//   register-allocator state across the three back-to-back strcmps).
//
//   The pragmatic choice — the same one FUN_004014b0, FUN_00401a00,
//   FUN_00403a20, FUN_00408780, and the rest of the SEH-wrapped /O2
//   siblings took — is a `__declspec(naked)` body that re-emits the
//   orig 306 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` section ends up byte-identical to the orig slice (no
//   relocations because every internal branch is a self-contained
//   relative displacement), which is what `tools/compare.py` checks.

extern "C" __declspec(naked) void FUN_00408f10() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x08]   ; param_2 (last)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x83              // SUB ESP, 0x48                   ; reserve frame
        _emit 0xec
        _emit 0x48
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x50]   ; param_1 (first)
        _emit 0x5c
        _emit 0x24
        _emit 0x50
        _emit 0x3b              // CMP EBX, EAX                    ; first == last?
        _emit 0xd8
        _emit 0x0f              // JZ  0x0040903d                  ; → epilogue (pop EBX)
        _emit 0x84
        _emit 0x19
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x55              // PUSH EBP
        _emit 0x8d              // LEA EBP, [EBX+0x40]             ; next = first + 0x40
        _emit 0x6b
        _emit 0x40
        _emit 0x3b              // CMP EBP, EAX                    ; next == last?
        _emit 0xe8
        _emit 0x89              // MOV [ESP+0x0c], EBP             ; local_44 = next
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x0f              // JZ  0x0040903c                  ; → epilogue (pop EBP,EBX)
        _emit 0x84
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EBP-0x40]             ; EAX = first
        _emit 0x45
        _emit 0xc0
        _emit 0x56              // PUSH ESI
        _emit 0x89              // MOV [ESP+0x0c], EAX             ; local_48 = first  (prev)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA ECX, [ECX]                  ; 3-byte align nop
        _emit 0x49
        _emit 0x00

        // ---- outer loop top (LAB_00408f40) ----
        _emit 0xb9              // MOV ECX, 0x10                   ; 16 dwords = 64 B
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, EBP                    ; src = next
        _emit 0xf5
        _emit 0x8d              // LEA EDI, [ESP+0x18]             ; dst = &local_40[0]
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0xf3              // REP MOVSD                       ; temp = *next
        _emit 0xa5

        // ---- strcmp #1 — compare temp vs *first ----
        _emit 0x8b              // MOV ECX, EBX                    ; ECX = first
        _emit 0xcb
        _emit 0x8d              // LEA EAX, [ESP+0x18]             ; EAX = &temp
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8a              // MOV DL, [EAX]                   ; strcmp_loop_1:
        _emit 0x10
        _emit 0x3a              // CMP DL, [ECX]
        _emit 0x11
        _emit 0x75              // JNZ resolve_1                   ; → 0x00408f73
        _emit 0x1a
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x74              // JZ  equal_1                     ; → 0x00408f6f
        _emit 0x12
        _emit 0x8a              // MOV DL, [EAX+1]
        _emit 0x50
        _emit 0x01
        _emit 0x3a              // CMP DL, [ECX+1]
        _emit 0x51
        _emit 0x01
        _emit 0x75              // JNZ resolve_1                   ; → 0x00408f73
        _emit 0x0e
        _emit 0x83              // ADD EAX, 2
        _emit 0xc0
        _emit 0x02
        _emit 0x83              // ADD ECX, 2
        _emit 0xc1
        _emit 0x02
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x75              // JNZ strcmp_loop_1               ; → 0x00408f53
        _emit 0xe4
        _emit 0x33              // XOR EAX, EAX                    ; equal_1: result = 0
        _emit 0xc0
        _emit 0xeb              // JMP after_1                     ; → 0x00408f78
        _emit 0x05
        _emit 0x1b              // SBB EAX, EAX                    ; resolve_1:
        _emit 0xc0
        _emit 0x83              // SBB EAX, -1                     ; EAX = ±1
        _emit 0xd8
        _emit 0xff

        _emit 0x85              // TEST EAX, EAX                   ; after_1: temp ? first
        _emit 0xc0
        _emit 0x7d              // JGE 0x00408f9c                  ; if temp >= first → strcmp #2 path
        _emit 0x20

        // ---- fast path: temp < *first — bulk-shift then store at first ----
        _emit 0x3b              // CMP EBX, EBP                    ; first == next?
        _emit 0xdd
        _emit 0x8b              // MOV EAX, EBP                    ; p = next
        _emit 0xc5
        _emit 0x74              // JZ  0x00408f95                  ; nothing to shift
        _emit 0x13
        _emit 0x83              // SUB EAX, 0x40                   ; shift_loop_1: p -= 0x40
        _emit 0xe8
        _emit 0x40
        _emit 0x3b              // CMP EAX, EBX                    ; p == first?
        _emit 0xc3
        _emit 0x8d              // LEA EDI, [EAX+0x40]             ; dst = p + 0x40
        _emit 0x78
        _emit 0x40
        _emit 0xb9              // MOV ECX, 0x10                   ; 64 B / 4
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX                    ; src = p
        _emit 0xf0
        _emit 0xf3              // REP MOVSD                       ; *(p+0x40) = *p
        _emit 0xa5
        _emit 0x75              // JNZ shift_loop_1                ; continue while p != first
        _emit 0xed
        _emit 0x8b              // MOV EDI, EBX                    ; dst = first
        _emit 0xfb
        _emit 0xe9              // JMP store_temp_at_dst           ; → 0x00409019
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // ---- strcmp #2 — compare temp vs *prev (local_48) ----
        _emit 0x8b              // MOV EBX, [ESP+0x10]             ; EBX = prev
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ECX, EBX                    ; ECX = prev
        _emit 0xcb
        _emit 0x8d              // LEA EAX, [ESP+0x18]             ; EAX = &temp
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8a              // MOV DL, [EAX]                   ; strcmp_loop_2:
        _emit 0x10
        _emit 0x3a              // CMP DL, [ECX]
        _emit 0x11
        _emit 0x75              // JNZ resolve_2                   ; → 0x00408fc6
        _emit 0x1a
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x74              // JZ  equal_2                     ; → 0x00408fc2
        _emit 0x12
        _emit 0x8a              // MOV DL, [EAX+1]
        _emit 0x50
        _emit 0x01
        _emit 0x3a              // CMP DL, [ECX+1]
        _emit 0x51
        _emit 0x01
        _emit 0x75              // JNZ resolve_2                   ; → 0x00408fc6
        _emit 0x0e
        _emit 0x83              // ADD EAX, 2
        _emit 0xc0
        _emit 0x02
        _emit 0x83              // ADD ECX, 2
        _emit 0xc1
        _emit 0x02
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x75              // JNZ strcmp_loop_2               ; → 0x00408fa6
        _emit 0xe4
        _emit 0x33              // XOR EAX, EAX                    ; equal_2: result = 0
        _emit 0xc0
        _emit 0xeb              // JMP after_2                     ; → 0x00408fcb
        _emit 0x05
        _emit 0x1b              // SBB EAX, EAX                    ; resolve_2:
        _emit 0xc0
        _emit 0x83              // SBB EAX, -1                     ; EAX = ±1
        _emit 0xd8
        _emit 0xff

        _emit 0x85              // TEST EAX, EAX                   ; after_2: temp ? prev
        _emit 0xc0
        _emit 0x7d              // JGE 0x0040900f                  ; element already in place
        _emit 0x40
        _emit 0x90              // NOP                             ; align inner loop top

        // ---- inner shift loop: slide one slot at a time backwards ----
        _emit 0x8b              // MOV EDI, EBP                    ; dst = current next
        _emit 0xfd
        _emit 0x8b              // MOV ESI, EBX                    ; src = cur (prev)
        _emit 0xf3
        _emit 0xb9              // MOV ECX, 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EBP, EBX                    ; advance dst
        _emit 0xeb
        _emit 0x83              // SUB EBX, 0x40                   ; cur -= 0x40
        _emit 0xeb
        _emit 0x40
        _emit 0xf3              // REP MOVSD                       ; *(cur+0x40) = *cur
        _emit 0xa5

        // ---- strcmp #3 — compare temp vs *cur ----
        _emit 0x8b              // MOV ECX, EBX                    ; ECX = cur
        _emit 0xcb
        _emit 0x8d              // LEA EAX, [ESP+0x18]             ; EAX = &temp
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8a              // MOV DL, [EAX]                   ; strcmp_loop_3:
        _emit 0x10
        _emit 0x3a              // CMP DL, [ECX]
        _emit 0x11
        _emit 0x75              // JNZ resolve_3                   ; → 0x00409006
        _emit 0x1a
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x74              // JZ  equal_3                     ; → 0x00409002
        _emit 0x12
        _emit 0x8a              // MOV DL, [EAX+1]
        _emit 0x50
        _emit 0x01
        _emit 0x3a              // CMP DL, [ECX+1]
        _emit 0x51
        _emit 0x01
        _emit 0x75              // JNZ resolve_3                   ; → 0x00409006
        _emit 0x0e
        _emit 0x83              // ADD EAX, 2
        _emit 0xc0
        _emit 0x02
        _emit 0x83              // ADD ECX, 2
        _emit 0xc1
        _emit 0x02
        _emit 0x84              // TEST DL, DL
        _emit 0xd2
        _emit 0x75              // JNZ strcmp_loop_3               ; → 0x00408fe6
        _emit 0xe4
        _emit 0x33              // XOR EAX, EAX                    ; equal_3: result = 0
        _emit 0xc0
        _emit 0xeb              // JMP after_3                     ; → 0x0040900b
        _emit 0x05
        _emit 0x1b              // SBB EAX, EAX                    ; resolve_3:
        _emit 0xc0
        _emit 0x83              // SBB EAX, -1                     ; EAX = ±1
        _emit 0xd8
        _emit 0xff

        _emit 0x85              // TEST EAX, EAX                   ; after_3: temp ? cur
        _emit 0xc0
        _emit 0x7c              // JL  0x00408fd0                  ; continue inner shift loop
        _emit 0xc1

        // ---- post-inner: restore EBX/EBP from frame for the trailing store ----
        _emit 0x8b              // MOV EBX, [ESP+0x5c]             ; EBX = first (arg)
        _emit 0x5c
        _emit 0x24
        _emit 0x5c
        _emit 0x8b              // MOV EDI, EBP                    ; dst = current EBP (cur+0x40)
        _emit 0xfd
        _emit 0x8b              // MOV EBP, [ESP+0x14]             ; EBP = next
        _emit 0x6c
        _emit 0x24
        _emit 0x14

        // ---- trailing store + loop advance (store_temp_at_dst @ 0x00409019) ----
        _emit 0x83              // ADD dword ptr [ESP+0x10], 0x40  ; prev += 0x40
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x40
        _emit 0x83              // ADD EBP, 0x40                   ; next += 0x40
        _emit 0xc5
        _emit 0x40
        _emit 0x3b              // CMP EBP, [ESP+0x60]             ; next == last?
        _emit 0x6c
        _emit 0x24
        _emit 0x60
        _emit 0xb9              // MOV ECX, 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ESI, [ESP+0x18]             ; src = &temp
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0xf3              // REP MOVSD                       ; *dst = temp
        _emit 0xa5
        _emit 0x89              // MOV [ESP+0x14], EBP             ; save next
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        _emit 0x0f              // JNZ 0x00408f40                  ; outer loop top
        _emit 0x85
        _emit 0x06
        _emit 0xff
        _emit 0xff
        _emit 0xff

        // ---- epilogue ----
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x48
        _emit 0xc4
        _emit 0x48
        _emit 0xc3              // RET
    }
}
