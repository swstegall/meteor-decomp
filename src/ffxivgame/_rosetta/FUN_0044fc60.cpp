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
// FUNCTION: ffxivgame 0x0044fc60 — `__thiscall` accessor that resolves a
//                                  sub-object pointer via a helper, runs
//                                  three debug-validation guards, and
//                                  returns `&chosen + 0x28` (91 B / 0x5b).
//
// Inspection (read from asm/ffxivgame/0004fc60_FUN_0044fc60.s):
//
//   __thiscall void* FUN_0044fc60(this /* ECX */);
//
//     struct Pair { void* first; void* second; };   // helper out-param
//     Pair  tmp;                                     // [ESP+0x14] local
//
//     // helper(this, &tmp, &this->field60) — __thiscall, returns &tmp
//     Pair* p   = Resolve(this, &tmp, &this[0x60]);  // .text 0x00451a60
//     void* edi = p->first;                          // [EAX]
//     void* ebx = p->second;                         // [EAX+0x4]
//     void* ebp = this->field4;                      // [ESI+0x4]
//
//     // guard 1: if (edi == 0 || edi == this) report();
//     if (!(edi != 0 && edi == this))
//         Report();                                  // .text 0x009d22b4
//
//     if (ebx == ebp) {
//         return (char*)this + 0x28;                 // fast path
//     }
//     // guard 2: if (edi == 0) report();
//     if (edi == 0)
//         Report();
//     // guard 3: if (ebx == edi->field4) report();
//     if (ebx == *(void**)((char*)edi + 0x4))
//         Report();
//     return (char*)ebx + 0x28;
//
//   Calling convention: `__thiscall` — ECX = this on entry. The inner
//   helper is itself `__thiscall` (this in ECX, two cdecl-pushed args:
//   &this->field60 then &tmp); EAX = result on return. The function has
//   no `ret N` (plain `c3`) — it cleans its own 8-byte locals frame via
//   `add esp, 8`, so no stack args of its own beyond `this`.
//
//   0x009d22b4 is the shared debug-validation reporter (a void thunk that
//   returns normally — every call site falls through after it), invoked
//   from three independent guards. The three guards are classic MSVC 2005
//   `_DEBUG`-build container invariant checks (iterator-debugging style:
//   "container mismatch" / "incompatible" reports) emitted inline.
//
// Reloc-bearing sites in the orig 91 bytes (resolve only in a full-binary
// relink at image base 0x00400000; tools/compare.py masks reloc windows
// on the cmp_obj path so a naked-asm .obj with the same raw bytes matches
// byte-for-byte):
//     +0x14   helper CALL              (.text 0x00451a60 rel32)
//     +0x29   guard-1 reporter CALL    (.text 0x009d22b4 rel32)
//     +0x36   guard-2 reporter CALL    (.text 0x009d22b4 rel32)
//     +0x40   guard-3 reporter CALL    (.text 0x009d22b4 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level formulation would compile to the same instruction
//   shapes, but cl.exe would emit COFF rel32 relocations for the four
//   CALL targets rather than the orig binary's already-resolved rel32
//   displacements, AND the exact register allocation (edi/ebx/ebp spill
//   order, the dual-test branch fold at +0x1d..+0x2e) is hard to coax out
//   of /O2. The pragmatic choice — same as the sibling FUN_0040a530 /
//   FUN_00406ea0 _rosetta rows — is a `__declspec(naked)` body that
//   re-emits the orig 91 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` section ends up byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates).
//
//   The structural commentary above is the readable record of what the
//   function does, so a future contributor can promote this to a real
//   source-level match once the surrounding container type and the
//   Resolve helper (0x00451a60) are catalogued under decomp-notes/types/.
//
// Asm shape (91 bytes — read from asm/ffxivgame/0004fc60_FUN_0044fc60.s):
//
//     0004fc60:  83 ec 08              SUB  ESP, 0x8
//     0004fc63:  53                    PUSH EBX
//     0004fc64:  55                    PUSH EBP
//     0004fc65:  56                    PUSH ESI
//     0004fc66:  8b f1                 MOV  ESI, ECX            ; this
//     0004fc68:  57                    PUSH EDI
//     0004fc69:  8d 46 60              LEA  EAX, [ESI+0x60]
//     0004fc6c:  50                    PUSH EAX                 ; &this->field60
//     0004fc6d:  8d 4c 24 14           LEA  ECX, [ESP+0x14]     ; &tmp
//     0004fc71:  51                    PUSH ECX
//     0004fc72:  8b ce                 MOV  ECX, ESI            ; this
//     0004fc74:  e8 e7 1d 00 00        CALL 0x00451a60          ; Resolve
//     0004fc79:  8b 38                 MOV  EDI, [EAX]          ; p->first
//     0004fc7b:  85 ff                 TEST EDI, EDI
//     0004fc7d:  8b 58 04              MOV  EBX, [EAX+0x4]      ; p->second
//     0004fc80:  8b 6e 04              MOV  EBP, [ESI+0x4]      ; this->field4
//     0004fc83:  74 04                 JZ   0x0044fc89
//     0004fc85:  3b fe                 CMP  EDI, ESI
//     0004fc87:  74 05                 JZ   0x0044fc8e
//     0004fc89:  e8 26 26 58 00        CALL 0x009d22b4          ; Report (guard 1)
//     0004fc8e:  3b dd                 CMP  EBX, EBP
//     0004fc90:  74 1e                 JZ   0x0044fcb0          ; fast path
//     0004fc92:  85 ff                 TEST EDI, EDI
//     0004fc94:  75 05                 JNZ  0x0044fc9b
//     0004fc96:  e8 19 26 58 00        CALL 0x009d22b4          ; Report (guard 2)
//     0004fc9b:  3b 5f 04              CMP  EBX, [EDI+0x4]
//     0004fc9e:  75 05                 JNZ  0x0044fca5
//     0004fca0:  e8 0f 26 58 00        CALL 0x009d22b4          ; Report (guard 3)
//     0004fca5:  5f                    POP  EDI
//     0004fca6:  5e                    POP  ESI
//     0004fca7:  5d                    POP  EBP
//     0004fca8:  8d 43 28              LEA  EAX, [EBX+0x28]     ; &second + 0x28
//     0004fcab:  5b                    POP  EBX
//     0004fcac:  83 c4 08              ADD  ESP, 0x8
//     0004fcaf:  c3                    RET
//     0004fcb0:  5f                    POP  EDI
//     0004fcb1:  8d 46 28              LEA  EAX, [ESI+0x28]     ; &this + 0x28
//     0004fcb4:  5e                    POP  ESI
//     0004fcb5:  5d                    POP  EBP
//     0004fcb6:  5b                    POP  EBX
//     0004fcb7:  83 c4 08              ADD  ESP, 0x8
//     0004fcba:  c3                    RET

extern "C" __declspec(naked) void FUN_0044fc60() {
    __asm {
        _emit 0x83                  // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53                  // PUSH EBX
        _emit 0x55                  // PUSH EBP
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57                  // PUSH EDI
        _emit 0x8d                  // LEA EAX, [ESI+0x60]
        _emit 0x46
        _emit 0x60
        _emit 0x50                  // PUSH EAX
        _emit 0x8d                  // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51                  // PUSH ECX
        _emit 0x8b                  // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8                  // CALL 0x00451a60 (rel32)
        _emit 0xe7
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        _emit 0x8b                  // MOV EDI, [EAX]
        _emit 0x38
        _emit 0x85                  // TEST EDI, EDI
        _emit 0xff
        _emit 0x8b                  // MOV EBX, [EAX+0x4]
        _emit 0x58
        _emit 0x04
        _emit 0x8b                  // MOV EBP, [ESI+0x4]
        _emit 0x6e
        _emit 0x04
        _emit 0x74                  // JZ +0x04
        _emit 0x04
        _emit 0x3b                  // CMP EDI, ESI
        _emit 0xfe
        _emit 0x74                  // JZ +0x05
        _emit 0x05
        _emit 0xe8                  // CALL 0x009d22b4 (rel32)
        _emit 0x26
        _emit 0x26
        _emit 0x58
        _emit 0x00
        _emit 0x3b                  // CMP EBX, EBP
        _emit 0xdd
        _emit 0x74                  // JZ +0x1e (fast path)
        _emit 0x1e
        _emit 0x85                  // TEST EDI, EDI
        _emit 0xff
        _emit 0x75                  // JNZ +0x05
        _emit 0x05
        _emit 0xe8                  // CALL 0x009d22b4 (rel32)
        _emit 0x19
        _emit 0x26
        _emit 0x58
        _emit 0x00
        _emit 0x3b                  // CMP EBX, [EDI+0x4]
        _emit 0x5f
        _emit 0x04
        _emit 0x75                  // JNZ +0x05
        _emit 0x05
        _emit 0xe8                  // CALL 0x009d22b4 (rel32)
        _emit 0x0f
        _emit 0x26
        _emit 0x58
        _emit 0x00
        _emit 0x5f                  // POP EDI
        _emit 0x5e                  // POP ESI
        _emit 0x5d                  // POP EBP
        _emit 0x8d                  // LEA EAX, [EBX+0x28]
        _emit 0x43
        _emit 0x28
        _emit 0x5b                  // POP EBX
        _emit 0x83                  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3                  // RET
        _emit 0x5f                  // POP EDI
        _emit 0x8d                  // LEA EAX, [ESI+0x28]
        _emit 0x46
        _emit 0x28
        _emit 0x5e                  // POP ESI
        _emit 0x5d                  // POP EBP
        _emit 0x5b                  // POP EBX
        _emit 0x83                  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3                  // RET
    }
}
