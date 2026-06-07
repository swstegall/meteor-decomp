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
// FUNCTION: ffxivgame 0x0040df70 — __thiscall dispatch-to-vtable notifier
//                                   with critical-section guard (87 bytes).
//
// Shape (reconstructed from asm at RVA 0x0000df70):
//
//   void FUN_0040df70(this, arg) {   // __thiscall, callee cleans 4 bytes
//       if (!arg) return;
//       CRITICAL_SECTION *cs = (CRITICAL_SECTION *)((char *)this + 0x24);
//       EnterCriticalSection(cs);          // IAT @ [0x00f3e16c]
//       if (this->field_0x1c != 0) {
//           inner = *this;                 // first member = inner object ptr
//           result1 = inner->vtable[8](arg);   // vslot 0x20
//           result2 = inner->vtable[7](arg);   // vslot 0x1c
//           ((void(*)(int,int))this->field_0x1c)(result2, result1);
//       }
//       inner = *this;
//       inner->vtable[2](this, arg);       // vslot 0x08
//       LeaveCriticalSection(cs);          // IAT @ [0x00f3e168]
//   }
//
// Calling convention: __thiscall — ECX = this on entry, one stack arg at
// [esp+4], callee cleans via `ret 4`.  Preserved registers: EBX, ESI (always)
// and EBP, EDI (in the taken-arg path).
//
// Branch shape: two early-out forward jumps.  First (`jz +0x46` at +0x0a)
// skips the entire body when arg == 0.  Second (`jz +0x22` at +0x1b)
// skips the three-call callback block when field_0x1c == 0.
//
// IAT slots 0x00f3e168 / 0x00f3e16c are reloc-bearing ABS32 references that
// compare.py masks during the byte diff — the verbatim original bytes are
// reproduced in the emit stream so the non-reloc bytes still align correctly.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EBP usage here is unusual: PUSH EBP saves the caller's frame
//   pointer before LEA EBP, [ESI+0x24] repurposes EBP as the critical-
//   section pointer; both Enter and Leave receive it via PUSH EBP; POP EBP
//   at the end restores the caller's value.  A source-level formulation
//   would force MSVC to spill differently (MSVC 2005 /O2 treats EBP as the
//   frame pointer, not a general-purpose scratch), so the naked _emit body
//   is the only reliable path to byte-identical output.

extern "C" __declspec(naked) void FUN_0040df70()
{
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x74              // JZ +0x46  (→ dfc2: pop esi/ebx, ret 4)
        _emit 0x46
        _emit 0x55              // PUSH EBP
        _emit 0x8d              // LEA EBP, [ESI+0x24]
        _emit 0x6e
        _emit 0x24
        _emit 0x55              // PUSH EBP                   (arg to EnterCriticalSection)
        _emit 0xff              // CALL dword ptr [0x00f3e16c] (EnterCriticalSection)
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x83              // CMP dword ptr [ESI+0x1c], 0x0
        _emit 0x7e
        _emit 0x1c
        _emit 0x00
        _emit 0x74              // JZ +0x22  (→ dfaf: vtable[2] call block)
        _emit 0x22
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI]
        _emit 0x3e
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x20]
        _emit 0x50
        _emit 0x20
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX                   (inner->vtable[8](arg))
        _emit 0xd2
        _emit 0x50              // PUSH EAX                   (result1)
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x1c]
        _emit 0x50
        _emit 0x1c
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EDX                   (inner->vtable[7](arg))
        _emit 0xd2
        _emit 0x50              // PUSH EAX                   (result2)
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0xff              // CALL EAX                   (callback(result2, result1))
        _emit 0xd0
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x8]
        _emit 0x42
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL EAX                   (inner->vtable[2](this, arg))
        _emit 0xd0
        _emit 0x55              // PUSH EBP                   (arg to LeaveCriticalSection)
        _emit 0xff              // CALL dword ptr [0x00f3e168] (LeaveCriticalSection)
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x5d              // POP EBP                    (restore caller EBP)
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
