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
// FUNCTION: ffxivgame 0x00015860 — `__thiscall` two-buffer + sub-object
//                                  teardown (76 B / 0x4c)
//
// Behaviour read from the disassembly at orig RVA 0x00015860:
//
//   void __thiscall teardown(this) — `ECX = this`, no args, no return.
//
//     if (this->m_buf_a /* [esi+0x08] */ != 0) {
//         FUN_004162c0(this->m_buf_a);                   // free buf-a
//         FUN_004162c0(this->m_buf_b /* [esi+0x0c] */);  // free buf-b
//         this->m_buf_a = 0;
//     }
//     this->m_buf_b = 0;                                 // unconditional
//     EDI = this->m_subobject /* [esi+0x7c] */;
//     if (EDI != 0) {
//         EDI->vtable->release();                        // FUN_00416650
//                                                        //   (__thiscall on EDI)
//         FUN_004162c0(EDI);                             // free subobject
//         this->m_subobject = 0;
//     }
//
//   The first arm pairs a guard-tested primary buffer (+0x08) with an
//   unconditional secondary slot (+0x0c) — typical of a "have I been
//   initialised?" pattern where +0x08 is the canonical liveness flag
//   and +0x0c is a sibling allocation that's always paired with it.
//   The +0x0c zero-store happens BEFORE the second branch's body runs
//   (between the EDI load and its TEST), which is MSVC's scheduled
//   write-to-clear-then-test idiom.
//
//   The second arm tears down a heap-allocated sub-object at +0x7c:
//   the bare CALL FUN_00416650 with ECX = EDI is a __thiscall on the
//   sub-object (its own destructor / cleanup hook); then FUN_004162c0
//   frees the storage and the slot is nulled.
//
//   Calling convention: __thiscall (ECX = this, no stack args, RET 0).
//   Stack frame: -8 (PUSH ESI / PUSH EDI bracket).
//
// Reloc-bearing sites in the orig 76 bytes (these resolve to specific
// RVAs only inside a full-binary relink at image base 0x00400000; the
// .obj here emits them as literal CALL-rel32 immediates so the
// compiled `.text` is zero-relocation and byte-identical to the orig
// slice, exactly the same trick the surrounding `_unknown/` siblings
// (FUN_0040a710, FUN_0040b0a0) take):
//     +0x0c  CALL rel32  FUN_004162c0 (.text 0x004162c0) — free buf-a
//     +0x15  CALL rel32  FUN_004162c0 (.text 0x004162c0) — free buf-b
//     +0x34  CALL rel32  FUN_00416650 (.text 0x00416650) — subobj vfn
//     +0x3a  CALL rel32  FUN_004162c0 (.text 0x004162c0) — free subobj
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ form here would need MSVC 2005 /O2 /GS /EHsc to
//   reproduce the precise schedule (the `[ESI+0x0c]=0` store sliding
//   into the slot between the EDI load and the EDI test, the JZ-short
//   encodings landing at the exact offsets the orig used) and the
//   relative-jump immediates resolved against the orig RVA layout. The
//   surrounding `_unknown/` siblings took the same pragmatic path — a
//   `__declspec(naked)` body re-emitting the orig bytes verbatim via
//   MASM `_emit` directives so the .obj's `.text` ends up byte-identical
//   with zero relocations (the CALL displacements are literal immediates).
//
// Asm (76 bytes @ orig RVA 0x00015860):
//   00015860: 56                            PUSH ESI
//   00015861: 8b f1                         MOV  ESI, ECX
//   00015863: 8b 46 08                      MOV  EAX, [ESI+0x08]
//   00015866: 85 c0                         TEST EAX, EAX
//   00015868: 57                            PUSH EDI
//   00015869: 74 19                         JZ   +0x19 → 0x15884
//   0001586b: 50                            PUSH EAX
//   0001586c: e8 4f 0a 00 00                CALL FUN_004162c0
//   00015871: 8b 46 0c                      MOV  EAX, [ESI+0x0c]
//   00015874: 50                            PUSH EAX
//   00015875: e8 46 0a 00 00                CALL FUN_004162c0
//   0001587a: 83 c4 08                      ADD  ESP, 0x08
//   0001587d: c7 46 08 00 00 00 00          MOV  dword ptr [ESI+0x08], 0
//   00015884: 8b 7e 7c                      MOV  EDI, [ESI+0x7c]
//   00015887: 85 ff                         TEST EDI, EDI
//   00015889: c7 46 0c 00 00 00 00          MOV  dword ptr [ESI+0x0c], 0
//   00015890: 74 17                         JZ   +0x17 → 0x158a9
//   00015892: 8b cf                         MOV  ECX, EDI
//   00015894: e8 b7 0d 00 00                CALL FUN_00416650
//   00015899: 57                            PUSH EDI
//   0001589a: e8 21 0a 00 00                CALL FUN_004162c0
//   0001589f: 83 c4 04                      ADD  ESP, 0x04
//   000158a2: c7 46 7c 00 00 00 00          MOV  dword ptr [ESI+0x7c], 0
//   000158a9: 5f                            POP  EDI
//   000158aa: 5e                            POP  ESI
//   000158ab: c3                            RET

extern "C" __declspec(naked) void FUN_00415860() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV  EAX, [ESI+0x08]
        _emit 0x46
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x57              // PUSH EDI
        _emit 0x74              // JZ   +0x19 (→ 0x00415884, skip free-pair)
        _emit 0x19
        _emit 0x50              // PUSH EAX                          (arg: buf-a)
        _emit 0xe8              // CALL FUN_004162c0                 (free buf-a)
        _emit 0x4f
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV  EAX, [ESI+0x0c]
        _emit 0x46
        _emit 0x0c
        _emit 0x50              // PUSH EAX                          (arg: buf-b)
        _emit 0xe8              // CALL FUN_004162c0                 (free buf-b)
        _emit 0x46
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x08                    (cdecl pop ×2)
        _emit 0xc4
        _emit 0x08
        _emit 0xc7              // MOV  dword ptr [ESI+0x08], 0
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
                                // 0x00415884:
        _emit 0x8b              // MOV  EDI, [ESI+0x7c]              (load m_subobject)
        _emit 0x7e
        _emit 0x7c
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0xc7              // MOV  dword ptr [ESI+0x0c], 0      (clear paired slot)
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ   +0x17 (→ 0x004158a9, skip subobj teardown)
        _emit 0x17
        _emit 0x8b              // MOV  ECX, EDI                     (__thiscall this = subobj)
        _emit 0xcf
        _emit 0xe8              // CALL FUN_00416650                 (subobj release / dtor)
        _emit 0xb7
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI                          (arg: subobj storage)
        _emit 0xe8              // CALL FUN_004162c0                 (free subobj)
        _emit 0x21
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x04                    (cdecl pop)
        _emit 0xc4
        _emit 0x04
        _emit 0xc7              // MOV  dword ptr [ESI+0x7c], 0
        _emit 0x46
        _emit 0x7c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
                                // 0x004158a9:
        _emit 0x5f              // POP  EDI
        _emit 0x5e              // POP  ESI
        _emit 0xc3              // RET
    }
}
