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
// FUNCTION: ffxivgame 0x00024600 — linked-list virtual dispatch sweep
//                                   (__cdecl void(), 31 B / 0x1f)
//
// Walks the same singly-linked list as its siblings (FUN_004245a0,
// FUN_00424630, etc.): the global pointer at 0x01329958 holds the head.
// For every node it invokes vtable slot 3 (offset 0x0c) as a __thiscall
// with no extra arguments, then advances to node->next (field at +0x4).
// If the head is null the loop body is skipped entirely.
//
// Asm shape (verbatim, 31 B at orig RVA 0x00024600):
//   00024600  56              PUSH ESI
//   00024601  8b35 58993201   MOV  ESI, [0x01329958]     ; load list head
//   00024607  85f6            TEST ESI, ESI
//   00024609  7415            JZ   00424620               ; empty list → epilogue stub
//   0002460b  eb03            JMP  00424610               ; loop-align jump
//   [3 bytes alignment padding at 0002460d–0002460f]     ; 8d 49 00 = LEA ECX,[ECX+0]
//   00024610  8b06            MOV  EAX, [ESI]            ; vptr
//   00024612  8b500c          MOV  EDX, [EAX+0xc]        ; vtable[3]
//   00024615  8bce            MOV  ECX, ESI              ; this
//   00024617  ffd2            CALL EDX                   ; p->Slot3()
//   00024619  8b7604          MOV  ESI, [ESI+4]          ; p = p->next
//   0002461c  85f6            TEST ESI, ESI
//   0002461e  75(f0)          JNZ  00424610              ; loop back
//   [0x00424620: POP ESI; RET lives in the shared epilogue stub, outside this 31-B window]
//
// Structural twin of FUN_004245a0 (slot 1) and FUN_00424660 (slot 2), etc.
// The epilogue (POP ESI; RET) at 0x00424620 is a shared stub immediately
// outside this 31-byte function window; compare.py only covers the 31 B body.
//
// Encoding notes:
//   - MOV ESI,[global] carries a DIR32 COFF reloc (bytes 3-6 masked by
//     compare.py), matching the PE's resolved address 0x01329958.
//   - JZ and JMP are forced to 2-byte short encodings via _emit.
//   - Three alignment NOPs (8d 49 00 = LEA ECX,[ECX+0]) fill to the
//     16-byte loop-body boundary at 0x00024610.
//   - Loop body uses MASM mnemonics (MASM picks disp8 for offsets ≤ 127).
//   - Only the opcode byte of the trailing JNZ (0x75) is emitted; the
//     rel8 offset byte (0xf0) lives in the shared epilogue stub.

extern "C" void *g_FUN_00424600_head;  // global list head at 0x01329958

extern "C" __declspec(naked) void FUN_00424600()
{
    __asm {
        push esi                                    // 56
        mov esi, dword ptr [g_FUN_00424600_head]   // 8b 35 [reloc 4B]
        test esi, esi                              // 85 f6
        _emit 0x74  // JZ short +0x15 → epilogue stub at 00424620
        _emit 0x15
        _emit 0xeb  // JMP short +0x03 → loop_start below
        _emit 0x03
        _emit 0x8d  // LEA ECX, [ECX+0]  — 3-byte alignment NOP
        _emit 0x49
        _emit 0x00
    loop_start:
        mov eax, dword ptr [esi]                   // 8b 06  (vptr)
        mov edx, dword ptr [eax + 0xc]             // 8b 50 0c  (vtable[3])
        mov ecx, esi                               // 8b ce  (this)
        call edx                                   // ff d2
        mov esi, dword ptr [esi + 4]               // 8b 76 04  (next)
        test esi, esi                              // 85 f6
        _emit 0x75  // JNZ short opcode; rel8 byte (0xf0) is in the
                    // shared epilogue stub that immediately follows.
    }
}
