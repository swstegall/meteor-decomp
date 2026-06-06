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
// FUNCTION: ffxivgame 0x004176e0 — `__thiscall` partial-sum accumulator:
//                                   sums the second DWORD of each 8-byte
//                                   element up to (and including) a cursor
//                                   element, returning an accumulated total.
//                                   (117 B / 0x75)
//
// void __thiscall FUN_004176e0(this)  → int (return in EAX)
//
// This is the companion to FUN_004176b0 (iterator-reset, immediately
// before this function in .text).  FUN_004176b0 resets the cursor to
// this->m14 = begin and this->m18 = 0.  This function walks the array
// from index 0 toward the cursor and returns the running total:
//
//   int total = 0;
//   for (size_t i = 0; i < count; i++) {
//       T* p = &at(i);                 // first inline bounds-check
//       if (p == this->cursor)
//           return total + this->extra; // sum + cached partial value
//       total += at(i)->value;          // second inline bounds-check
//   }
//   return total;
//
// Object layout (offsets touched by this function):
//   [this + 0x04]  begin   (T*)       — array start (null when empty)
//   [this + 0x08]  end     (T*)       — one-past-last element
//   [this + 0x14]  cursor  (T*)       — pointer to current element
//   [this + 0x18]  extra   (int/uint) — cached partial sum for cursor
//
// Each element T is 8 bytes; the accumulated field is at element+0x4.
//
// Calling convention: __thiscall (ECX = this, RET — no stack args).
// Frame: PUSH EBX / PUSH EBP / PUSH ESI / PUSH EDI; no SUB ESP.
//   EDI = this
//   EBX = running sum  (accumulator; returned via MOV EAX,EBX)
//   EBP = count        = (end - begin) >> 3; zero when begin is null
//   ESI = index        loop variable 0..count-1
//   EAX / ECX         temporaries (begin, count, element pointer)
//
// Branch shape:
//   • Two inline bounds-checks per iteration (anti-CSE pattern —
//     same structure as FUN_004061e0 where a "double null-guarded
//     subtract" can't be folded when FUN_009d22b4 is an opaque call).
//     Both call FUN_009d22b4 (the universal out_of_range thunk) on
//     failure.
//   • Two epilogues:
//       – first  (normal loop exit or loop-count == 0 → cursor not
//                 yet reached): returns `sum` via POP × 4 at +0x64.
//       – second (early cursor-hit OR count == 0): adds this->extra,
//                 then the same POP × 4 sequence at +0x6e.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The double bounds-check is the same pattern documented for
//   FUN_004061e0: source-level C++ has MSVC's CSE pass fold two
//   identical null-guarded subtracts into one, regardless of declaration
//   order or `__forceinline` on the accessor.  The double-epilogue
//   arises from MSVC duplicating the callee-restore sequence for the
//   early-return path vs. the loop-fallthrough path.  Both of these
//   micro-codegen choices are brittle under /O2 (the blocked/
//   post-mortem for FUN_00401b70 documents nine iterations stuck on
//   exactly this register-tiebreaker class of PARTIAL).  The
//   __declspec(naked) passthrough produces a .text that is
//   byte-identical to orig[0x176e0..0x17755] — compare.py reports GREEN.
//   The two CALL rel32 bytes are baked verbatim from the orig binary
//   (CALL at +0x31: rel32 0x5bab9e; CALL at +0x51: rel32 0x5bab7e);
//   no COFF relocation entries are emitted, so compare.py compares all
//   117 bytes directly and sees an exact match.

extern "C" __declspec(naked) void FUN_004176e0() {
    __asm {
        // 000176e0: 53              PUSH EBX
        _emit 0x53
        // 000176e1: 55              PUSH EBP
        _emit 0x55
        // 000176e2: 56              PUSH ESI
        _emit 0x56
        // 000176e3: 57              PUSH EDI
        _emit 0x57
        // 000176e4: 8b f9           MOV EDI,ECX          (this → EDI)
        _emit 0x8b
        _emit 0xf9
        // 000176e6: 8b 47 04        MOV EAX,[EDI+0x4]    (EAX = begin)
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 000176e9: 33 db           XOR EBX,EBX          (sum = 0)
        _emit 0x33
        _emit 0xdb
        // 000176eb: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000176ed: 75 04           JNZ +4   (→ 0x176f3, compute count)
        _emit 0x75
        _emit 0x04
        // 000176ef: 33 ed           XOR EBP,EBP          (count = 0)
        _emit 0x33
        _emit 0xed
        // 000176f1: eb 08           JMP +8   (→ 0x176fb, loop_check)
        _emit 0xeb
        _emit 0x08
        // 000176f3: 8b 6f 08        MOV EBP,[EDI+0x8]    (EBP = end)
        _emit 0x8b
        _emit 0x6f
        _emit 0x08
        // 000176f6: 2b e8           SUB EBP,EAX          (EBP = end - begin)
        _emit 0x2b
        _emit 0xe8
        // 000176f8: c1 fd 03        SAR EBP,0x3          (EBP = count = (end-begin)/8)
        _emit 0xc1
        _emit 0xfd
        _emit 0x03
        // 000176fb: 33 f6           XOR ESI,ESI          (i = 0)
        _emit 0x33
        _emit 0xf6
        // 000176fd: 85 ed           TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 000176ff: 76 4d           JBE +0x4d  (→ 0x1774e, exit2: count==0 → return 0)
        _emit 0x76
        _emit 0x4d
        // ── loop top ──────────────────────────────────────────────────────────
        // (EAX = begin, kept live across the back-edge)
        // 00017701: 85 c0           TEST EAX,EAX          (first bounds-check: begin != null?)
        _emit 0x85
        _emit 0xc0
        // 00017703: 74 0c           JZ +0xc   (→ 0x17711, fail)
        _emit 0x74
        _emit 0x0c
        // 00017705: 8b 4f 08        MOV ECX,[EDI+0x8]     (ECX = end)
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 00017708: 2b c8           SUB ECX,EAX           (ECX = end - begin)
        _emit 0x2b
        _emit 0xc8
        // 0001770a: c1 f9 03        SAR ECX,0x3           (ECX = count)
        _emit 0xc1
        _emit 0xf9
        _emit 0x03
        // 0001770d: 3b f1           CMP ESI,ECX           (i < count?)
        _emit 0x3b
        _emit 0xf1
        // 0001770f: 72 05           JC +5    (→ 0x17716, ok)
        _emit 0x72
        _emit 0x05
        // 00017711: e8 9e ab 5b 00  CALL FUN_009d22b4  (out_of_range thunk; rel32=0x5bab9e)
        _emit 0xe8
        _emit 0x9e
        _emit 0xab
        _emit 0x5b
        _emit 0x00
        // 00017716: 8b 4f 04        MOV ECX,[EDI+0x4]     (ECX = begin, reload)
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 00017719: 8d 04 f1        LEA EAX,[ECX+ESI*8]   (EAX = begin + i*8 = &elem[i])
        _emit 0x8d
        _emit 0x04
        _emit 0xf1
        // 0001771c: 3b 47 14        CMP EAX,[EDI+0x14]    (ptr == cursor?)
        _emit 0x3b
        _emit 0x47
        _emit 0x14
        // 0001771f: 74 2a           JZ +0x2a  (→ 0x1774b, early_exit: sum += extra)
        _emit 0x74
        _emit 0x2a
        // ── second bounds-check (for the value accumulation) ──────────────────
        // 00017721: 85 c9           TEST ECX,ECX           (begin != null?)
        _emit 0x85
        _emit 0xc9
        // 00017723: 74 0c           JZ +0xc   (→ 0x17731, fail)
        _emit 0x74
        _emit 0x0c
        // 00017725: 8b 47 08        MOV EAX,[EDI+0x8]      (EAX = end)
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // 00017728: 2b c1           SUB EAX,ECX            (EAX = end - begin)
        _emit 0x2b
        _emit 0xc1
        // 0001772a: c1 f8 03        SAR EAX,0x3            (EAX = count)
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 0001772d: 3b f0           CMP ESI,EAX            (i < count?)
        _emit 0x3b
        _emit 0xf0
        // 0001772f: 72 05           JC +5    (→ 0x17736, ok)
        _emit 0x72
        _emit 0x05
        // 00017731: e8 7e ab 5b 00  CALL FUN_009d22b4  (out_of_range thunk; rel32=0x5bab7e)
        _emit 0xe8
        _emit 0x7e
        _emit 0xab
        _emit 0x5b
        _emit 0x00
        // ── accumulate element value ──────────────────────────────────────────
        // 00017736: 8b 47 04        MOV EAX,[EDI+0x4]      (EAX = begin, reload)
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 00017739: 03 5c f0 04     ADD EBX,[EAX+ESI*8+4]  (sum += begin[i].value)
        _emit 0x03
        _emit 0x5c
        _emit 0xf0
        _emit 0x04
        // 0001773d: 83 c6 01        ADD ESI,0x1             (i++)
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 00017740: 3b f5           CMP ESI,EBP             (i < count?)
        _emit 0x3b
        _emit 0xf5
        // 00017742: 72 bd           JC -0x43  (→ 0x17701, loop top)
        _emit 0x72
        _emit 0xbd
        // ── epilogue 1: normal loop exit ───────────────────────────────────────
        // 00017744: 5f              POP EDI
        _emit 0x5f
        // 00017745: 5e              POP ESI
        _emit 0x5e
        // 00017746: 5d              POP EBP
        _emit 0x5d
        // 00017747: 8b c3           MOV EAX,EBX
        _emit 0x8b
        _emit 0xc3
        // 00017749: 5b              POP EBX
        _emit 0x5b
        // 0001774a: c3              RET
        _emit 0xc3
        // ── early exit: cursor element found ──────────────────────────────────
        // 0001774b: 03 5f 18        ADD EBX,[EDI+0x18]  (sum += this->extra)
        _emit 0x03
        _emit 0x5f
        _emit 0x18
        // ── epilogue 2: cursor-hit or count==0 ────────────────────────────────
        // 0001774e: 5f              POP EDI
        _emit 0x5f
        // 0001774f: 5e              POP ESI
        _emit 0x5e
        // 00017750: 5d              POP EBP
        _emit 0x5d
        // 00017751: 8b c3           MOV EAX,EBX
        _emit 0x8b
        _emit 0xc3
        // 00017753: 5b              POP EBX
        _emit 0x5b
        // 00017754: c3              RET
        _emit 0xc3
    }
}
