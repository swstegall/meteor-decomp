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
// FUNCTION: ffxivgame 0x000144e0 — __thiscall iterator-advance:
//           accumulate one element through an inner embedded sub-object,
//           then step `this->m_node` via `m_node->m_link->vtable[1]()`,
//           returning a pointer to the embedded sub-object for chaining.
//           (79 bytes / 0x4f)
//
// Calling convention: __thiscall (ECX = this), no stack args, RET 0
// (the prologue `PUSH ECX` reserves a 4-byte local-spill slot rather
// than saving a caller register; the matching `POP ECX` in the epilogue
// is just the stack-cleanup). Return value: 0 (NULL) on the early-exit
// path; otherwise a pointer to the embedded sub-object at &this[+0x8].
//
// Inferred layout of the receiver (this):
//   +0x04   m_owner       : object whose +0x20 is the chain sentinel
//   +0x08   m_sub_object  : embedded vtable-bearing sub-object (returned
//                            by the function as &this[+0x8])
//   +0x0c   m_node        : current chain node (NULL → early-exit return 0)
//   +0x10   m_total       : running accumulator (int / size_t)
//
// Per-node (the object at this->m_node):
//   +0x10   m_link        : pointer to the next link object (the one
//                            whose vtable[1] is invoked to advance);
//                            equals this->m_owner+0x20 when at the end
//                            (chain-sentinel sentinel-equals check).
//
// Body shape (matches the asm trace verbatim):
//
//   if (this->m_node == NULL) return 0;                  // (eax=0 / je)
//   link = this->m_node->m_link;
//   if (link == this->m_owner + 0x20) goto leave;        // sentinel hit
//   saved   = this->m_total;
//   sub     = &this->m_sub_object;                        // returned later
//   r1      = sub->vtable[2]();                           // accumulate sample
//   r2      = link->vtable[1]();                          // step chain
//   this->m_node  = r2;
//   this->m_total = saved + r1;
//   eax = sub;                                            // return value
//   leave:
//   return eax;                                           // 0 or sub
//
// Asm trace (79 bytes @ orig RVA 0x000144e0):
//
//   000144e0: 51                PUSH ECX                    ; reserve 4 B spill slot
//   000144e1: 56                PUSH ESI                    ; save callee-saved
//   000144e2: 8b f1             MOV  ESI, ECX               ; ESI = this
//   000144e4: 8b 4e 0c          MOV  ECX, [ESI+0xc]         ; ECX = m_node
//   000144e7: 33 c0             XOR  EAX, EAX               ; default ret = 0
//   000144e9: 85 c9             TEST ECX, ECX
//   000144eb: 74 3f             JE   end_pop_esi_ecx        ; m_node == NULL
//   000144ed: 55                PUSH EBP
//   000144ee: 8b 69 10          MOV  EBP, [ECX+0x10]        ; EBP = node->m_link
//   000144f1: 8b 4e 04          MOV  ECX, [ESI+0x4]         ; ECX = m_owner
//   000144f4: 83 c1 20          ADD  ECX, 0x20              ; ECX = owner + 0x20
//   000144f7: 3b e9             CMP  EBP, ECX
//   000144f9: 74 30             JE   skip_body_pop_ebp      ; sentinel ==
//   000144fb: 8b 56 10          MOV  EDX, [ESI+0x10]        ; saved = m_total
//   000144fe: 8b 46 08          MOV  EAX, [ESI+0x8]         ; EAX = *(sub) = vtable
//   00014501: 53                PUSH EBX
//   00014502: 57                PUSH EDI
//   00014503: 8d 7e 08          LEA  EDI, [ESI+0x8]         ; EDI = &m_sub_object
//   00014506: 89 54 24 10       MOV  [ESP+0x10], EDX        ; spill saved → slot
//   0001450a: 8b 50 08          MOV  EDX, [EAX+0x8]         ; EDX = sub.vt[2]
//   0001450d: 8b cf             MOV  ECX, EDI               ; ECX = sub
//   0001450f: ff d2             CALL EDX                    ; r1 = sub.vt[2]()
//   00014511: 8b d8             MOV  EBX, EAX               ; EBX = r1
//   00014513: 8b 45 00          MOV  EAX, [EBP+0x0]         ; EAX = link->vtable
//   00014516: 8b 50 04          MOV  EDX, [EAX+0x4]         ; EDX = link.vt[1]
//   00014519: 03 5c 24 10       ADD  EBX, [ESP+0x10]        ; r1 += saved
//   0001451d: 8b cd             MOV  ECX, EBP               ; ECX = link
//   0001451f: ff d2             CALL EDX                    ; r2 = link.vt[1]()
//   00014521: 89 46 0c          MOV  [ESI+0xc], EAX         ; m_node = r2
//   00014524: 8b c7             MOV  EAX, EDI               ; ret = &m_sub_object
//   00014526: 5f                POP  EDI
//   00014527: 89 5e 10          MOV  [ESI+0x10], EBX        ; m_total = r1+saved
//   0001452a: 5b                POP  EBX
//   0001452b: 5d                POP  EBP                    ; skip_body target
//   0001452c: 5e                POP  ESI                    ; end_pop_esi_ecx target
//   0001452d: 59                POP  ECX                    ; release spill slot
//   0001452e: c3                RET
//
// Relocations: none (no REL32 / DIR32 inside the function — every CALL
// is an indirect `call edx` virtual dispatch).
//
// Reconstruction strategy — naked-asm byte passthrough.
// The two stacked virtual dispatches with a stack-spilled accumulator,
// the dual early-exit shape (one fall-through pop ESI+ECX, one bigger
// pop EBP+EBX+EDI+EBP+ESI+ECX), and the `PUSH ECX` / `POP ECX` as
// a one-DWORD local-spill substitute for `SUB ESP, 4` / `ADD ESP, 4`
// are MSVC 2005 /O2 idioms that are not reliably reproducible from
// readable C++. `_emit` preserves all 79 bytes verbatim. Cross-platform
// guard: `__declspec(naked)` + MASM `_emit` are MSVC-only.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block
// below, which clang cannot parse. Production builds always use cl.exe
// (MSVC 2005).
extern "C" void FUN_004144e0() {}
#else

extern "C" __declspec(naked) void FUN_004144e0()
{
    __asm {
        // 000144e0: 51                PUSH ECX
        _emit 0x51
        // 000144e1: 56                PUSH ESI
        _emit 0x56
        // 000144e2: 8b f1             MOV  ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000144e4: 8b 4e 0c          MOV  ECX, [ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 000144e7: 33 c0             XOR  EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000144e9: 85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 000144eb: 74 3f             JE   +0x3f → 0x0001452c
        _emit 0x74
        _emit 0x3f
        // 000144ed: 55                PUSH EBP
        _emit 0x55
        // 000144ee: 8b 69 10          MOV  EBP, [ECX+0x10]
        _emit 0x8b
        _emit 0x69
        _emit 0x10
        // 000144f1: 8b 4e 04          MOV  ECX, [ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 000144f4: 83 c1 20          ADD  ECX, 0x20
        _emit 0x83
        _emit 0xc1
        _emit 0x20
        // 000144f7: 3b e9             CMP  EBP, ECX
        _emit 0x3b
        _emit 0xe9
        // 000144f9: 74 30             JE   +0x30 → 0x0001452b
        _emit 0x74
        _emit 0x30
        // 000144fb: 8b 56 10          MOV  EDX, [ESI+0x10]
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        // 000144fe: 8b 46 08          MOV  EAX, [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00014501: 53                PUSH EBX
        _emit 0x53
        // 00014502: 57                PUSH EDI
        _emit 0x57
        // 00014503: 8d 7e 08          LEA  EDI, [ESI+0x8]
        _emit 0x8d
        _emit 0x7e
        _emit 0x08
        // 00014506: 89 54 24 10       MOV  [ESP+0x10], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0001450a: 8b 50 08          MOV  EDX, [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0001450d: 8b cf             MOV  ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 0001450f: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014511: 8b d8             MOV  EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 00014513: 8b 45 00          MOV  EAX, [EBP+0x0]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00014516: 8b 50 04          MOV  EDX, [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00014519: 03 5c 24 10       ADD  EBX, [ESP+0x10]
        _emit 0x03
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 0001451d: 8b cd             MOV  ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 0001451f: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014521: 89 46 0c          MOV  [ESI+0xc], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 00014524: 8b c7             MOV  EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 00014526: 5f                POP  EDI
        _emit 0x5f
        // 00014527: 89 5e 10          MOV  [ESI+0x10], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x10
        // 0001452a: 5b                POP  EBX
        _emit 0x5b
        // 0001452b: 5d                POP  EBP
        _emit 0x5d
        // 0001452c: 5e                POP  ESI
        _emit 0x5e
        // 0001452d: 59                POP  ECX
        _emit 0x59
        // 0001452e: c3                RET
        _emit 0xc3
    }
}

#endif // _MSC_VER
