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

// Forward decls for the targets the destructor / constructor call.
extern "C" void __cdecl operator_delete(void *);

class SubObjAt1c {
public:
    SubObjAt1c();
    ~SubObjAt1c();
};

// Iteration #2: define ChunkReadUInt's destructor inline-empty so MSVC
// can see it's trivial and elide the parent CALL from PackRead's D1.
// (Round #1 used `virtual ~ChunkReadUInt();` — declared but not defined,
//  which forced MSVC to emit an external CALL.)
class ChunkReadUInt {
public:
    ChunkReadUInt(const void *data, unsigned size);
    virtual ~ChunkReadUInt() {}

protected:
    const char *m_data_start;     // +0x04
    const char *m_data_end;       // +0x08
    const char *m_cursor;         // +0x0c
    int         m_field10;        // +0x10 — uninitialised by ctor
    char        m_pad14;          // +0x14 — uninitialised
    char        m_flag15;         // +0x15 — set to 0 by ctor
    short       m_pad16;          // +0x16 — uninitialised
    int         m_field18;        // +0x18 — set to 0 by ctor
};

inline ChunkReadUInt::ChunkReadUInt(const void *data, unsigned size)
    : m_data_start((const char *)data),
      m_data_end((const char *)data + size),
      m_cursor((const char *)data),
      m_flag15(0),
      m_field18(0)
{}

class PackRead : public ChunkReadUInt {
public:
    PackRead(const void *data, unsigned size);
    virtual ~PackRead();

    void PostInit();

private:
    SubObjAt1c m_subobj;          // +0x1c (composition; default-ctor'd)
    char       m_pad[0x74 - 0x1c - sizeof(SubObjAt1c)];
    void      *m_buffer;          // +0x74
    void      *m_field78;         // +0x78
    void      *m_field7c;         // +0x7c
};

// FUNCTION: ffxivgame 0x008c6670 — PackRead::~PackRead (round-#2 GREEN)
PackRead::~PackRead() {
    if (m_buffer) {
        operator_delete(m_buffer);
    }
    m_buffer = 0;
    m_field78 = 0;
    m_field7c = 0;
    // m_subobj's destructor runs implicitly via the C++ ABI.
    // Parent ChunkRead's destructor is then invoked by the surrounding
    // D1 variant generated by MSVC (elided to a vtable-swap when
    // ChunkReadUInt's body is inline-empty).
}

// FUNCTION: ffxivgame 0x00942800 — PackRead::PackRead(data, size)
//
// Original 132 B (RET 8 → __stdcall convention; 2 args = data ptr + size).
//
// Bytes (orig):
//   00942800: 6a ff 68 99 15 f0 00 64 a1 00 00 00 00 50 83 ec
//   00942810: 08 53 56 a1 b0 a8 2e 01 33 c4 50 8d 44 24 14 64
//   00942820: a3 00 00 00 00 8b f1 89 74 24 10 8b 44 24 24 8b
//   00942830: 4c 24 28 33 db 8d 14 08 89 46 04 89 56 08 89 46
//   00942840: 0c 88 5e 15 89 5e 18 8d 4e 1c 89 5c 24 1c c7 06
//   00942850: 40 dd 10 01 e8 97 34 70 ff 89 5e 74 89 5e 78 89
//   00942860: 5e 7c 8b ce c6 44 24 1c 03 e8 d2 fe ff ff 8b c6
//   00942870: 8b 4c 24 14 64 89 0d 00 00 00 00 59 5e 5b 83 c4
//   00942880: 14 c2 08 00
//
// Iteration history:
//
//   #1 [PARTIAL — 118/132 B (89%); structurally close, missing the
//      explicit PostInit CALL because MSVC inlined the empty body]
//      An inline-empty `void PostInit() {}` makes MSVC see the body
//      is trivial → CALL elided + SEH state-3 marker elided. Net: 14
//      bytes shorter than orig.
//
//   #2 [PARTIAL — 130/132 B (98%); only the prologue stack-allocation
//      pattern + SEH-state-value differ]
//      Declaring `void PostInit();` external forces MSVC to assume it
//      can throw → the SEH state machine + explicit CALL get emitted.
//      Body bytes from offset 0x16 onward match orig modulo a 2-byte
//      shift that propagates from the prologue choice:
//
//        ORIG bytes 0x0e..0x10:  83 ec 08              SUB ESP, 8
//        MINE byte  0x0e..0x0e:  51                    PUSH ECX
//
//      Orig allocates 8 bytes of locals (this-save at [ESP+0x10] +
//      one extra slot at [ESP+0xc] used by the SEH unwind table for
//      its `__EH4`-style scope record). Mine allocates 4 bytes (just
//      the this-save), which is functionally equivalent but 2 bytes
//      shorter. The choice between PUSH-reg and SUB-ESP is MSVC's
//      stack-allocator heuristic; with 1 local (4 B) it picks PUSH,
//      with ≥ 2 locals (8 B) it picks SUB ESP. To force SUB ESP we'd
//      need MSVC to see two distinct local stack slots, which would
//      typically come from using try/__try-style scope records in the
//      ctor body.
//
//      Body-byte divergences after the prologue shift:
//        - SEH state-index slot: mine [ESP+0x18], orig [ESP+0x1c]
//          (4-byte shift from prologue size diff)
//        - SEH state value at the mid-point: mine writes 1, orig 3.
//          MSVC's state numbering is unwind-table compaction; the
//          values are arbitrary labels, not iteration counters. Mine
//          has 1 transition (subobj ctor → ready), orig has more
//          states allocated in its unwind table.
//        - Order of `MOV ECX, ESI` vs the m_buffer-zero stores:
//          MSVC schedules the ECX load before the buffer stores in
//          mine, after them in orig. Same effect, different scheduling.
//
//   Next steps (round #3+ — deferred):
//     The remaining 2-byte gap needs either a `__declspec(naked)`
//     emission (loses C++ readability) or an experimentally-found
//     C++ pattern that triggers MSVC's larger-frame heuristic. Try
//     one of:
//       (a) Add `__try { PostInit(); } __except(...) { ... }` —
//           forces MSVC to allocate the full __EH4 scope record.
//       (b) Add a pair of stack-allocated locals (e.g., `int a, b;`
//           with side-effecting use) so MSVC switches from PUSH to
//           SUB ESP for the local-allocation step.
//       (c) Naked asm for the prologue + body call sequence.
//     None are urgent — the constructor is functionally correct and
//     will work for any reader of the file's data fields.
//
// SEH state transitions in orig: -1 (initial), 0 (after trivial field
// stores, before sub-obj ctor; if it throws nothing's constructed),
// 3 (after sub-obj ctor returns, before PostInit; if PostInit throws,
// ~SubObjAt1c needs to run during unwind). The 0→3 jump rather than
// 0→1→2→3 is MSVC's state-id compaction — only values that mark real
// unwind transitions get assigned, others fall out.
// Declared external — MSVC has to assume it may throw, which forces
// the SEH state machine (`MOV [ESP+0x1c], 0` after vtable+sub-obj
// init, `MOV [ESP+0x1c], 3` after PostInit guard) to be emitted in
// the constructor. An inline-empty body would elide the CALL and
// the state markers entirely.
PackRead::PackRead(const void *data, unsigned size)
    : ChunkReadUInt(data, size),
      m_buffer(0),
      m_field78(0),
      m_field7c(0)
{
    PostInit();
}
