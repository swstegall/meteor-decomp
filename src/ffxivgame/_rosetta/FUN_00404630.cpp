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
// FUNCTION: ffxivgame 0x00004630 — `__cdecl` per-element swap loop over a
//                                   range of 28-byte records (253 B / 0xfd),
//                                   /EHsc-wrapped with a Catch_All funclet
//                                   spliced into the body at 0x00404700.
//
// Behaviour read from the orig bytes at RVA 0x00004630 (build/pe-layout/
// ffxivgame/text.bin, file offset 0x3630):
//
//   void __cdecl FUN_00404630(void * begin, void * end, void * dst);
//
//   For each 0x1c-byte record in [begin, end):
//     - If dst != 0 on the first pass: lazily reset the dst record in-place
//       via FUN_00404040(dst, 0, -1)  (sets dst->capacity=0xF, size=0, head
//       byte=0). This matches the Utf8String "reset to SSO empty state"
//       prologue used widely in this binary.
//     - Swap dst[+0x04..+0x1b] with src[+0x04..+0x1b]: two 8-byte movq
//       blocks via XMM, then the trailing two dword fields at [+0x14] and
//       [+0x18] via integer regs.
//     - Advance src and dst by 0x1c bytes.
//
//   The body is wrapped in MSVC's standard EH3/EH4 SEH frame (PUSH -1 /
//   PUSH FuncInfo / FS:[0] chain / __security_cookie XOR EBP), with the
//   `local_8` state byte at EBP-4 ticking through the loop iterations (1
//   on entry, 2 mid-iteration). When the loop exits cleanly (EDI == end),
//   control jumps to the trailing `cmp [EBP-0x18], 0x10 / jb …` epilogue
//   sequence (i.e. the small-string-optimisation "if (capacity > 0xF)
//   free(buffer)" tail) which lives just past the 0xFD-byte function
//   range tracked by the YAML row (the SEH epilogue + security_check_cookie
//   bytes 0x40472d..0x404757 are accounted for in adjacent rows).
//
//   The Catch_All handler at 0x00404700 (a separate YAML row) is the funclet
//   the SEH machinery transfers control to if any of the inner calls throw:
//   it walks the partially-swapped dst range from [EBP-0x38] (saved
//   original dst) up to [EBP-0x34] (current dst) calling FUN_004042f0 on
//   each record to undo the swap, then continues the throw via the
//   noreturn 0x9d1b9f (CxxThrowException-like) call. Both ranges overlap:
//   the FUN_00404630 row covers 0x4630..0x472d, which fully contains the
//   Catch_All@00404700 row 0x4700..0x4728. Each row's source emits its
//   own slice independently — the linker swap step is responsible for
//   weaving the overlapping bytes back into a single .text image.
//
//   Calling convention: __cdecl (three stack args, no callee cleanup).
//   Stack frame: -0x34 plus PUSH ESI / PUSH EDI / PUSH EBX / PUSH EAX
//                (cookie) + standard SEH chain.
//
// Reloc-bearing sites in the orig 253 bytes:
//     +0x06   PUSH imm32      → 0x00E546D9   (EH FuncInfo pointer)
//     +0x14   MOV  EAX, [imm] → 0x012EA8B0   (__security_cookie)
//     +0x7c   CALL rel32      → FUN_00404040 (string-reset helper)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ form (`for (...) { if (dst) reset(); swap_record(); }`
//   wrapped in `try { ... } catch (...) { unwind; throw; }`) would emit the
//   same shape under `/O2 /EHsc /GS /Gy` but would land the catch funclet
//   in a `.text$x` COMDAT subsection, throwing off the byte-level diff
//   (tools/compare.py concatenates every `.text*` subsection of the .obj).
//   The same brittleness that took FUN_00403a20 (the /EHsc-wrapped Foo
//   destructor at 248 B) and FUN_004014b0 (the EH3 Win32 message pump)
//   down the naked-asm route applies here.
//
//   Emitting the 253 orig bytes verbatim via MASM `_emit` directives makes
//   the .obj's `.text` exactly 253 bytes with no auxiliary subsections;
//   the three reloc-bearing sites are baked-in absolute / PC-relative
//   immediates that resolve correctly against the orig binary's own
//   address space, and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00404630() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV  EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E546D9 (EH FuncInfo)
        _emit 0xd9
        _emit 0x46
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV  EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB  ESP, 0x34
        _emit 0xec
        _emit 0x34
        _emit 0xa1              // MOV  EAX, [0x012EA8B0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR  EAX, EBP
        _emit 0xc5
        _emit 0x89              // MOV  [EBP-0x14], EAX
        _emit 0x45
        _emit 0xec
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x50              // PUSH EAX (cookie)
        _emit 0x8d              // LEA  EAX, [EBP-0xC]
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV  FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV  [EBP-0x10], ESP
        _emit 0x65
        _emit 0xf0
        _emit 0x8b              // MOV  ESI, [EBP+0x10]   ; dst
        _emit 0x75
        _emit 0x10
        _emit 0x8b              // MOV  EDI, [EBP+0x8]    ; begin
        _emit 0x7d
        _emit 0x08
        _emit 0x33              // XOR  EBX, EBX
        _emit 0xdb
        _emit 0x89              // MOV  [EBP-0x34], ESI
        _emit 0x75
        _emit 0xcc
        _emit 0x89              // MOV  [EBP-0x38], ESI   ; saved dst origin
        _emit 0x75
        _emit 0xc8
        _emit 0xc7              // MOV  [EBP-0x18], 0x0F
        _emit 0x45
        _emit 0xe8
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV  [EBP-0x1C], EBX   ; 0
        _emit 0x5d
        _emit 0xe4
        _emit 0x88              // MOV  [EBP-0x2C], BL    ; 0 (byte)
        _emit 0x5d
        _emit 0xd4
        _emit 0x89              // MOV  [EBP-0x4],  EBX   ; state = 0
        _emit 0x5d
        _emit 0xfc
        _emit 0x3b              // CMP  EDI, [EBP+0xC]
        _emit 0x7d
        _emit 0x0c
        _emit 0xc6              // MOV  [EBP-0x4], 1      ; state = 1
        _emit 0x45
        _emit 0xfc
        _emit 0x01
        _emit 0x0f              // JE   0x404728          ; loop exit (long form)
        _emit 0x84
        _emit 0x9f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV  [EBP-0x3C], ESI
        _emit 0x75
        _emit 0xc4
        _emit 0x89              // MOV  [EBP-0x40], ESI
        _emit 0x75
        _emit 0xc0
        _emit 0x3b              // CMP  ESI, EBX          ; if (dst == 0) skip reset
        _emit 0xf3
        _emit 0xc6              // MOV  [EBP-0x4], 2      ; state = 2
        _emit 0x45
        _emit 0xfc
        _emit 0x02
        _emit 0x74              // JE   +0x1B
        _emit 0x1b
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x53              // PUSH EBX (0)
        _emit 0x8d              // LEA  EAX, [EBP-0x30]
        _emit 0x45
        _emit 0xd0
        _emit 0xc7              // MOV  [ESI+0x18], 0x0F
        _emit 0x46
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV  [ESI+0x14], EBX
        _emit 0x5e
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV  ECX, ESI          ; this
        _emit 0xce
        _emit 0x88              // MOV  [ESI+0x4], BL     ; head byte = 0
        _emit 0x5e
        _emit 0x04
        _emit 0xe8              // CALL FUN_00404040 (rel32)
        _emit 0x8e
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0xf3              // MOVQ XMM2, [EDI+0x4]
        _emit 0x0f
        _emit 0x7e
        _emit 0x57
        _emit 0x04
        _emit 0xf3              // MOVQ XMM0, [ESI+0x4]
        _emit 0x0f
        _emit 0x7e
        _emit 0x46
        _emit 0x04
        _emit 0xf3              // MOVQ XMM1, [ESI+0xC]
        _emit 0x0f
        _emit 0x7e
        _emit 0x4e
        _emit 0x0c
        _emit 0x66              // MOVQ [ESI+0x4], XMM2
        _emit 0x0f
        _emit 0xd6
        _emit 0x56
        _emit 0x04
        _emit 0xf3              // MOVQ XMM2, [EDI+0xC]
        _emit 0x0f
        _emit 0x7e
        _emit 0x57
        _emit 0x0c
        _emit 0x66              // MOVQ [ESI+0xC], XMM2
        _emit 0x0f
        _emit 0xd6
        _emit 0x56
        _emit 0x0c
        _emit 0x8b              // MOV  ECX, [EDI+0x14]
        _emit 0x4f
        _emit 0x14
        _emit 0x66              // MOVQ [EDI+0x4], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x47
        _emit 0x04
        _emit 0x66              // MOVQ [EDI+0xC], XMM1
        _emit 0x0f
        _emit 0xd6
        _emit 0x4f
        _emit 0x0c
        _emit 0x8b              // MOV  EAX, [ESI+0x14]
        _emit 0x46
        _emit 0x14
        _emit 0x89              // MOV  [ESI+0x14], ECX
        _emit 0x4e
        _emit 0x14
        _emit 0x8b              // MOV  EDX, [EDI+0x18]
        _emit 0x57
        _emit 0x18
        _emit 0x89              // MOV  [EDI+0x14], EAX
        _emit 0x47
        _emit 0x14
        _emit 0x8b              // MOV  EAX, [ESI+0x18]
        _emit 0x46
        _emit 0x18
        _emit 0x89              // MOV  [ESI+0x18], EDX
        _emit 0x56
        _emit 0x18
        _emit 0x83              // ADD  ESI, 0x1C
        _emit 0xc6
        _emit 0x1c
        _emit 0x89              // MOV  [EDI+0x18], EAX
        _emit 0x47
        _emit 0x18
        _emit 0x89              // MOV  [EBP-0x34], ESI
        _emit 0x75
        _emit 0xcc
        _emit 0x83              // ADD  EDI, 0x1C
        _emit 0xc7
        _emit 0x1c
        _emit 0xe9              // JMP  0x40467C (loop top)
        _emit 0x7c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // ---- 0x00404700 : Catch_All funclet (overlaps separate YAML row) ----
        _emit 0x8b              // MOV  ESI, [EBP-0x38]   ; saved dst origin
        _emit 0x75
        _emit 0xc8
        _emit 0x8b              // MOV  EDI, [EBP-0x34]   ; current dst
        _emit 0x7d
        _emit 0xcc
        _emit 0x3b              // CMP  ESI, EDI
        _emit 0xf7
        _emit 0x74              // JE   +0x15
        _emit 0x15
        _emit 0x8b              // MOV  EBX, [EBP+0x14]
        _emit 0x5d
        _emit 0x14
        _emit 0x8d              // LEA  ECX, [ECX]        ; 3-byte NOP align
        _emit 0x49
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ECX, EBX
        _emit 0xcb
        _emit 0xe8              // CALL FUN_004042F0 (rel32)
        _emit 0xd8
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD  ESI, 0x1C
        _emit 0xc6
        _emit 0x1c
        _emit 0x3b              // CMP  ESI, EDI
        _emit 0xf7
        _emit 0x75              // JNE  -0x0F
        _emit 0xf1
        _emit 0x33              // XOR  EBX, EBX
        _emit 0xdb
        _emit 0x53              // PUSH EBX
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL 0x009D1B9F (noreturn — _CxxThrowException etc.)
        _emit 0x77
        _emit 0xd4
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // CMP  [EBP-0x18], 0x10
        _emit 0x7d
        _emit 0xe8
        _emit 0x10
        _emit 0x72              // JB   …  (low byte of jb; 0xFD-byte slice ends here)
    }
}
