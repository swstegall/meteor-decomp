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
// FUNCTION: ffxivgame 0x00011c20 — drain a sentinel-guarded linked list,
//           releasing each node via virtual dispatch (64 B with shared epilogue)
//
// __thiscall void FUN_00411c20(this)
//   ECX : this — owner object
//
// Layout of `this`:
//   [+0x04] m_field4      — passed as ECX to FUN_0040df70 (the allocator/pool context)
//   [+0x18] m_size        — decremented by result->field_10 for each released node
//   [+0x24] m_sentinel    — embedded list sentinel node (sentinel address = this+0x24)
//   [+0x2c] m_head        — pointer to first node; equals &m_sentinel when list is empty
//
// Behaviour:
//   If the list is empty (m_head == &m_sentinel), return immediately.
//   Otherwise, for each node `cur` in the list until the sentinel is reached:
//     1. Call cur->vtable[1](cur) — __thiscall, no extra stack args — to obtain
//        a resource descriptor `result` (returned in EAX/EDI).
//     2. Advance `cur` to cur->next (cur->field_8).
//     3. this->m_size -= result->field_10.
//     4. Call result->vtable[0](result, 0) — __thiscall with one stack arg (0),
//        callee cleans via RET 4.
//     5. Call FUN_0040df70(result) — __stdcall with ECX=this->m_field4, 1 arg,
//        callee cleans via RET 4.
//     Continue until `cur` reaches `&m_sentinel`.
//
// Register allocation (MSVC 2005 /O2):
//   EBX = this           (callee-save; set from ECX in prologue)
//   ESI = cur node ptr   (callee-save; advanced each iteration via MOV ESI,[ESI+8])
//   EBP = &m_sentinel    (callee-save; set once by LEA EBP,[EBX+0x24] before loop)
//   EDI = result ptr     (callee-save; loaded from EAX after vtable[1] call)
//
// Stack frame (64-byte window):
//   Prologue: PUSH EBX / PUSH EBP / PUSH ESI  — 3 dwords
//   After empty-check: PUSH EDI               — 4 dwords total (only if non-empty)
//   Loop body: each of the three calls (vtable[1] / vtable[0] / FUN_0040df70) is
//   balanced by the callee's RET 4 or RET 0, so the net stack depth inside the
//   loop stays at 4 dwords. EDI is restored by POP EDI after the loop exits.
//   Epilogue: POP ESI / POP EBP / POP EBX / RET
//
// Note: the `.s` asm dump shipped with the function omits the 5-byte
//   loop-tail at 0x11c57–0x11c5b (CMP ESI,EBP / JNZ -0x2b / POP EDI)
//   that lies between the CALL FUN_0040df70 and the shared POP-epilogue.
//   These bytes are present in the binary (confirmed by compare.py size
//   override: 64 B vs symbols.json's 59 B) and are included below.
//
// Calling convention: __thiscall, no stack args, RET 0.
//
// Reloc-bearing site (rel32 masked by compare.py):
//   func offset 0x32 (abs 0x11c52): CALL FUN_0040df70
//
// Reconstruction strategy: naked-asm _emit passthrough; MASM `call`
//   mnemonic at the reloc site so the assembler emits a COFF REL32.

extern "C" void __stdcall FUN_0040df70(void *param_1);

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" void FUN_00411c20() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00411c20()
{
    __asm {
        // 00011c20: 53             PUSH EBX
        _emit 0x53
        // 00011c21: 55             PUSH EBP
        _emit 0x55
        // 00011c22: 8b d9          MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00011c24: 56             PUSH ESI
        _emit 0x56
        // 00011c25: 8b 73 2c       MOV ESI,dword ptr [EBX+0x2c]  ; cur = this->m_head
        _emit 0x8b
        _emit 0x73
        _emit 0x2c
        // 00011c28: 8d 6b 24       LEA EBP,[EBX+0x24]             ; sentinel = &m_sentinel
        _emit 0x8d
        _emit 0x6b
        _emit 0x24
        // 00011c2b: 3b f5          CMP ESI,EBP
        _emit 0x3b
        _emit 0xf5
        // 00011c2d: 74 2d          JZ  +0x2d  → 0x11c5c (POP ESI, empty-list exit)
        _emit 0x74
        _emit 0x2d
        // 00011c2f: 57             PUSH EDI   (callee-save; restored by POP EDI at 0x11c5b)
        _emit 0x57
        // --- loop body (target of JNZ at 0x11c59) ---
        // 00011c30: 8b 06          MOV EAX,dword ptr [ESI]        ; EAX = cur->vtable
        _emit 0x8b
        _emit 0x06
        // 00011c32: 8b 50 04       MOV EDX,dword ptr [EAX+0x4]   ; EDX = vtable[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00011c35: 8b ce          MOV ECX,ESI                    ; ECX = cur (thiscall)
        _emit 0x8b
        _emit 0xce
        // 00011c37: ff d2          CALL EDX   ; cur->vtable[1](cur) → EAX = result
        _emit 0xff
        _emit 0xd2
        // 00011c39: 8b 76 08       MOV ESI,dword ptr [ESI+0x8]   ; cur = cur->next
        _emit 0x8b
        _emit 0x76
        _emit 0x08
        // 00011c3c: 8b f8          MOV EDI,EAX                    ; EDI = result
        _emit 0x8b
        _emit 0xf8
        // 00011c3e: 8b 47 10       MOV EAX,dword ptr [EDI+0x10]  ; EAX = result->field_10
        _emit 0x8b
        _emit 0x47
        _emit 0x10
        // 00011c41: 29 43 18       SUB dword ptr [EBX+0x18],EAX  ; this->m_size -= field_10
        _emit 0x29
        _emit 0x43
        _emit 0x18
        // 00011c44: 8b 17          MOV EDX,dword ptr [EDI]        ; EDX = result->vtable
        _emit 0x8b
        _emit 0x17
        // 00011c46: 8b 02          MOV EAX,dword ptr [EDX]        ; EAX = vtable[0]
        _emit 0x8b
        _emit 0x02
        // 00011c48: 6a 00          PUSH 0x0   (arg for vtable[0]; callee cleans via RET 4)
        _emit 0x6a
        _emit 0x00
        // 00011c4a: 8b cf          MOV ECX,EDI                    ; ECX = result (thiscall)
        _emit 0x8b
        _emit 0xcf
        // 00011c4c: ff d0          CALL EAX   ; result->vtable[0](result, 0)
        _emit 0xff
        _emit 0xd0
        // 00011c4e: 8b 4b 04       MOV ECX,dword ptr [EBX+0x4]   ; ECX = this->m_field4
        _emit 0x8b
        _emit 0x4b
        _emit 0x04
        // 00011c51: 57             PUSH EDI   (arg = result; callee cleans via RET 4)
        _emit 0x57
        // 00011c52: e8 ...         CALL FUN_0040df70 (rel32 reloc, masked by compare.py)
        call    FUN_0040df70
        // 00011c57: 3b f5          CMP ESI,EBP  ; cur == sentinel?
        _emit 0x3b
        _emit 0xf5
        // 00011c59: 75 d5          JNZ -0x2b  → 0x11c30 (loop back)
        _emit 0x75
        _emit 0xd5
        // 00011c5b: 5f             POP EDI   (restore callee-save EDI)
        _emit 0x5f
        // 00011c5c: 5e             POP ESI
        _emit 0x5e
        // 00011c5d: 5d             POP EBP
        _emit 0x5d
        // 00011c5e: 5b             POP EBX
        _emit 0x5b
        // 00011c5f: c3             RET
        _emit 0xc3
    }
}
#endif
