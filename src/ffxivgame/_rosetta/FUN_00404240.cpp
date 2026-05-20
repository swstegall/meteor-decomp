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
// FUNCTION: ffxivgame 0x4240 — __stdcall range-iterator that invokes a
// __thiscall helper on each 0x54-byte element (35B)
//
// Iterates the half-open range [start, end) of objects of size 0x54
// bytes. For each element it sets ECX to the current element pointer
// and calls the __thiscall helper at FUN_00446f50 (typically a
// per-element destructor / cleanup hook), then advances the cursor by
// 0x54 bytes. The early-out at entry skips the loop entirely when the
// range is empty (start == end). The `mov edi, edi` two-byte filler is
// MSVC's 16-byte alignment padding for the loop-header (the loop body
// lands at offset 0x10).
//
// Calling convention: __stdcall (two stack args, callee pops via
// `ret 8`). The callee at 0x46f50 is __thiscall (this in ECX), which
// is why the iteration cursor lives in ESI and is copied into ECX on
// every iteration.
//
// Asm (35 bytes @ orig RVA 0x00004240):
//   56                  PUSH ESI
//   8b 74 24 08         MOV  ESI, DWORD PTR [ESP + 0x08]   ; start
//   57                  PUSH EDI
//   8b 7c 24 10         MOV  EDI, DWORD PTR [ESP + 0x10]   ; end
//   3b f7               CMP  ESI, EDI
//   74 10               JE   done
//   8b ff               MOV  EDI, EDI                      ; align pad
// loop:
//   8b ce               MOV  ECX, ESI                      ; this = cursor
//   e8 RR RR RR RR      CALL FUN_00446f50                  ; __thiscall
//   83 c6 54            ADD  ESI, 0x54
//   3b f7               CMP  ESI, EDI
//   75 f2               JNE  loop
// done:
//   5f                  POP  EDI
//   5e                  POP  ESI
//   c2 08 00            RET  8
//
// Encoded as `__declspec(naked)` inline asm so the .obj's `.text` is
// exactly 35 bytes matching orig; the CALL rel32 is the only reloc
// and `tools/compare.py` masks reloc bytes from the diff.

extern "C" void element_cleanup_helper();

extern "C" __declspec(naked) void FUN_00404240() {
    __asm {
        push esi
        mov esi, dword ptr [esp + 8]
        push edi
        mov edi, dword ptr [esp + 0x10]
        cmp esi, edi
        je done
        mov edi, edi
    loop_head:
        mov ecx, esi
        call element_cleanup_helper
        add esi, 0x54
        cmp esi, edi
        jne loop_head
    done:
        pop edi
        pop esi
        ret 8
    }
}
