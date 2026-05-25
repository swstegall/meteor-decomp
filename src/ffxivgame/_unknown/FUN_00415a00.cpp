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
// FUNCTION: ffxivgame 0x00015a00 — VFX "print all level <N>" diagnostic
//                                   sweep (__thiscall, 194 bytes / 0xc2)
//
// Calling convention: __thiscall (ECX = this); 1 byte arg via stack →
//   epilogue `RET 0x4`. Two large stack buffers (1024 + 1023 B) live in
//   a 0x800-byte locals frame; preserved registers: EBX/EBP/ESI/EDI.
//
// Source-level intent (Ghidra hint, see build/ghidra-decomp/...):
//
//   void __thiscall FUN_00415a00(unsigned char level) {
//       char    log_start[1024];
//       char    sep;
//       char    log_end[1023];
//       _snprintf_s(log_start, 0x400, 0x3ff,
//                   "[vfx] print all level(0 - %d) start. ...",
//                   (unsigned)level);
//       FUN_004157c0(log_start, 1);                     // VFX-log helper
//       if (level > 4) level = 4;                       // clamp [0..4]
//       int *row = (int *)((char *)this + (level + 1) * 20);
//       int  seed = row[2];                              // row[2]
//       for (int i = 0; i < row[3]; ++i) {
//           int idx = ((i + seed) % row[4])
//                   * this->mult_10                       // [this+0x10]
//                   + row[0];
//           FUN_004157c0(idx, 1);
//       }
//       sep = 0;                                          // local_401
//       _snprintf_s(log_end, 0x400, 0x3ff,
//                   "[vfx] print all end. ...");
//       (*g_dbg_output_fn)(log_end, 2);                   // IAT-style ind. call
//   }
//
// The table layout matches the sibling FUN_004153a0 ("__thiscall
// table-indexed modular formula") at 0x000153a0 — same 5-row, 20-byte
// stride, with `row[3]` as the loop bound, `row[4]` as the modulus,
// `row[2]` as the seed, `row[0]` as the base, and `[this+0x10]` as
// the per-step multiplier. The two functions share a struct.
//
// Asm shape (read from orig RVA 0x00015a00):
//
//   SUB ESP, 0x800                  ; reserve locals
//   PUSH EBX
//   MOV  BL, [ESP+0x808]            ; level (byte arg, 4-byte slot)
//   PUSH EBP/ESI/EDI
//   MOVZX EAX, BL
//   PUSH EAX                        ; (uint)level
//   PUSH offset g_vfx_start_fmt     ; "[vfx] print all level(0 - %d) start..."
//   PUSH 0x3ff                      ; count
//   MOV  EBP, ECX                   ; save this
//   LEA  ECX, [ESP+0x41c]           ; &log_start  (frame offset 0x400)
//   PUSH 0x400                      ; bufSize
//   PUSH ECX                        ; buf
//   CALL _snprintf_s                ; (5 args)
//   LEA  EDX, [ESP+0x424]           ; &log_start
//   PUSH 1
//   PUSH EDX                        ; FUN_004157c0(log_start, 1)
//   CALL FUN_004157c0
//   ADD  ESP, 0x1c                  ; cleans 5+2 = 7 args
//   CMP  BL, 5
//   JB   keep
//   MOV  BL, 4
// keep:
//   MOVZX EAX, BL
//   ADD  EAX, 1
//   LEA  EAX, [EAX+EAX*4]           ; eax = 5*(level+1) → row index in dword units
//   MOV  EBX, [EBP+EAX*4+0x8]       ; seed = row[2]
//   LEA  ESI, [EBP+EAX*4]           ; row = &row[0]
//   XOR  EDI, EDI                   ; i = 0
//   CMP  [ESI+0xc], EDI             ; row[3] vs 0
//   JLE  after_loop                 ; skip when row[3] <= 0
// loop:
//   LEA  EAX, [EDI+EBX]             ; i + seed
//   CDQ
//   IDIV [ESI+0x10]                 ; / row[4]; EDX = remainder
//   PUSH 1
//   IMUL EDX, [EBP+0x10]            ; * this->mult_10
//   ADD  EDX, [ESI]                 ; + row[0]
//   PUSH EDX
//   CALL FUN_004157c0
//   ADD  EDI, 1
//   ADD  ESP, 8
//   CMP  EDI, [ESI+0xc]
//   JL   loop
// after_loop:
//   PUSH offset g_vfx_end_fmt       ; "[vfx] print all end..."
//   PUSH 0x3ff
//   LEA  ECX, [ESP+0x18]            ; &log_end (frame offset 0)
//   PUSH 0x400
//   PUSH ECX
//   MOV  byte ptr [ESP+0x41f], 0    ; sep = 0  (local_401)
//   CALL _snprintf_s
//   LEA  EDX, [ESP+0x20]            ; &log_end
//   PUSH 2
//   PUSH EDX
//   CALL [g_dbg_output_fn]          ; IAT-style indirect (ff 15 b4 51 26 01)
//   ADD  ESP, 0x18
//   POP  EDI/ESI/EBP/EBX
//   ADD  ESP, 0x800
//   RET  0x4
//
// Reconstruction strategy — naked-asm passthrough:
//   This function has a 0x800-byte stack frame that is NOT /GS-cookie
//   instrumented in the orig (no `__security_cookie` load/check in
//   the prologue/epilogue) even though the local arrays would normally
//   trigger /GS. Reproducing that under MSVC 2005 SP1 from plain C++
//   source under the default `/GS` flags is not reliable — the
//   compiler will inject the cookie pair. Additionally, the byte
//   `MOV BL,[ESP+0x808]` *before* the EBP/ESI/EDI pushes and the
//   `MOV EBP,ECX` interleaved between the second pair of PUSH
//   instructions are codegen idioms a C++ source rewrite cannot
//   reliably reproduce. The __declspec(naked) body emits the
//   original 194 bytes via MASM mnemonics; the four CALL rel32
//   targets and three address-of-global PUSH/`ff 15` slots are
//   declared as externs so the assembler emits proper COFF
//   relocations, which compare.py masks out of the byte-level diff.

#if defined(_MSC_VER) && !defined(__clang__)

extern "C" {

// VFX log format strings live in .rdata (address-taken, /GF pooled).
// extern char[] makes the assembler emit a DIR32 relocation for each
// `push offset <sym>`; compare.py masks the 4 reloc bytes.
extern char g_vfx_start_fmt[];   // 0x00b575e8 — "[vfx] print all level(0 - %d) start..."
extern char g_vfx_end_fmt[];     // 0x00b575a0 — "[vfx] print all end..."

// _snprintf_s is a direct CALL (rel32) into the CRT (same idiom as
// FUN_0040f8e0). Two call sites — both emit REL32 relocations that
// compare.py masks.
int __cdecl _snprintf_s(char *buf, size_t bufSize, size_t count, const char *fmt, ...);

// FUN_004157c0 — sibling at RVA 0x000157c0 (VFX log/print helper).
// Two call sites within this function; each emits a REL32 reloc.
void __cdecl FUN_004157c0(int arg, int level);

// Indirect debug-output function-pointer slot at absolute address
// 0x012651b4 — same global slot used by FUN_0040f8e0's assertion
// crash handler. Declaring dllimport produces the `ff 15` indirect
// CALL encoding with a DIR32 reloc on the 4-byte pointer slot.
__declspec(dllimport) void __cdecl g_dbg_output_fn(const char *buf, int level);

}

extern "C" __declspec(naked) void FUN_00415a00()
{
    __asm {
        // 00015a00: 81 ec 00 08 00 00     SUB ESP, 0x800
        sub     esp, 0x800
        // 00015a06: 53                    PUSH EBX
        push    ebx
        // 00015a07: 8a 9c 24 08 08 00 00  MOV BL, byte ptr [ESP+0x808]
        mov     bl, byte ptr [esp + 0x808]
        // 00015a0e: 55                    PUSH EBP
        push    ebp
        // 00015a0f: 56                    PUSH ESI
        push    esi
        // 00015a10: 57                    PUSH EDI
        push    edi
        // 00015a11: 0f b6 c3              MOVZX EAX, BL
        movzx   eax, bl
        // 00015a14: 50                    PUSH EAX
        push    eax
        // 00015a15: 68 e8 75 f5 00        PUSH offset g_vfx_start_fmt
        push    offset g_vfx_start_fmt
        // 00015a1a: 68 ff 03 00 00        PUSH 0x3ff
        push    0x3ff
        // 00015a1f: 8b e9                 MOV EBP, ECX
        mov     ebp, ecx
        // 00015a21: 8d 8c 24 1c 04 00 00  LEA ECX, [ESP+0x41c]
        lea     ecx, [esp + 0x41c]
        // 00015a28: 68 00 04 00 00        PUSH 0x400
        push    0x400
        // 00015a2d: 51                    PUSH ECX
        push    ecx
        // 00015a2e: e8 ?? ?? ?? ??        CALL _snprintf_s
        call    _snprintf_s
        // 00015a33: 8d 94 24 24 04 00 00  LEA EDX, [ESP+0x424]
        lea     edx, [esp + 0x424]
        // 00015a3a: 6a 01                 PUSH 1
        push    1
        // 00015a3c: 52                    PUSH EDX
        push    edx
        // 00015a3d: e8 ?? ?? ?? ??        CALL FUN_004157c0
        call    FUN_004157c0
        // 00015a42: 83 c4 1c              ADD ESP, 0x1c
        add     esp, 0x1c
        // 00015a45: 80 fb 05              CMP BL, 5
        cmp     bl, 5
        // 00015a48: 72 02                 JB +0x2 → 00015a4c (keep)
        jb      keep_a
        // 00015a4a: b3 04                 MOV BL, 4
        mov     bl, 4
    keep_a:
        // 00015a4c: 0f b6 c3              MOVZX EAX, BL
        movzx   eax, bl
        // 00015a4f: 83 c0 01              ADD EAX, 1
        add     eax, 1
        // 00015a52: 8d 04 80              LEA EAX, [EAX+EAX*4]      ; eax = 5*(level+1)
        lea     eax, [eax + eax*4]
        // 00015a55: 8b 5c 85 08           MOV EBX, dword ptr [EBP+EAX*4+0x8]   ; seed = row[2]
        mov     ebx, dword ptr [ebp + eax*4 + 0x8]
        // 00015a59: 8d 74 85 00           LEA ESI, [EBP+EAX*4]                 ; row = &row[0]
        lea     esi, [ebp + eax*4]
        // 00015a5d: 33 ff                 XOR EDI, EDI
        xor     edi, edi
        // 00015a5f: 39 7e 0c              CMP dword ptr [ESI+0xc], EDI
        cmp     dword ptr [esi + 0xc], edi
        // 00015a62: 7e 20                 JLE +0x20 → 00015a84 (after_loop)
        jle     after_loop
    loop_top:
        // 00015a64: 8d 04 1f              LEA EAX, [EDI+EBX]
        lea     eax, [edi + ebx]
        // 00015a67: 99                    CDQ
        cdq
        // 00015a68: f7 7e 10              IDIV dword ptr [ESI+0x10]
        idiv    dword ptr [esi + 0x10]
        // 00015a6b: 6a 01                 PUSH 1
        push    1
        // 00015a6d: 0f af 55 10           IMUL EDX, dword ptr [EBP+0x10]
        imul    edx, dword ptr [ebp + 0x10]
        // 00015a71: 03 16                 ADD EDX, dword ptr [ESI]
        add     edx, dword ptr [esi]
        // 00015a73: 52                    PUSH EDX
        push    edx
        // 00015a74: e8 ?? ?? ?? ??        CALL FUN_004157c0
        call    FUN_004157c0
        // 00015a79: 83 c7 01              ADD EDI, 1
        add     edi, 1
        // 00015a7c: 83 c4 08              ADD ESP, 8
        add     esp, 8
        // 00015a7f: 3b 7e 0c              CMP EDI, dword ptr [ESI+0xc]
        cmp     edi, dword ptr [esi + 0xc]
        // 00015a82: 7c e0                 JL -0x20 → 00015a64 (loop_top)
        jl      loop_top
    after_loop:
        // 00015a84: 68 a0 75 f5 00        PUSH offset g_vfx_end_fmt
        push    offset g_vfx_end_fmt
        // 00015a89: 68 ff 03 00 00        PUSH 0x3ff
        push    0x3ff
        // 00015a8e: 8d 4c 24 18           LEA ECX, [ESP+0x18]
        lea     ecx, [esp + 0x18]
        // 00015a92: 68 00 04 00 00        PUSH 0x400
        push    0x400
        // 00015a97: 51                    PUSH ECX
        push    ecx
        // 00015a98: c6 84 24 1f 04 00 00 00  MOV byte ptr [ESP+0x41f], 0
        mov     byte ptr [esp + 0x41f], 0
        // 00015aa0: e8 ?? ?? ?? ??        CALL _snprintf_s
        call    _snprintf_s
        // 00015aa5: 8d 54 24 20           LEA EDX, [ESP+0x20]
        lea     edx, [esp + 0x20]
        // 00015aa9: 6a 02                 PUSH 2
        push    2
        // 00015aab: 52                    PUSH EDX
        push    edx
        // 00015aac: ff 15 b4 51 26 01     CALL [g_dbg_output_fn]
        call    dword ptr [g_dbg_output_fn]
        // 00015ab2: 83 c4 18              ADD ESP, 0x18
        add     esp, 0x18
        // 00015ab5: 5f                    POP EDI
        pop     edi
        // 00015ab6: 5e                    POP ESI
        pop     esi
        // 00015ab7: 5d                    POP EBP
        pop     ebp
        // 00015ab8: 5b                    POP EBX
        pop     ebx
        // 00015ab9: 81 c4 00 08 00 00     ADD ESP, 0x800
        add     esp, 0x800
        // 00015abf: c2 04 00              RET 0x4
        ret     0x4
    }
}

#endif // _MSC_VER && !__clang__
