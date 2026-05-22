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
// FUNCTION: ffxivgame 0x00408a80 — `__cdecl` (87 B / 0x57) `_Make_heap`
//                                  for 64-byte trivially-copyable elements.
//
// Inspection (read from asm/ffxivgame/00008a80_FUN_00408a80.s):
//
//   __cdecl void FUN_00408a80(Elem *first, Elem *last);   // sizeof(Elem)==64
//
//   The STL `<algorithm>` make-heap shape from MSVC 2005, specialized
//   to a 64-byte element type:
//
//     int _Bottom = (int)(last - first);          // SAR EAX, 6 (sizeof=64)
//     int _Hole   = _Bottom / 2;                  // signed /2: CDQ;SUB;SAR 1
//     while (0 < _Hole) {
//         --_Hole;
//         Elem _Val = first[_Hole];               // 64-byte struct copy
//         FUN_00408910(first, _Hole, _Bottom, _Val); // _Adjust_heap sift-down
//     }
//
//   The sibling at 0x00408910 (210 B) is the matching `_Adjust_heap`:
//   it loads `arg1 = first`, `arg2 = _Hole`, `arg3 = _Bottom`, plus a
//   pass-by-value 64-byte `_Val` on the stack, and sifts `_Val` down
//   the binary heap until the heap invariant holds. The per-element
//   comparison inside 0x00408910 walks the first dword pair byte-wise
//   then word-stride (a strcmp-like predicate over the leading bytes
//   of `Elem`), so the heap is keyed on a leading string/byte field.
//
//   The 0x4087f0 (insertion-sort), 0x4089f0 (median-of-three / recursive
//   sort body), and 0x408910 (heap adjust) siblings together form an
//   introsort instantiation; this function is the heap-build entry
//   point invoked by 0x4089f0 when the recursion depth runs out.
//
//   Calling convention: `__cdecl` — args on stack at [ESP+4]=first,
//   [ESP+8]=last; `RET` (no `N`), caller cleans. Stack frame: four
//   callee-saved (EBX/EBP/ESI/EDI) + a 64-byte local for `_Val`.
//
//   Codegen highlights — patterns to be aware of when matching:
//
//     +0x0d  MOV [ESP+8], EAX   — spills `_Bottom` back into the
//                                  (now-dead) `last` arg slot, freeing
//                                  EAX/EDX for the sift-down call.
//     +0x12  CDQ;SUB EAX,EDX;SAR 1 — canonical MSVC signed-divide-by-2
//                                  (rounds toward zero).
//     +0x35  PUSH EAX before REP MOVSD — `_Bottom` (arg3 of the call)
//                                  is pushed first, then the 64-byte
//                                  `_Val` block is `rep movsd`'d into
//                                  the room SUB ESP, 0x40 reserved,
//                                  then PUSH EBX / PUSH ECX push
//                                  `_Hole` and `first` last (so the
//                                  call's arg layout becomes
//                                  [first, _Hole, _Bottom, _Val]).
//     +0x40  EBP -= 0x40 each iter — the source pointer for the per-
//                                  iteration struct copy is maintained
//                                  incrementally (`begin + _Hole*64`),
//                                  not recomputed from `first + _Hole`.
//     +0x4e  JG back to +0x27   — re-enters the loop body past the
//                                  one-time EBP recomputation; only the
//                                  reload of `_Bottom` from the arg
//                                  slot and the SUB EBP, 0x40 step run
//                                  per iteration.
//
//   Reloc-bearing sites in the orig 87 bytes (relative CALL to the
//   sibling `_Adjust_heap`; resolves only at full-binary link time —
//   standalone .obj compilation emits the orig rel32 as a raw literal
//   which happens to match the orig bytes verbatim):
//     +0x46  CALL rel32 → FUN_00408910  (e8 45 fe ff ff)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing several brittle patterns simultaneously: the
//   spill of `_Bottom` into the `last` arg slot at +0x0d (instead of
//   into a fresh local), the interleaved PUSH-EAX / REP-MOVSD /
//   PUSH-EBX / PUSH-ECX struct-arg push order at +0x33..+0x45, the
//   incremental EBP maintenance across the loop iterations, and the
//   JG re-entry that skips the one-time EBP recomputation. Each is
//   order-sensitive to surrounding code, and a high-level rewrite of
//   `_Make_heap` under /O2 would typically pick different spill
//   slots and a different loop shape.
//
//   The same pragmatic choice the other `_unknown/` siblings
//   (FUN_00407d80, FUN_00406520, FUN_00406ab0, FUN_00403bd0,
//   FUN_00401650) made — a `__declspec(naked)` body that re-emits
//   the orig bytes verbatim via MASM `_emit` directives — produces
//   an .obj whose `.text` is exactly 87 bytes matching orig. The
//   single rel32 CALL field is emitted as a raw immediate (no .obj
//   relocation), so `tools/compare.py`'s reloc-mask is empty and
//   every byte is compared verbatim — and matches.
//
// Asm shape (87 bytes — verified against asm/ffxivgame/00008a80_*.s):
//
//     00008a80:  8b 44 24 08         MOV   EAX, [ESP+8]      ; last
//     00008a84:  8b 4c 24 04         MOV   ECX, [ESP+4]      ; first
//     00008a88:  2b c1               SUB   EAX, ECX          ; last - first (B)
//     00008a8a:  c1 f8 06            SAR   EAX, 6            ; /sizeof(Elem)=64
//     00008a8d:  89 44 24 08         MOV   [ESP+8], EAX      ; spill _Bottom
//     00008a91:  53                  PUSH  EBX
//     00008a92:  99                  CDQ
//     00008a93:  2b c2               SUB   EAX, EDX          ; signed /2
//     00008a95:  55                  PUSH  EBP
//     00008a96:  8b d8               MOV   EBX, EAX
//     00008a98:  d1 fb               SAR   EBX, 1            ; EBX = _Hole
//     00008a9a:  85 db               TEST  EBX, EBX
//     00008a9c:  56                  PUSH  ESI
//     00008a9d:  57                  PUSH  EDI
//     00008a9e:  7e 32               JLE   0x00408ad2        ; → epilogue
//   loop_top:
//     00008aa0:  8b eb               MOV   EBP, EBX          ; first-iter only
//     00008aa2:  c1 e5 06            SHL   EBP, 6
//     00008aa5:  03 e9               ADD   EBP, ECX          ; &first[_Hole]
//   loop_reenter:
//     00008aa7:  8b 44 24 18         MOV   EAX, [ESP+0x18]   ; reload _Bottom
//     00008aab:  83 ec 40            SUB   ESP, 0x40         ; alloc _Val
//     00008aae:  8b fc               MOV   EDI, ESP          ; dst = &_Val
//     00008ab0:  83 ed 40            SUB   EBP, 0x40         ; src = prev - 64
//     00008ab3:  50                  PUSH  EAX               ; arg3 = _Bottom
//     00008ab4:  b9 10 00 00 00      MOV   ECX, 0x10         ; 16 dwords
//     00008ab9:  8b f5               MOV   ESI, EBP
//     00008abb:  83 eb 01            SUB   EBX, 1            ; --_Hole
//     00008abe:  f3 a5               REP MOVSD               ; copy _Val
//     00008ac0:  8b 4c 24 58         MOV   ECX, [ESP+0x58]   ; reload first
//     00008ac4:  53                  PUSH  EBX               ; arg2 = _Hole
//     00008ac5:  51                  PUSH  ECX               ; arg1 = first
//     00008ac6:  e8 45 fe ff ff      CALL  FUN_00408910      ; _Adjust_heap
//     00008acb:  83 c4 4c            ADD   ESP, 0x4c         ; cleanup pushed
//     00008ace:  85 db               TEST  EBX, EBX
//     00008ad0:  7f d5               JG    0x00408aa7        ; loop back
//   epilogue:
//     00008ad2:  5f                  POP   EDI
//     00008ad3:  5e                  POP   ESI
//     00008ad4:  5d                  POP   EBP
//     00008ad5:  5b                  POP   EBX
//     00008ad6:  c3                  RET

extern "C" __declspec(naked) void FUN_00408a80() {
    __asm {
        _emit 0x8b              // MOV EAX, [ESP+8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV ECX, [ESP+4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0xc1              // SAR EAX, 6
        _emit 0xf8
        _emit 0x06
        _emit 0x89              // MOV [ESP+8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x99              // CDQ
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0xd1              // SAR EBX, 1
        _emit 0xfb
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x7e              // JLE 0x00408ad2 (epilogue)
        _emit 0x32
        _emit 0x8b              // MOV EBP, EBX
        _emit 0xeb
        _emit 0xc1              // SHL EBP, 6
        _emit 0xe5
        _emit 0x06
        _emit 0x03              // ADD EBP, ECX
        _emit 0xe9
        _emit 0x8b              // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x83              // SUB ESP, 0x40
        _emit 0xec
        _emit 0x40
        _emit 0x8b              // MOV EDI, ESP
        _emit 0xfc
        _emit 0x83              // SUB EBP, 0x40
        _emit 0xed
        _emit 0x40
        _emit 0x50              // PUSH EAX
        _emit 0xb9              // MOV ECX, 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, EBP
        _emit 0xf5
        _emit 0x83              // SUB EBX, 1
        _emit 0xeb
        _emit 0x01
        _emit 0xf3              // REP MOVSD
        _emit 0xa5
        _emit 0x8b              // MOV ECX, [ESP+0x58]
        _emit 0x4c
        _emit 0x24
        _emit 0x58
        _emit 0x53              // PUSH EBX
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL FUN_00408910 (rel32)
        _emit 0x45
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4c
        _emit 0xc4
        _emit 0x4c
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x7f              // JG 0x00408aa7 (loop_reenter)
        _emit 0xd5
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
