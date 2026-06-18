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
// FUNCTION: ffxivgame 0x00015a00 — Printer::PrintTier (194 B / 0xc2)
//                                  Logs all ring-buffer lines for one of the
//                                  five severity tiers stored in the Printer
//                                  object (see decomp-notes/types/ffxivgame/
//                                  0x000158b0.md for the full field map).
//
// Calling convention: __thiscall, `ret 4` (one DWORD stack arg = tier byte).
//
//   void __thiscall Printer::PrintTier(this, unsigned char tier)
//     ECX     = this (Printer*)
//     [ESP+4] = tier (0..4, clamped; values ≥5 are treated as 4)
//
// Behaviour (read from asm/ffxivgame/00015a00_FUN_00415a00.s):
//
//   1. Formats `tier` into a 1 KB stack buffer (buf1 @ [ESP-0x400])
//      using _snprintf_s(buf1, 0x400, 0x3ff, fmt_header, tier).
//      The format string lives at .rdata 0xf575e8.
//   2. Emits buf1 via FUN_004157c0(buf1, /*newline=*/1).
//   3. Clamps tier to the range [0, 4] (5 tiers total).
//   4. Selects the corresponding view slot in the Printer (each view is
//      20 bytes wide: data_ptr, meta_ptr, head_pos, tail_pos, chunks).
//      The index formula is: EAX = (tier+1)*5; view = &this[EAX] (dwords).
//   5. Loops view.tail_pos times, computing the physical ring-buffer
//      address for each logical line:
//
//        physical = ((i + view.head_pos) % view.chunks) * this->m_cols
//                   + view.data_ptr
//
//      Each physical address is passed to FUN_004157c0(addr, 1).
//   6. Formats a footer/separator into a second 1 KB buffer (buf2 @
//      [ESP-0x800]) using _snprintf_s without extra args.
//      The format string lives at .rdata 0xf575a0.
//   7. Dispatches buf2 via the IAT log emitter [0x012651b4] with level 2.
//
// Stack frame layout (0x800-byte local allocation + 4 saved regs):
//   [ESP - 0x800 .. - 0x401]  buf2 (second 1 KB, for the footer)
//   [ESP - 0x400 .. - 0x001]  buf1 (first 1 KB, for the header line)
//   (the frame is allocated as a single SUB ESP, 0x800; buf1 occupies
//    the upper half and buf2 the lower half at runtime)
//
// Reloc-bearing call sites in the orig 194 bytes:
//   +0x2e  rel32  → 0x009d4f9f  (_snprintf_s)
//   +0x3d  rel32  → 0x004157c0  (FUN_004157c0, log-line emitter)
//   +0x74  rel32  → 0x004157c0  (FUN_004157c0, inner loop)
//   +0xac  rel32  → 0x009d4f9f  (_snprintf_s, footer)
//   +0xac  dir32  → 0x012651b4  (IAT: log output dispatch)
//
// Reconstruction strategy — symbolic naked-asm (same approach as
// FUN_00409120 / FUN_00408230): CALL rel32 sites are wildcarded by
// tools/compare.py via the COFF reloc table; all remaining bytes are
// determined by MASM's standard x86 encoding rules.  The two absolute-
// address PUSHes (format strings 0xf575e8 and 0xf575a0) and the IAT-
// call operand are emitted as raw immediates / symbol references exactly
// matching the PE's resolved bytes.

extern "C" {
    void FUN_009d4f9f();   // _snprintf_s(buf, buf_sz, count, fmt, ...)
    void FUN_004157c0();   // log-line emitter (cdecl, 2 args)
    extern int ext_12651b4;  // IAT slot: output dispatch (level 2)
}

extern "C" __declspec(naked) void FUN_00415a00() {
    __asm {
        // 00015a00
        sub  esp, 0x800
        push ebx
        // 00015a07  load the tier byte from [ESP+0x808] (= original arg[0])
        mov  bl, byte ptr [esp + 0x808]
        push ebp
        push esi
        push edi
        // 00015a11  zero-extend BL → EAX for the _snprintf_s vararg
        movzx eax, bl
        push eax                    // arg5: tier value
        push 0xf575e8               // arg4: format string (.rdata)
        push 0x3ff                  // arg3: max count (without NUL)
        mov  ebp, ecx               // save this pointer
        lea  ecx, [esp + 0x41c]     // buf1 = frame_base - 0x400
        push 0x400                  // arg2: buffer size
        push ecx                    // arg1: buf1
        call FUN_009d4f9f           // _snprintf_s(buf1, 0x400, 0x3ff, fmt, tier)
        // 00015a33  buf1 is still at [ESP+0x424] (after the 5 arg pushes)
        lea  edx, [esp + 0x424]
        push 1
        push edx
        call FUN_004157c0           // emit header line with newline
        add  esp, 0x1c              // clean up 7 dwords from both calls

        // 00015a45  clamp tier to [0, 4]
        cmp  bl, 5
        jc   SHORT no_clamp
        mov  bl, 4
    no_clamp:
        // 00015a4c  compute view index: EAX = (tier+1)*5
        movzx eax, bl
        add  eax, 1
        lea  eax, [eax + eax*4]     // EAX *= 5

        // 00015a55  select the view slot; each view is 5 dwords = 20 bytes
        mov  ebx, dword ptr [ebp + eax*4 + 8]  // EBX = view.head_pos
        lea  esi, [ebp + eax*4]                 // ESI = &view (data_ptr base)

        // 00015a5d  loop: EDI = 0..view.tail_pos-1
        xor  edi, edi
        cmp  dword ptr [esi + 0xc], edi         // view.tail_pos == 0?
        jle  SHORT loop_done

    loop_body:
        // 00015a64  physical_idx = (i + head_pos) % chunks
        lea  eax, [edi + ebx]
        cdq
        idiv dword ptr [esi + 0x10]             // EDX = (i+head_pos) % chunks

        // 00015a6b  line_addr = physical_idx * m_cols + data_ptr
        push 1
        imul edx, dword ptr [ebp + 0x10]        // EDX *= m_cols
        add  edx, dword ptr [esi]               // EDX += data_ptr
        push edx
        call FUN_004157c0                       // emit line with newline

        add  edi, 1
        add  esp, 8
        cmp  edi, dword ptr [esi + 0xc]         // i < view.tail_pos?
        jl   SHORT loop_body

    loop_done:
        // 00015a84  format and emit the footer/separator line
        push 0xf575a0               // format string (.rdata)
        push 0x3ff
        lea  ecx, [esp + 0x18]      // buf2 = frame_base - 0x800
        push 0x400
        push ecx
        // 00015a98  null-terminate last byte of buf2 before snprintf
        mov  byte ptr [esp + 0x41f], 0
        call FUN_009d4f9f
        lea  edx, [esp + 0x20]      // edx = buf2
        push 2
        push edx
        call dword ptr [ext_12651b4]  // IAT: dispatch with level 2

        add  esp, 0x18
        pop  edi
        pop  esi
        pop  ebp
        pop  ebx
        add  esp, 0x800
        ret  4
    }
}
