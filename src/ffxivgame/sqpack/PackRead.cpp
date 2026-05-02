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
// FUNCTION: ffxivgame 0x008c6670 — Sqex::Data::PackRead::~PackRead
// Phase 4 first matching target. NOT YET MATCHED — this file documents
// the analysis and stages the source for an iteration loop.
//
// Original function bytes (110 B; Ghidra under-counted by 3, true size is
// 0x6e not 0x6b — confirmed by the trailing `83 c4 10 c3 cc cc` ADD/RET +
// INT3 padding before the next function at 0x008c66e0):
//
//   008c6670: 6a ff 68 f8 45 ef 00 64 a1 00 00 00 00 50 51 56
//   008c6680: 57 a1 b0 a8 2e 01 33 c4 50 8d 44 24 10 64 a3 00
//   008c6690: 00 00 00 8b f1 89 74 24 0c c7 06 40 dd 10 01 8b
//   008c66a0: 46 74 33 ff 3b c7 89 7c 24 18 74 09 50 e8 65 b4
//   008c66b0: d0 ff 83 c4 04 8d 4e 1c 89 7e 74 89 7e 78 89 7e
//   008c66c0: 7c e8 8a 08 78 ff c7 06 c8 31 f9 00 8b 4c 24 10
//   008c66d0: 64 89 0d 00 00 00 00 59 5f 5e 83 c4 10 c3
//
// Decoded:
//
//   ; Standard MSVC SEH-protected `__thiscall` destructor frame.
//   PUSH -1                     ; SEH handle marker
//   PUSH offset _$LSDA$         ; @ 0x00ef45f8 — SEH handler / unwind table
//   PUSH FS:[0]                 ; chain prev handler
//   PUSH ECX                    ; save `this`
//   PUSH ESI                    ; callee-saved
//   PUSH EDI                    ; callee-saved
//   MOV  EAX, [__security_cookie]
//   XOR  EAX, ESP               ; cookie ^ ESP (no later validation —
//   PUSH EAX                    ; this function has no stack-buffer
//                                 ; arrays so /GS skips the check)
//   LEA  EAX, [ESP + 0x10]
//   MOV  FS:[0], EAX            ; install SEH
//
//   ; Body.
//   MOV  ESI, ECX                       ; ESI = this
//   MOV  [ESP + 0xc], ESI               ; (this slot used by SEH unwind)
//   MOV  DWORD PTR [ESI], 0x110dd40     ; this->vftable = PackRead::vftable
//   MOV  EAX, [ESI + 0x74]              ; EAX = this->m_buffer
//   XOR  EDI, EDI                       ; EDI = 0 (reused below)
//   CMP  EAX, EDI
//   MOV  [ESP + 0x18], EDI              ; SEH state index = 0
//   JZ   skip_delete
//     PUSH EAX
//     CALL operator_delete                ; @ 0x005d1b1a (cdecl void(void*))
//     ADD  ESP, 4
//   skip_delete:
//   LEA  ECX, [ESI + 0x1c]              ; ECX = &this->m_subobj
//   MOV  [ESI + 0x74], EDI              ; m_buffer  = nullptr
//   MOV  [ESI + 0x78], EDI              ; m_field78 = nullptr
//   MOV  [ESI + 0x7c], EDI              ; m_field7c = nullptr
//   CALL m_subobj.~Dtor                  ; @ 0x00446f50 (composition member)
//   MOV  DWORD PTR [ESI], 0xb931c8       ; vtable -> parent
//                                         ; ChunkRead<u32,u32>::vftable
//
//   ; SEH frame teardown.
//   MOV  ECX, [ESP + 0x10]              ; restore prev SEH chain
//   MOV  FS:[0], ECX
//   POP  ECX                            ; discard cookie XOR
//   POP  EDI
//   POP  ESI
//   ADD  ESP, 0x10                      ; -1 + handler + chain + this-save
//   RET
//
// Class layout inferred (best-effort — read methods to be analysed
// will refine these field names and types):
//
//   class ChunkRead<unsigned int, unsigned int> {  // vtable @ 0xb931c8
//       /* +0x00 */ void *vftable;
//       /* +0x04 */ char  base_state[0x18];   // ChunkRead's own data
//   };
//
//   class PackRead : public ChunkRead<unsigned int, unsigned int> {
//       /* +0x1c */ SubObjAt1c m_subobj;       // composition; size 0x58
//       /* +0x74 */ void      *m_buffer;       // heap-allocated, freed in dtor
//       /* +0x78 */ void      *m_field78;
//       /* +0x7c */ void      *m_field7c;
//   };
//
//   sizeof(PackRead) >= 0x80
//
// Iteration history:
//
//   #1 [PARTIAL — first 85 bytes match exactly; 9-byte tail divergence]
//      Class definition with `virtual ~ChunkReadUInt();` (declared but
//      not defined). MSVC treats this as an external function and emits
//      a `CALL ChunkReadUInt::~ChunkReadUInt` (15 B with the surrounding
//      `MOV ECX, ESI; MOV [ESP+0x18], -1`) at the tail of PackRead's
//      destructor — the D1 ("complete object destructor") variant. The
//      original is the D0 ("base destructor") variant which just swaps
//      the parent vtable and returns.
//
//   #2 [✅ GREEN — 110/110 bytes match modulo 6 reloc fields]
//      Defining ChunkReadUInt's destructor inline-empty
//      (`virtual ~ChunkReadUInt() {}`) lets MSVC see the parent body is
//      trivial under /O2 and elide the CALL entirely — falling through
//      to a plain vtable-swap-and-return that matches the D0 variant
//      bit-for-bit. Verified 2026-05-02 via a symbol-aware diff; the
//      6 reloc-wildcarded ranges line up at offsets 0x03, 0x12, 0x2b,
//      0x3e, 0x52, 0x57 (SEH handler / security cookie / PackRead
//      vtable / operator delete / SubObjAt1c::~SubObjAt1c / ChunkRead
//      vtable).
//
// Note: this .cpp produces multiple `.text` sections (one for PackRead's
// destructor, one for ChunkReadUInt's inline-empty destructor, plus the
// scalar deleting destructor wrapper). `tools/compare.py` currently
// picks the FIRST .text section, which is the wrong one here. Use the
// symbol-aware diff in `tools/verify_packread.py` (or move PackRead's
// destructor to its own translation unit when matching this file via
// the standard rosetta workflow).

// Forward decls for the targets the destructor calls.
extern "C" void __cdecl operator_delete(void *);

class SubObjAt1c {
public:
    ~SubObjAt1c();
};

// Iteration #2: define ChunkReadUInt's destructor inline-empty so MSVC
// can see it's trivial and elide the parent CALL from PackRead's D1.
// (Round #1 used `virtual ~ChunkReadUInt();` — declared but not defined,
//  which forced MSVC to emit an external CALL.)
class ChunkReadUInt {
public:
    virtual ~ChunkReadUInt() {}
private:
    char base_state[0x18];
};

class PackRead : public ChunkReadUInt {
public:
    virtual ~PackRead();

private:
    SubObjAt1c m_subobj;          // +0x1c
    char       m_pad[0x74 - 0x1c - sizeof(SubObjAt1c)];
    void      *m_buffer;          // +0x74
    void      *m_field78;         // +0x78
    void      *m_field7c;         // +0x7c
};

PackRead::~PackRead() {
    if (m_buffer) {
        operator_delete(m_buffer);
    }
    m_buffer = 0;
    m_field78 = 0;
    m_field7c = 0;
    // m_subobj's destructor runs implicitly via the C++ ABI.
    // Parent ChunkRead's destructor is then invoked by the surrounding
    // D1 variant generated by MSVC.
}
