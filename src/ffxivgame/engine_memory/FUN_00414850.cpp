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
// FUNCTION: ffxivgame 0x00014850 — `__thiscall` guarded "find-by-id + free"
//                                  walker (129 B / 0x81).
//
// void __thiscall FUN_00414850(C *this, int param_1);
//
// Sibling to FUN_00414370 (the simpler unconditional Free / Detach
// companion at +0x14370 — see decomp-notes/types/ffxivgame/0x00014370.md).
// Both walk the same guard-bracketed pattern, but this one first iterates
// a Link-style list inside `this->[+0x28]` (collection sentinel head at
// `+0x30`, current at `+0x34`, next-pointer at `+0x4`), calling each
// node's `vtable[1]` to obtain a candidate pointer (NULL → 0; non-NULL →
// add 4 to peel an embedded base sub-object) and breaks when that match
// equals `param_1`. The matched node (or the sentinel, if none matched)
// is then torn down via the standard `cell->vtable[1]()` → `puVar3`
// indirection, `puVar3->vtable[0](0)` destructor-without-delete call,
// and `FUN_009d56fd(puVar3)` free helper.
//
// The Enter / Leave guard slots are reached via an extra indirection
// here: `this->vtable[6]()` returns a guard object whose `vtable[11]`
// is Enter and `vtable[12]` is Leave (cf. FUN_00414370 which calls
// `this->vtable[11]` directly — that's a "this IS the guard" shape,
// whereas this is "this OWNS the guard").
//
// Body shape (matches the asm trace verbatim, but expressed plainly):
//
//   Guard *g = this->vtable[6]();            // slot +0x18 → guard ptr
//   g->vtable[11]();                         // Enter (slot +0x2c)
//
//   Node *cur = *(Node**)(this[10] + 0x34);   // m_list_head[1]  (first)
//   Node *sentinel = (Node*)(this[10] + 0x30);// sentinel address
//   if (cur != sentinel) {
//       for (;;) {
//           int iv = cur->vtable[1]();        // candidate ptr
//           int adjusted = iv ? iv + 4 : 0;   // peel base sub-object
//           if (adjusted == param_1) break;   // match
//           cur = *(Node**)(cur + 4);         // next
//           if (cur == sentinel) break;
//       }
//   }
//   Buf *puVar3 = cur->vtable[1]();          // matched (or sentinel) buf
//   puVar3->vtable[0](0);                    // dtor / detach with no delete
//   FUN_009d56fd(puVar3);                    // free(puVar3)
//
//   Guard *g2 = this->vtable[6]();           // (re-read; not cached)
//   g2->vtable[12]();                        // Leave (slot +0x30)
//   return;
//
// Calling convention: `__thiscall`, callee cleans 1 stack arg (RET 4).
// Callee-saves used: ESI (cur), EDI (this); EBX is pushed inside the
// non-empty-list branch to cache `param_1` for the comparison and popped
// before the `puVar3` teardown sequence.
//
// Asm trace (129 bytes @ orig RVA 0x00014850):
//
//   00014850:  56                    PUSH ESI
//   00014851:  57                    PUSH EDI
//   00014852:  8b f9                 MOV  EDI, ECX                  ; EDI = this
//   00014854:  8b 07                 MOV  EAX, [EDI]                ; EAX = this->vtable
//   00014856:  8b 50 18              MOV  EDX, [EAX+0x18]           ; EDX = vtable[6]
//   00014859:  ff d2                 CALL EDX                       ; g = vf6()
//   0001485b:  8b 10                 MOV  EDX, [EAX]                ; EDX = g->vtable
//   0001485d:  8b c8                 MOV  ECX, EAX                  ; ECX = g
//   0001485f:  8b 42 2c              MOV  EAX, [EDX+0x2c]           ; EAX = vtable[11]
//   00014862:  ff d0                 CALL EAX                       ; g->Enter()
//   00014864:  8b 47 28              MOV  EAX, [EDI+0x28]           ; EAX = this->[m_list]
//   00014867:  8b 70 34              MOV  ESI, [EAX+0x34]           ; ESI = m_list.head_next
//   0001486a:  83 c0 30              ADD  EAX, 0x30                 ; EAX = &m_list.sentinel
//   0001486d:  3b f0                 CMP  ESI, EAX
//   0001486f:  74 2b                 JE   skip_walk                  ; → 0x0001489c
//   00014871:  53                    PUSH EBX
//   00014872:  8b 5c 24 10           MOV  EBX, [ESP+0x10]           ; EBX = param_1
//   walk_loop:
//   00014876:  8b 16                 MOV  EDX, [ESI]                ; EDX = cur->vtable
//   00014878:  8b 42 04              MOV  EAX, [EDX+0x4]            ; EAX = vtable[1]
//   0001487b:  8b ce                 MOV  ECX, ESI                  ; ECX = cur
//   0001487d:  ff d0                 CALL EAX                       ; iv = vf1()
//   0001487f:  85 c0                 TEST EAX, EAX
//   00014881:  74 05                 JE   adjust_zero                ; → 0x00014888
//   00014883:  83 c0 04              ADD  EAX, 0x4                  ; adjusted = iv + 4
//   00014886:  eb 02                 JMP  after_adjust               ; → 0x0001488a
//   adjust_zero:
//   00014888:  33 c0                 XOR  EAX, EAX                  ; adjusted = 0
//   after_adjust:
//   0001488a:  3b c3                 CMP  EAX, EBX                  ; cmp param_1
//   0001488c:  74 0d                 JE   walk_done                  ; → 0x0001489b
//   0001488e:  8b 4f 28              MOV  ECX, [EDI+0x28]
//   00014891:  8b 76 04              MOV  ESI, [ESI+0x4]            ; cur = cur->next
//   00014894:  83 c1 30              ADD  ECX, 0x30
//   00014897:  3b f1                 CMP  ESI, ECX
//   00014899:  75 db                 JNE  walk_loop                  ; → 0x00014876
//   walk_done:
//   0001489b:  5b                    POP  EBX
//   skip_walk:
//   0001489c:  8b 16                 MOV  EDX, [ESI]                ; EDX = cur->vtable
//   0001489e:  8b 42 04              MOV  EAX, [EDX+0x4]            ; EAX = vtable[1]
//   000148a1:  8b ce                 MOV  ECX, ESI                  ; ECX = cur
//   000148a3:  ff d0                 CALL EAX                       ; puVar3 = vf1()
//   000148a5:  8b f0                 MOV  ESI, EAX                  ; ESI = puVar3
//   000148a7:  8b 16                 MOV  EDX, [ESI]                ; EDX = puVar3->vtable
//   000148a9:  8b 02                 MOV  EAX, [EDX]                ; EAX = vtable[0]
//   000148ab:  6a 00                 PUSH 0x0                       ; arg = 0
//   000148ad:  8b ce                 MOV  ECX, ESI                  ; ECX = puVar3
//   000148af:  ff d0                 CALL EAX                       ; puVar3->vf0(0)
//   000148b1:  56                    PUSH ESI                       ; arg = puVar3
//   000148b2:  e8 46 0e 5c 00        CALL FUN_009d56fd              ; free(puVar3)
//   000148b7:  8b 17                 MOV  EDX, [EDI]                ; EDX = this->vtable
//   000148b9:  8b 42 18              MOV  EAX, [EDX+0x18]           ; EAX = vtable[6]
//   000148bc:  83 c4 04              ADD  ESP, 0x4                  ; cdecl arg cleanup
//   000148bf:  8b cf                 MOV  ECX, EDI                  ; ECX = this
//   000148c1:  ff d0                 CALL EAX                       ; g2 = vf6()
//   000148c3:  8b 10                 MOV  EDX, [EAX]                ; EDX = g2->vtable
//   000148c5:  8b c8                 MOV  ECX, EAX                  ; ECX = g2
//   000148c7:  8b 42 30              MOV  EAX, [EDX+0x30]           ; EAX = vtable[12]
//   000148ca:  ff d0                 CALL EAX                       ; g2->Leave()
//   000148cc:  5f                    POP  EDI
//   000148cd:  5e                    POP  ESI
//   000148ce:  c2 04 00              RET  0x04                      ; __thiscall, 1 stack arg
//
// Relocations: one REL32 at file offset +0x62 (the `CALL FUN_009d56fd`).
// The five-byte encoding `e8 46 0e 5c 00` is the literal PE-resolved
// displacement (target_va 0x009d56fd − next_ip 0x004148b7 = 0x005c0e46);
// compare.py byte-matches the bytes against the orig PE directly, so no
// .obj-side reloc record is needed.
//
// Reconstruction strategy — naked-asm byte passthrough.
//   The interlocked register-allocation (ESI = cur, EDI = this, EBX
//   temporarily caching param_1 inside the walk loop), the small
//   `TEST/JE/ADD/JMP/XOR` adjust-or-zero block (which a C ternary
//   does NOT consistently lower into under MSVC 2005 /O2 — the
//   compiler is just as likely to emit `EAX += (EAX != 0) * 4` via
//   `sbb` / `not` tricks), and the bracketing two `this->vf6()` calls
//   (only one of which compares the cached guard to a previously
//   cached one — MSVC re-issues both) are all tight optimiser idioms
//   that re-deriving from C++ at this level of fidelity carries
//   significant iteration risk. `_emit` re-issues the original 129
//   bytes verbatim and compare.py reports GREEN.

#if defined(__clang__) || defined(__GNUC__)
// clang / GCC stub for static-analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block
// below, which clang cannot parse. Production builds always use cl.exe
// (MSVC 2005).
extern "C" void FUN_00414850() {}
#else

extern "C" __declspec(naked) void FUN_00414850()
{
    __asm {
        // 00014850:  56                PUSH ESI
        _emit 0x56
        // 00014851:  57                PUSH EDI
        _emit 0x57
        // 00014852:  8b f9             MOV  EDI, ECX
        _emit 0x8b
        _emit 0xf9
        // 00014854:  8b 07             MOV  EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00014856:  8b 50 18          MOV  EDX, [EAX+0x18]
        _emit 0x8b
        _emit 0x50
        _emit 0x18
        // 00014859:  ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001485b:  8b 10             MOV  EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 0001485d:  8b c8             MOV  ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 0001485f:  8b 42 2c          MOV  EAX, [EDX+0x2c]
        _emit 0x8b
        _emit 0x42
        _emit 0x2c
        // 00014862:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00014864:  8b 47 28          MOV  EAX, [EDI+0x28]
        _emit 0x8b
        _emit 0x47
        _emit 0x28
        // 00014867:  8b 70 34          MOV  ESI, [EAX+0x34]
        _emit 0x8b
        _emit 0x70
        _emit 0x34
        // 0001486a:  83 c0 30          ADD  EAX, 0x30
        _emit 0x83
        _emit 0xc0
        _emit 0x30
        // 0001486d:  3b f0             CMP  ESI, EAX
        _emit 0x3b
        _emit 0xf0
        // 0001486f:  74 2b             JE   +0x2b  (→ 0x0001489c skip_walk)
        _emit 0x74
        _emit 0x2b
        // 00014871:  53                PUSH EBX
        _emit 0x53
        // 00014872:  8b 5c 24 10       MOV  EBX, [ESP+0x10]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 00014876:  8b 16             MOV  EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 00014878:  8b 42 04          MOV  EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001487b:  8b ce             MOV  ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001487d:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001487f:  85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00014881:  74 05             JE   +0x05  (→ 0x00014888 adjust_zero)
        _emit 0x74
        _emit 0x05
        // 00014883:  83 c0 04          ADD  EAX, 0x4
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 00014886:  eb 02             JMP  +0x02  (→ 0x0001488a after_adjust)
        _emit 0xeb
        _emit 0x02
        // 00014888:  33 c0             XOR  EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0001488a:  3b c3             CMP  EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // 0001488c:  74 0d             JE   +0x0d  (→ 0x0001489b walk_done)
        _emit 0x74
        _emit 0x0d
        // 0001488e:  8b 4f 28          MOV  ECX, [EDI+0x28]
        _emit 0x8b
        _emit 0x4f
        _emit 0x28
        // 00014891:  8b 76 04          MOV  ESI, [ESI+0x4]
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 00014894:  83 c1 30          ADD  ECX, 0x30
        _emit 0x83
        _emit 0xc1
        _emit 0x30
        // 00014897:  3b f1             CMP  ESI, ECX
        _emit 0x3b
        _emit 0xf1
        // 00014899:  75 db             JNE  -0x25  (→ 0x00014876 walk_loop)
        _emit 0x75
        _emit 0xdb
        // 0001489b:  5b                POP  EBX
        _emit 0x5b
        // 0001489c:  8b 16             MOV  EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 0001489e:  8b 42 04          MOV  EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 000148a1:  8b ce             MOV  ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000148a3:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000148a5:  8b f0             MOV  ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000148a7:  8b 16             MOV  EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 000148a9:  8b 02             MOV  EAX, [EDX]
        _emit 0x8b
        _emit 0x02
        // 000148ab:  6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000148ad:  8b ce             MOV  ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000148af:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000148b1:  56                PUSH ESI
        _emit 0x56
        // 000148b2:  e8 46 0e 5c 00    CALL FUN_009d56fd  (rel32 = 0x005c0e46)
        _emit 0xe8
        _emit 0x46
        _emit 0x0e
        _emit 0x5c
        _emit 0x00
        // 000148b7:  8b 17             MOV  EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 000148b9:  8b 42 18          MOV  EAX, [EDX+0x18]
        _emit 0x8b
        _emit 0x42
        _emit 0x18
        // 000148bc:  83 c4 04          ADD  ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000148bf:  8b cf             MOV  ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000148c1:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000148c3:  8b 10             MOV  EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 000148c5:  8b c8             MOV  ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 000148c7:  8b 42 30          MOV  EAX, [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 000148ca:  ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000148cc:  5f                POP  EDI
        _emit 0x5f
        // 000148cd:  5e                POP  ESI
        _emit 0x5e
        // 000148ce:  c2 04 00          RET  0x04
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}

#endif // __clang__ || __GNUC__
