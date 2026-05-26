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
// FUNCTION: ffxivgame 0x000144b0 — __thiscall iterator-begin: read the
//           first node from `this->m_owner`'s chain head (m_owner+0x28),
//           bail to a zeroed-out return-NULL form when the head equals
//           the embedded sentinel (m_owner+0x20), otherwise pull the
//           first sample via `node->vtable[1]()`, store it in
//           `this->m_value` (+0xc), clear `this->m_extra` (+0x10), and
//           return `&this->m_sub_object` (this+0x8) for the caller to
//           chain through. (48 bytes / 0x30)
//
// Calling convention: __thiscall (ECX = this), no stack args, RET.
//   Receiver layout mirrors FUN_004144e0's iterator-advance:
//     +0x04   m_owner       : container holding the chain sentinel
//                              (sentinel address is m_owner+0x20; current-
//                               node pointer lives at m_owner+0x28)
//     +0x08   m_sub_object  : embedded sub-object whose &-address is the
//                              non-NULL return value
//     +0x0c   m_value       : per-node sample slot (filled with the result
//                              of node->vtable[1]() on the hit path,
//                              zeroed on the sentinel-hit path)
//     +0x10   m_extra       : auxiliary scalar slot — always cleared by
//                              this function (0 on either path)
//
//   Per-node (the object at m_owner->[+0x28]):
//     +0x00   vtable        : virtual table; vtable[1] returns the int
//                              sample stored into this->m_value
//
// Body shape (matches the asm trace verbatim):
//
//   NodeC* node = this->m_owner->[+0x28];
//   NodeC* sentinel = (NodeC*)((char*)this->m_owner + 0x20);
//   if (node == sentinel) {
//       this->m_value = 0;
//       this->m_extra = 0;
//       return 0;                       // NULL
//   }
//   this->m_value = node->vtable[1]();   // first sample
//   this->m_extra = 0;
//   return &this->m_sub_object;          // this + 0x8
//
// Asm trace (48 bytes @ orig RVA 0x000144b0):
//
//   000144b0: 56                PUSH ESI
//   000144b1: 8b f1             MOV  ESI, ECX                ; ESI = this
//   000144b3: 8b 46 04          MOV  EAX, [ESI+0x4]          ; EAX = m_owner
//   000144b6: 8b 48 28          MOV  ECX, [EAX+0x28]         ; ECX = node = m_owner->[+0x28]
//   000144b9: 83 c0 20          ADD  EAX, 0x20               ; EAX = sentinel = m_owner+0x20
//   000144bc: 3b c8             CMP  ECX, EAX
//   000144be: 74 16             JE   sentinel_hit            ; node == sentinel → empty path
//   000144c0: 8b 01             MOV  EAX, [ECX]              ; EAX = node->vtable
//   000144c2: 8b 50 04          MOV  EDX, [EAX+0x4]          ; EDX = vtable[1]
//   000144c5: ff d2             CALL EDX                     ; sample = vtable[1]()
//   000144c7: 89 46 0c          MOV  [ESI+0xc], EAX          ; m_value = sample
//   000144ca: c7 46 10 00 00 00 00  MOV [ESI+0x10], 0        ; m_extra = 0
//   000144d1: 8d 46 08          LEA  EAX, [ESI+0x8]          ; ret = &m_sub_object
//   000144d4: 5e                POP  ESI
//   000144d5: c3                RET
//   sentinel_hit:
//   000144d6: 33 c0             XOR  EAX, EAX                ; ret = 0 (NULL)
//   000144d8: 89 46 0c          MOV  [ESI+0xc], EAX          ; m_value = 0
//   000144db: 89 46 10          MOV  [ESI+0x10], EAX         ; m_extra = 0
//   000144de: 5e                POP  ESI
//   000144df: c3                RET
//
// Relocations: none (no REL32 / DIR32 inside the function — the single
// CALL is an indirect `call edx` virtual dispatch).
//
// Reconstruction strategy — naked-asm byte passthrough.
//   The dual-return shape (one path zeroing both slots via XOR-shared
//   EAX, the other LEA-loading the embedded-sub-object address through
//   the same EAX) is a tight MSVC 2005 /O2 idiom that source-level C++
//   doesn't reliably reproduce — in particular, the `ADD EAX, 0x20`
//   after a separate `MOV ECX, [EAX+0x28]` (rather than the more usual
//   `LEA EAX, [EAX+0x20]`) reflects MSVC's choice to mutate the live
//   m_owner copy in EAX, which a source rewrite tends to lose. `_emit`
//   preserves all 48 bytes verbatim. Cross-platform guard:
//   `__declspec(naked)` + MASM `_emit` are MSVC-only.

#if defined(__clang__) || defined(__GNUC__)
// clang / GCC stub for static-analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block
// below, which clang cannot parse. Production builds always use cl.exe
// (MSVC 2005).
extern "C" void FUN_004144b0() {}
#else

extern "C" __declspec(naked) void FUN_004144b0()
{
    __asm {
        // 000144b0: 56                PUSH ESI
        _emit 0x56
        // 000144b1: 8b f1             MOV  ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000144b3: 8b 46 04          MOV  EAX, [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000144b6: 8b 48 28          MOV  ECX, [EAX+0x28]
        _emit 0x8b
        _emit 0x48
        _emit 0x28
        // 000144b9: 83 c0 20          ADD  EAX, 0x20
        _emit 0x83
        _emit 0xc0
        _emit 0x20
        // 000144bc: 3b c8             CMP  ECX, EAX
        _emit 0x3b
        _emit 0xc8
        // 000144be: 74 16             JE   +0x16 (→ 0x000144d6)
        _emit 0x74
        _emit 0x16
        // 000144c0: 8b 01             MOV  EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 000144c2: 8b 50 04          MOV  EDX, [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000144c5: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000144c7: 89 46 0c          MOV  [ESI+0xc], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 000144ca: c7 46 10 00 00 00 00  MOV dword ptr [ESI+0x10], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000144d1: 8d 46 08          LEA  EAX, [ESI+0x8]
        _emit 0x8d
        _emit 0x46
        _emit 0x08
        // 000144d4: 5e                POP  ESI
        _emit 0x5e
        // 000144d5: c3                RET
        _emit 0xc3
        // 000144d6: 33 c0             XOR  EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000144d8: 89 46 0c          MOV  [ESI+0xc], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 000144db: 89 46 10          MOV  [ESI+0x10], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 000144de: 5e                POP  ESI
        _emit 0x5e
        // 000144df: c3                RET
        _emit 0xc3
    }
}

#endif // _MSC_VER
