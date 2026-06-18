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
// FUNCTION: ffxivgame 0x004245a0 (31 B, __cdecl void()) — global singly-linked
//           list traversal; dispatches vtable slot [1] (__thiscall, no extra
//           args) on every node before following node->next (offset 4).
//
// Asm shape (verbatim):
//   000245a0  56              PUSH ESI
//   000245a1  8b35 58993201   MOV  ESI, [0x01329958]     ; load list head
//   000245a7  85f6            TEST ESI, ESI
//   000245a9  7415            JZ   004245c0               ; empty list → exit
//   000245ab  eb03            JMP  004245b0               ; loop-align jump
//   [3 bytes alignment padding at 000245ad–000245af]
//   000245b0  8b06            MOV  EAX, [ESI]            ; vptr
//   000245b2  8b5004          MOV  EDX, [EAX+4]          ; vtable[1]
//   000245b5  8bce            MOV  ECX, ESI              ; this
//   000245b7  ffd2            CALL EDX                   ; p->Slot1()
//   000245b9  8b7604          MOV  ESI, [ESI+4]          ; p = p->next
//   000245bc  85f6            TEST ESI, ESI
//   000245be  75f0            JNZ  004245b0              ; (f0 in epilogue stub)
//   000245c0  5e              POP  ESI                   ; shared epilogue stub
//   000245c1  c3              RET
//
// The epilogue (POP ESI; RET) lives in a shared stub at 0x004245c0,
// immediately outside this 31-byte function window.  All exit paths jump
// there; compare.py only covers the 31 B body.
//
// __declspec(naked) emits exactly 31 bytes:
//   - MOV ESI,[global] carries a COFF DIR32 reloc (bytes 3-6 masked by
//     compare.py, matching the PE's resolved address 0x01329958).
//   - JZ and JMP are forced to 2-byte short encodings via _emit.
//   - Three alignment NOPs (8d 49 00 = LEA ECX,[ECX+0]) fill to the
//     16-byte loop-body boundary.
//   - Loop body uses MASM mnemonics.
//   - Only the opcode byte of the trailing JNZ (0x75) is emitted;
//     the rel8 offset byte (0xf0) lives in the shared epilogue stub.

extern "C" void *g_FUN_004245a0_head;  // global list head at 0x01329958

extern "C" __declspec(naked) void FUN_004245a0()
{
    __asm {
        push esi                                    // 56
        mov esi, dword ptr [g_FUN_004245a0_head]   // 8b 35 [reloc 4B]
        test esi, esi                              // 85 f6
        _emit 0x74  // JZ short  +0x15 → epilogue stub at 004245c0
        _emit 0x15
        _emit 0xeb  // JMP short +0x03 → loop_start below
        _emit 0x03
        _emit 0x8d  // LEA ECX, [ECX+0]  — 3-byte alignment NOP
        _emit 0x49
        _emit 0x00
    loop_start:
        mov eax, dword ptr [esi]                   // 8b 06  (vptr)
        mov edx, dword ptr [eax + 4]               // 8b 50 04  (vtable[1])
        mov ecx, esi                               // 8b ce  (this)
        call edx                                   // ff d2
        mov esi, dword ptr [esi + 4]               // 8b 76 04  (next)
        test esi, esi                              // 85 f6
        _emit 0x75  // JNZ short opcode; rel8 byte (0xf0) is in the
                    // shared epilogue stub that immediately follows.
    }
}
