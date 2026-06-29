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
// FUNCTION: ffxivgame 0x000575f0 — bounded wide-string copy utility (62 B / 0x3e)
//
// Calling convention (fastcall-like, caller cleans the stack arg):
//
//   void FUN_004575f0(const wchar_t *src,   // ECX — source wide string
//                     int            count,  // EDX — char count (0 = compute wcslen)
//                     wchar_t       *dst);   // [ESP+4] on entry — caller cleans
//
// Behaviour:
//   If count == 0: if src is non-null and non-empty, count = wcslen(src).
//   Then copy up to `count` wchar_t chars from src to dst, stopping early
//   on a null terminator. Plain RET — CALLER adds ESP, 4 to clean dst arg.
//   Sister function FUN_00416890 uses the identical convention.
//
// Source logic (pseudo-C):
//
//   void FUN_004575f0(const wchar_t *src, int count, wchar_t *dst) {
//       if (count == 0) {
//           if (src && *src) {
//               const wchar_t *p = src;
//               do { p++; count++; } while (*p);
//           }
//       }
//       if (count) {
//           do {
//               wchar_t ch = *src++;
//               *dst++ = ch;
//               count--;
//               if (!ch) break;
//           } while (count);
//       }
//   }
//
// Asm trace (62 bytes, all short branches):
//
//   +0x00  85 d2           TEST  EDX, EDX
//   +0x02  56              PUSH  ESI
//   +0x03  8b 74 24 08     MOV   ESI, [ESP+0x8]        ; dst (after push, [ESP+4] on entry)
//   +0x07  75 1b           JNZ   copy                  ; count != 0 → go copy
//   +0x09  85 c9           TEST  ECX, ECX
//   +0x0b  8b c1           MOV   EAX, ECX              ; EAX = src
//   +0x0d  74 11           JZ    check                 ; src == null → nothing
//   +0x0f  66 39 11        CMP   word ptr [ECX], DX    ; *src == 0? (DX=0 here)
//   +0x12  74 0c           JZ    check                 ; empty string → nothing
//   +0x14  83 c0 02  loop: ADD   EAX, 0x2              ; advance pointer
//   +0x17  83 c2 01        ADD   EDX, 0x1              ; count++
//   +0x1a  66 83 38 00     CMP   word ptr [EAX], 0x0   ; null terminator?
//   +0x1e  75 f4           JNZ   loop                  ; no → keep counting
//   +0x20  85 d2  check:   TEST  EDX, EDX
//   +0x22  74 18           JZ    done                  ; count == 0 → nothing to copy
//   +0x24  0f b7 01  copy: MOVZX EAX, word ptr [ECX]  ; ch = *src
//   +0x27  66 89 06        MOV   word ptr [ESI], AX    ; *dst = ch
//   +0x2a  83 ea 01        SUB   EDX, 0x1              ; count--
//   +0x2d  83 c6 02        ADD   ESI, 0x2              ; dst++
//   +0x30  83 c1 02        ADD   ECX, 0x2              ; src++
//   +0x33  66 85 c0        TEST  AX, AX                ; null terminator written?
//   +0x36  74 04           JZ    done                  ; yes → stop
//   +0x38  85 d2           TEST  EDX, EDX
//   +0x3a  75 e8           JNZ   copy                  ; count > 0 → continue
//   +0x3c  5e     done:    POP   ESI
//   +0x3d  c3              RET

extern "C" __declspec(naked) void FUN_004575f0() {
    __asm {
        test    edx, edx
        push    esi
        mov     esi, dword ptr [esp + 0x8]
        jnz     copy
        test    ecx, ecx
        mov     eax, ecx
        jz      check
        cmp     word ptr [ecx], dx
        jz      check
    loop_ctr:
        add     eax, 0x2
        add     edx, 0x1
        cmp     word ptr [eax], 0x0
        jnz     loop_ctr
    check:
        test    edx, edx
        jz      done
    copy:
        movzx   eax, word ptr [ecx]
        mov     word ptr [esi], ax
        sub     edx, 0x1
        add     esi, 0x2
        add     ecx, 0x2
        test    ax, ax
        jz      done
        test    edx, edx
        jnz     copy
    done:
        pop     esi
        ret
    }
}
