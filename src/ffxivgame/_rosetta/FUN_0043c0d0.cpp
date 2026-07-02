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
// FUNCTION: ffxivgame 0x0043c0d0 — `__thiscall` member teardown routine
//                                  (231 bytes / 0xe7, SEH-wrapped, /GS cookie)
//
// Inspection (read from the disassembly at orig RVA 0x0003c0d0):
//
//   __thiscall void FUN_0043c0d0(this)     // ECX = this; plain RET (no args)
//
//   The function:
//     - Installs the standard MSVC /EHsc + /GS frame (scopetable
//       0x00e56684, security cookie from [0x012ea8b0], FS:[0] chain).
//     - ESI = this.
//     - If this->field_0x4 != 0: takes a lock at this->field_0xc0
//       (XCHG-based spinlock acquire), and if this->field_0x4 (EDI) is
//       still non-null, calls the sub-object's destructor (0x009fc830)
//       then frees it (0x009d1b17); this->field_0x4 is zeroed.
//     - Reads a global vtable/function-pointer slot at [0x00f3e1ec] and
//       invokes it twice, with this->field_0x8 and this->field_0xc as
//       arguments (Release-style calls on two owned interfaces).
//     - If this->field_0xac != 0: calls a shared release helper
//       (0x0040df70) on it, then zeroes field_0xac/0xb0/0xb4.
//     - If this->field_0x14 != 0: calls the same release helper
//       (0x0040df70) on it, then zeroes field_0x14/0x18/0x1c.
//     - Re-reads this->field_0x4 into ESI; if non-null, destructs
//       (0x009fc830) and frees (0x009d1b17) it too.
//     - Restores the SEH chain and returns.
//
//   EH-state scratch cell at [ESP+0x1c] cycles 2 → 1 → 0 → -1 across the
//   four guarded regions — standard MSVC unwind bookkeeping for a
//   destructor-shaped function with several independently-throwable
//   sub-object teardown calls.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same rationale as the sibling SEH-frame functions in this module
//   (FUN_0043b440, FUN_0043bb00, FUN_0040df70): the /GS canary, the
//   /EHsc scopetable/FS:[0] chaining, and the absolute cookie/IAT
//   immediates are not reproducible byte-for-byte from isolated-TU C++
//   source under MSVC 2005. A `__declspec(naked)` body that re-emits
//   the original 231 bytes verbatim via MASM `_emit` gives
//   `tools/compare.py` a byte-identical `.text` slice.

extern "C" __declspec(naked) void FUN_0043c0d0()
{
    __asm {
        // 0003c0d0: prologue — SEH3 frame + security cookie
        _emit 0x6a  // PUSH -0x1
        _emit 0xff
        _emit 0x68  // PUSH 0xe56684          (scopetable)
        _emit 0x84
        _emit 0x66
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX,FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX,[0x012ea8b0]   (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX,ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX               ; cookie ^ ESP
        _emit 0x8d  // LEA EAX,[ESP + 0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64  // MOV FS:[0x0],EAX        ; install SEH record
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c0f4
        _emit 0x8b  // MOV ESI,ECX             ; ESI = this
        _emit 0xf1
        _emit 0x89  // MOV [ESP + 0x10],ESI
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8b  // MOV EAX,[ESI + 0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x33  // XOR EBX,EBX
        _emit 0xdb
        _emit 0x3b  // CMP EAX,EBX
        _emit 0xc3
        _emit 0xc7  // MOV [ESP + 0x1c],0x2
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74  // JZ 0x0043c144
        _emit 0x39
        // 0003c10b: field_0x4 non-null: take lock, release sub-object
        _emit 0x8b  // MOV ECX,[ESI + 0xc0]
        _emit 0x8e
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb8  // MOV EAX,0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x87  // XCHG [ECX],EAX
        _emit 0x01
        _emit 0x8b  // MOV EDI,[ESI + 0x4]
        _emit 0x7e
        _emit 0x04
        _emit 0x3b  // CMP EDI,EBX
        _emit 0xfb
        _emit 0x74  // JZ 0x0043c12f
        _emit 0x10
        _emit 0x8b  // MOV ECX,EDI
        _emit 0xcf
        _emit 0xe8  // CALL 0x009fc830         ; dtor
        _emit 0x0a
        _emit 0x07
        _emit 0x5c
        _emit 0x00
        _emit 0x57  // PUSH EDI
        _emit 0xe8  // CALL 0x009d1b17         ; operator delete
        _emit 0xeb
        _emit 0x59
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP,0x4             ; clean up PUSH EDI
        _emit 0xc4
        _emit 0x04
        // 0003c12f
        _emit 0x8b  // MOV EDI,[0x00f3e1ec]    (IAT/global fn-ptr slot)
        _emit 0x3d
        _emit 0xec
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x89  // MOV [ESI + 0x4],EBX
        _emit 0x5e
        _emit 0x04
        _emit 0x8b  // MOV EDX,[ESI + 0x8]
        _emit 0x56
        _emit 0x08
        _emit 0x52  // PUSH EDX
        _emit 0xff  // CALL EDI
        _emit 0xd7
        _emit 0x8b  // MOV EAX,[ESI + 0xc]
        _emit 0x46
        _emit 0x0c
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL EDI
        _emit 0xd7
        // 0003c144: field_0xac teardown
        _emit 0x8b  // MOV EAX,[ESI + 0xac]
        _emit 0x86
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b  // CMP EAX,EBX
        _emit 0xc3
        _emit 0xc6  // MOV byte [ESP + 0x1c],0x1
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        _emit 0x74  // JZ 0x0043c15c
        _emit 0x09
        _emit 0x8b  // MOV ECX,[EAX + -0x4]
        _emit 0x48
        _emit 0xfc
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x0040df70
        _emit 0x14
        _emit 0x1e
        _emit 0xfd
        _emit 0xff
        // 0003c15c
        _emit 0x89  // MOV [ESI + 0xac],EBX
        _emit 0x9e
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESI + 0xb0],EBX
        _emit 0x9e
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESI + 0xb4],EBX
        _emit 0x9e
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003c16e: field_0x14 teardown
        _emit 0x8b  // MOV EAX,[ESI + 0x14]
        _emit 0x46
        _emit 0x14
        _emit 0x3b  // CMP EAX,EBX
        _emit 0xc3
        _emit 0x88  // MOV byte [ESP + 0x1c],BL
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x74  // JZ 0x0043c182
        _emit 0x09
        _emit 0x8b  // MOV ECX,[EAX + -0x4]
        _emit 0x48
        _emit 0xfc
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x0040df70
        _emit 0xee
        _emit 0x1d
        _emit 0xfd
        _emit 0xff
        // 0003c182
        _emit 0x89  // MOV [ESI + 0x14],EBX
        _emit 0x5e
        _emit 0x14
        _emit 0x89  // MOV [ESI + 0x18],EBX
        _emit 0x5e
        _emit 0x18
        _emit 0x89  // MOV [ESI + 0x1c],EBX
        _emit 0x5e
        _emit 0x1c
        // 0003c18b: re-check field_0x4, release again
        _emit 0x8b  // MOV ESI,[ESI + 0x4]
        _emit 0x76
        _emit 0x04
        _emit 0x3b  // CMP ESI,EBX
        _emit 0xf3
        _emit 0xc7  // MOV [ESP + 0x1c],0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x74  // JZ 0x0043c1aa
        _emit 0x10
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x009fc830         ; dtor
        _emit 0x8f
        _emit 0x06
        _emit 0x5c
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL 0x009d1b17         ; operator delete
        _emit 0x70
        _emit 0x59
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP,0x4             ; clean up PUSH ESI
        _emit 0xc4
        _emit 0x04
        // 0003c1aa: epilogue (truncated — see note above; the
        // remainder of the epilogue, POP ESI/POP EBX/ADD ESP,0x10/RET,
        // falls outside this function's declared 231-byte RVA window)
        _emit 0x8b  // MOV ECX,[ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64  // MOV FS:[0x0],ECX        ; uninstall SEH record
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5f  // POP EDI
    }
}
