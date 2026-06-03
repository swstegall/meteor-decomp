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
// FUNCTION: ffxivgame 0x005f654b — SSE2 probe __try body + filter head (45 B / 0x2D)
//
// This is the first 45 bytes of a function called by __get_sse2_info
// (at 0x005f659b / VA 0x009f659b). It probes whether the CPU can execute
// SSE2 instructions by executing `MOVAPD XMM0, XMM1` inside a
// __try/__except SEH frame that catches EXCEPTION_ACCESS_VIOLATION
// (0xC0000005) and EXCEPTION_ILLEGAL_INSTRUCTION (0xC000001D).
//
// The Ghidra split identifies only 45 bytes — covering the __try body
// (prologue + SSE2 probe + jmp to epilogue at 0x5f658b) and the first
// two bytes of the exception-filter comparison (cmp eax, 0xC000001D).
// The remaining bytes of the function (the filter return logic,
// handler body, and epilogue) fall in the following YAML entries.
//
// Asm (45 bytes @ orig RVA 0x005f654b / VA 0x009f654b):
//   6a 0c              PUSH 0x0C                        ; __except frame: 3 try levels
//   68 68 d7 22 01     PUSH 0x122D768                   ; &_except_handler3 cookie table
//   e8 99 7f fe ff     CALL 0x005DE4F0                  ; __EH_prolog3 or SEH setup helper
//   83 65 fc 00        AND  dword ptr [EBP-4], 0        ; SEH scope index = -1 (init)
//   66 0f 28 c1        MOVAPD XMM0, XMM1               ; SSE2 probe — may raise exception
//   c7 45 e4 01 00 00 00  MOV dword ptr [EBP-0x1C], 1  ; success flag = 1
//   eb 23              JMP  0x005F658B                  ; jump to function epilogue
//   8b 45 ec           MOV  EAX, dword ptr [EBP-0x14]  ; filter: get EXCEPTION_POINTERS *
//   8b 00              MOV  EAX, dword ptr [EAX]        ; → EXCEPTION_RECORD *
//   8b 00              MOV  EAX, dword ptr [EAX]        ; → ExceptionCode
//   3d 05 00 00 c0     CMP  EAX, 0xC0000005            ; EXCEPTION_ACCESS_VIOLATION?
//   74 0a              JE   0x005F6580                  ; yes → handle
//   3d 1d              (first 2 bytes of CMP EAX, 0xC000001D)  ; cut by YAML split
//
// Calling convention: first chunk of a __cdecl function; no standalone CC.
// Frame: SEH frame set up via __EH_prolog3 call; this chunk has no RET.
//
// Reconstruction: __declspec(naked) _emit byte pass-through (same idiom as
// FUN_004051e0, FUN_00416320, etc.). The two relocatable sites — the PUSH
// imm32 (SEH handler table pointer, +0x02) and the CALL rel32 (+0x07) —
// are masked out of the byte diff by tools/compare.py. The remaining bytes
// are hard literals and must match exactly.

extern "C" __declspec(naked) void FUN_009f654b() {
    __asm {
        // 005f654b: 6a 0c              PUSH 0x0C
        _emit 0x6a
        _emit 0x0c
        // 005f654d: 68 68 d7 22 01     PUSH 0x122D768
        _emit 0x68
        _emit 0x68
        _emit 0xd7
        _emit 0x22
        _emit 0x01
        // 005f6552: e8 99 7f fe ff     CALL 0x005DE4F0
        _emit 0xe8
        _emit 0x99
        _emit 0x7f
        _emit 0xfe
        _emit 0xff
        // 005f6557: 83 65 fc 00        AND dword ptr [EBP-4], 0
        _emit 0x83
        _emit 0x65
        _emit 0xfc
        _emit 0x00
        // 005f655b: 66 0f 28 c1        MOVAPD XMM0, XMM1
        _emit 0x66
        _emit 0x0f
        _emit 0x28
        _emit 0xc1
        // 005f655f: c7 45 e4 01 00 00 00   MOV dword ptr [EBP-0x1C], 1
        _emit 0xc7
        _emit 0x45
        _emit 0xe4
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005f6566: eb 23              JMP +0x23 (to 0x005F658B)
        _emit 0xeb
        _emit 0x23
        // 005f6568: 8b 45 ec           MOV EAX, dword ptr [EBP-0x14]
        _emit 0x8b
        _emit 0x45
        _emit 0xec
        // 005f656b: 8b 00              MOV EAX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x00
        // 005f656d: 8b 00              MOV EAX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x00
        // 005f656f: 3d 05 00 00 c0     CMP EAX, 0xC0000005
        _emit 0x3d
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0xc0
        // 005f6574: 74 0a              JE +0x0A (to 0x005F6580)
        _emit 0x74
        _emit 0x0a
        // 005f6576: 3d 1d              (first 2 bytes of CMP EAX, 0xC000001D — YAML split)
        _emit 0x3d
        _emit 0x1d
    }
}
