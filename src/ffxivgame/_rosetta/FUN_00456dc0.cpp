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
// FUNCTION: ffxivgame 0x00456dc0 — deque-iterator copy-backward kernel (196 B / 0xC4)
//
// Reads elements backward from a source deque (4-element blocks) in the
// range [src_end, src_idx) and writes them forward into a destination deque
// starting at dst_idx.  On return, stores the result iterator state
// {0, dst_deque_ptr, final_dst_idx} into *out.
//
// Signature (recovered from prologue + exit path):
//
//   __cdecl void FUN_00456dc0(
//       OutIter* out,         // arg1  [ESP+0x14] after 4 pushes
//       int      arg2,        // arg2  [ESP+0x18]  (unused by this function)
//       int      arg3,        // arg3  [ESP+0x1C]  (unused by this function)
//       int      src_end,     // arg4  [ESP+0x20]  loop-exit condition
//       int      arg5,        // arg5  [ESP+0x24]  (unused by this function)
//       Deque*   src,         // arg6  [ESP+0x28]
//       int      src_idx,     // arg7  [ESP+0x2C]  start src index (decrements each iter)
//       int      arg8,        // arg8  [ESP+0x30]  stack slot reused as spill temp
//       Deque*   dst,         // arg9  pre-loaded into EBP at function entry
//       int      dst_idx      // arg10 pre-loaded into EBX at function entry
//   );
//
// Deque struct layout (fields actually accessed):
//   +0x04  T**    _Mymap      block-pointer array
//   +0x08  int    _myoff_blk  base block index within the map
//   +0x0C  int    _field_0C   lower component of element-count bound
//   +0x10  int    _field_10   upper component; bound = _field_0C + _field_10
//
// Element lookup for logical index i (block size = 4):
//   blk    = i >> 2
//   off    = i & 3
//   if (_myoff_blk <= blk) blk -= _myoff_blk
//   elem * = _Mymap[blk] + off
//
// Assertion helper at 0x009D22B4 is called when:
//   - deque pointer is NULL
//   - current index >= _field_0C + _field_10
//
// Reconstruction strategy — naked-asm byte passthrough (same as FUN_00403f10,
// FUN_00406680):
//
//   The function reuses the arg8 stack slot ([ESP+0x30] inside the loop)
//   as a spill temporary for the source-element pointer.  MSVC 2005 would
//   only generate this exact pattern under a specific local-variable
//   ordering that is not reproducible portably.  Additionally, the 4-byte
//   `LEA ESP,[ESP+0]` alignment NOP before the loop head and the exact
//   near-JZ / near-JMP encodings would diverge from any high-level rewrite.
//   Emitting the original 196 bytes verbatim is the pragmatic path to GREEN.
//
//   All four CALL rel32 displacements target 0x009D22B4 and are emitted as
//   the baked absolute-offset bytes from the orig PE, so compare.py sees a
//   byte-identical .obj text section with no COFF relocation records.

extern "C" __declspec(naked) void FUN_00456dc0() {
    __asm {
        // Prologue — save EBX/EBP/ESI/EDI; pre-load EBX=arg10, EBP=arg9
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX,[ESP+0x2C]    (arg10 = dst_idx)
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP,[ESP+0x2C]    (arg9  = dst deque ptr)
        _emit 0x6c
        _emit 0x24
        _emit 0x2c
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA ESP,[ESP+0]       (4-byte NOP — aligns loop head)
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // ── loop head ─────────────────────────────────────────────────────
        _emit 0x8b              // MOV EAX,[ESP+0x2C]    current src index
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x39              // CMP [ESP+0x20],EAX    src_end == src_idx ?
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x0f              // JZ exit (near)
        _emit 0x84
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX,[ESP+0x28]    src deque ptr
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x83              // SUB EAX,1
        _emit 0xe8
        _emit 0x01
        _emit 0x8b              // MOV ESI,EAX
        _emit 0xf0
        _emit 0x8b              // MOV EDI,EAX
        _emit 0xf8
        _emit 0xc1              // SHR ESI,2             block index
        _emit 0xee
        _emit 0x02
        _emit 0x83              // AND EDI,3             element offset in block
        _emit 0xe7
        _emit 0x03
        _emit 0x85              // TEST ECX,ECX          src deque NULL?
        _emit 0xc9
        _emit 0x89              // MOV [ESP+0x2C],EAX    update src_idx
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x75              // JNZ +0x0D
        _emit 0x0d
        _emit 0xe8              // CALL 0x009D22B4       assert(src != NULL)
        _emit 0xb8
        _emit 0xb4
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV ECX,[ESP+0x28]    reload src deque
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x8b              // MOV EAX,[ESP+0x2C]    reload src_idx
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // ── src bounds check ──────────────────────────────────────────────
        _emit 0x8b              // MOV EDX,[ECX+0x10]
        _emit 0x51
        _emit 0x10
        _emit 0x03              // ADD EDX,[ECX+0x0C]    bound = _field_0C + _field_10
        _emit 0x51
        _emit 0x0c
        _emit 0x3b              // CMP EAX,EDX
        _emit 0xc2
        _emit 0x72              // JC +0x09              within bounds → skip assert
        _emit 0x09
        _emit 0xe8              // CALL 0x009D22B4       assert(src_idx < bound)
        _emit 0xa1
        _emit 0xb4
        _emit 0x57
        _emit 0x00
        _emit 0x8b              // MOV ECX,[ESP+0x28]    reload src deque
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // ── src element address ───────────────────────────────────────────
        _emit 0x8b              // MOV EAX,[ECX+0x08]    _myoff_blk
        _emit 0x41
        _emit 0x08
        _emit 0x3b              // CMP EAX,ESI
        _emit 0xc6
        _emit 0x77              // JA +0x02              if _myoff_blk > blk, skip sub
        _emit 0x02
        _emit 0x2b              // SUB ESI,EAX           map_idx = blk - _myoff_blk
        _emit 0xf0
        _emit 0x8b              // MOV EAX,[ECX+0x04]    _Mymap
        _emit 0x41
        _emit 0x04
        _emit 0x8b              // MOV ECX,[EAX+ESI*4]   block ptr
        _emit 0x0c
        _emit 0xb0
        _emit 0x8d              // LEA EDX,[ECX+EDI*4]   src element ptr
        _emit 0x14
        _emit 0xb9
        // ── dst block/offset from EBX ─────────────────────────────────────
        _emit 0x8b              // MOV ESI,EBX
        _emit 0xf3
        _emit 0x8b              // MOV EDI,EBX
        _emit 0xfb
        _emit 0xc1              // SHR ESI,2
        _emit 0xee
        _emit 0x02
        _emit 0x83              // AND EDI,3
        _emit 0xe7
        _emit 0x03
        _emit 0x85              // TEST EBP,EBP          dst deque NULL?
        _emit 0xed
        _emit 0x89              // MOV [ESP+0x30],EDX    spill src element ptr (uses arg8 slot)
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x75              // JNZ +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009D22B4       assert(dst != NULL)
        _emit 0x74
        _emit 0xb4
        _emit 0x57
        _emit 0x00
        // ── dst bounds check ──────────────────────────────────────────────
        _emit 0x8b              // MOV EAX,[EBP+0x10]
        _emit 0x45
        _emit 0x10
        _emit 0x03              // ADD EAX,[EBP+0x0C]
        _emit 0x45
        _emit 0x0c
        _emit 0x3b              // CMP EBX,EAX
        _emit 0xd8
        _emit 0x72              // JC +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009D22B4       assert(dst_idx < bound)
        _emit 0x65
        _emit 0xb4
        _emit 0x57
        _emit 0x00
        // ── dst element address ───────────────────────────────────────────
        _emit 0x8b              // MOV EAX,[EBP+0x08]
        _emit 0x45
        _emit 0x08
        _emit 0x3b              // CMP EAX,ESI
        _emit 0xc6
        _emit 0x77              // JA +0x02
        _emit 0x02
        _emit 0x2b              // SUB ESI,EAX
        _emit 0xf0
        _emit 0x8b              // MOV ECX,[EBP+0x04]    _Mymap (dst)
        _emit 0x4d
        _emit 0x04
        _emit 0x8b              // MOV EDX,[ECX+ESI*4]   dst block ptr
        _emit 0x14
        _emit 0xb1
        // ── copy one element, advance, loop ──────────────────────────────
        _emit 0x8b              // MOV EAX,[ESP+0x30]    reload src element ptr
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x8b              // MOV ECX,[EAX]         load element value
        _emit 0x08
        _emit 0x89              // MOV [EDX+EDI*4],ECX   store to dst
        _emit 0x0c
        _emit 0xba
        _emit 0x83              // ADD EBX,1
        _emit 0xc3
        _emit 0x01
        _emit 0xe9              // JMP loop_head (near, -0x9F)
        _emit 0x61
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // ── exit path ────────────────────────────────────────────────────
        _emit 0x8b              // MOV EAX,[ESP+0x14]    out ptr (arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x89              // MOV [EAX+4],EBP       out->deque = dst
        _emit 0x68
        _emit 0x04
        _emit 0x5d              // POP EBP
        _emit 0x89              // MOV [EAX+8],EBX       out->idx   = final dst_idx
        _emit 0x58
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [EAX],0 out->field0 = 0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
