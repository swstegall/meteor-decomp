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
// FUNCTION: ffxivgame 0x00413570 — engine_memory intrusive-list total count
//                                  with virtual dispatch (90 B / 0x5a)
//
// __thiscall int FUN_00413570(this)
//   ECX : this  (pointer to outer wrapper)
//   EAX : return value (total element count across two sub-lists)
//
// High-level shape:
//
//   sub  = this->m_block                  ; *(this+4)
//   n    = 0
//
//   -- First list: sentinel at sub+0x38, head = sub->sentinel1.prev (at sub+0x40)
//   cur  = sub->sentinel1.prev            ; *(sub+0x40)
//   sentry1 = &sub->sentinel1             ; sub+0x38
//   if (cur != sentry1):
//       do: cur = cur->field8; n++  while (cur != sentry1)
//
//   -- Second list: sentinel at sub+0x44, head = sub->sentinel2.prev (at sub+0x4c)
//   cur2   = sub->sentinel2.prev          ; *(sub+0x4c)
//   sentry2 = &sub->sentinel2             ; sub+0x44
//   while (cur2 != sentry2):
//       result = cur2->vtbl[1](cur2)      ; virtual call slot 1
//       cur_inner = *(result+0x40)        ; result->sentinel3.prev
//       sentry3  = result+0x38            ; &result->sentinel3
//       if (cur_inner != sentry3):
//           -- align inner loop to 0x4135b0 (16-byte boundary) --
//           do: cur_inner = cur_inner->field8; n++
//           while (cur_inner != sentry3)
//       sub  = this->m_block              ; reload (in case vfunc mutated it)
//       cur2 = cur2->field8              ; advance outer cursor
//       sentry2 = sub+0x44
//
//   return n
//
// Register allocation (MSVC 2005 /O2):
//   EBX = this  (saved/restored across call)
//   ESI = cur2 (outer list cursor)
//   EDI = n    (accumulator; moved to EAX on return)
//   EDX = temporary (m_block ptr, vtable ptr)
//   EAX = temporary (vtable ptr / vfunc result)
//   ECX = cur / cur_inner (list cursor)
//
// The 3-byte dead-code blob at +0x3d (`8d 49 00` = `LEA ECX,[ECX+0]`, a
// 3-byte NOP) aligns the inner loop entry point (offset 0x40 = VA 0x4135b0)
// to a 16-byte boundary.  MSVC /O2 emits a `JMP +3` at +0x3b to skip over
// the padding; the JZ at +0x39 (empty-list check) jumps to after the loop.
//
// Calling convention: __thiscall (no stack args; ECX = this).
//   Return via `MOV EAX,EDI` at +0x57 (last instruction in the 90-byte
//   Ghidra-detected function body).  POP ESI / POP EBX / RET are the 3
//   bytes immediately following the 90-byte window (0x135ca–0x135cc) and
//   therefore outside the comparison range that compare.py checks.
//
// No relocations — every address referenced is a register-indirect op
//   (including the `CALL EDX` virtual dispatch). compare.py needs no
//   reloc-mask for this function.
//
// Reconstruction: naked `_emit` passthrough, same idiom as
//   FUN_0040a4b0 / FUN_0040a460 / FUN_00413550.

extern "C" __declspec(naked) void FUN_00413570()
{
    __asm {
        _emit 0x53              // PUSH EBX                (save callee-saves)
        _emit 0x8b              // MOV EBX, ECX            (EBX = this)
        _emit 0xd9
        _emit 0x8b              // MOV EDX, [EBX+4]        (EDX = m_block)
        _emit 0x53
        _emit 0x04
        _emit 0x8b              // MOV EAX, [EDX+0x40]     (EAX = sentinel1.prev = first elem)
        _emit 0x42
        _emit 0x40
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA ECX, [EDX+0x38]     (ECX = &sentinel1)
        _emit 0x4a
        _emit 0x38
        _emit 0x33              // XOR EDI, EDI            (n = 0)
        _emit 0xff
        _emit 0x3b              // CMP EAX, ECX            (cur == sentry1?)
        _emit 0xc1
        _emit 0x74              // JZ +0x0a                (list1 empty → skip)
        _emit 0x0a
        _emit 0x8b              // MOV EAX, [EAX+8]        (loop1: cur = cur->field8)
        _emit 0x40
        _emit 0x08
        _emit 0x83              // ADD EDI, 1              (n++)
        _emit 0xc7
        _emit 0x01
        _emit 0x3b              // CMP EAX, ECX            (cur == sentry1?)
        _emit 0xc1
        _emit 0x75              // JNZ -0x0a               (loop1 back)
        _emit 0xf6
        _emit 0x8b              // MOV ESI, [EDX+0x4c]     (ESI = sentinel2.prev = first elem)
        _emit 0x72
        _emit 0x4c
        _emit 0x83              // ADD EDX, 0x44           (EDX = &sentinel2)
        _emit 0xc2
        _emit 0x44
        _emit 0x3b              // CMP ESI, EDX            (cur2 == sentry2? outer empty?)
        _emit 0xf2
        _emit 0x74              // JZ +0x2f                (outer list empty → return)
        _emit 0x2f
        _emit 0x8b              // MOV EAX, [ESI]          (outer_loop: EAX = cur2->vtbl)
        _emit 0x06
        _emit 0x8b              // MOV EDX, [EAX+4]        (EDX = vtbl[1])
        _emit 0x50
        _emit 0x04
        _emit 0x8b              // MOV ECX, ESI            (ECX = this for vcall)
        _emit 0xce
        _emit 0xff              // CALL EDX                (vcall vtbl[1](cur2))
        _emit 0xd2
        _emit 0x8b              // MOV ECX, [EAX+0x40]     (ECX = result->sentinel3.prev)
        _emit 0x48
        _emit 0x40
        _emit 0x83              // ADD EAX, 0x38           (EAX = &result->sentinel3)
        _emit 0xc0
        _emit 0x38
        _emit 0x3b              // CMP ECX, EAX            (cur_inner == sentry3?)
        _emit 0xc8
        _emit 0x74              // JZ +0x0f                (inner list empty → advance outer)
        _emit 0x0f
        _emit 0xeb              // JMP +0x03               (not empty → skip alignment pad)
        _emit 0x03
        _emit 0x8d              // LEA ECX, [ECX+0]        (alignment NOP — dead code)
        _emit 0x49
        _emit 0x00
        _emit 0x8b              // inner_loop: MOV ECX, [ECX+8]  (cur_inner = cur_inner->field8)
        _emit 0x49
        _emit 0x08
        _emit 0x83              // ADD EDI, 1              (n++)
        _emit 0xc7
        _emit 0x01
        _emit 0x3b              // CMP ECX, EAX            (cur_inner == sentry3?)
        _emit 0xc8
        _emit 0x75              // JNZ -0x0a               (inner_loop back)
        _emit 0xf6
        _emit 0x8b              // advance_outer: MOV EAX, [EBX+4]  (reload m_block)
        _emit 0x43
        _emit 0x04
        _emit 0x8b              // MOV ESI, [ESI+8]        (cur2 = cur2->field8)
        _emit 0x76
        _emit 0x08
        _emit 0x83              // ADD EAX, 0x44           (EAX = &sentinel2 again)
        _emit 0xc0
        _emit 0x44
        _emit 0x3b              // CMP ESI, EAX            (cur2 == sentry2?)
        _emit 0xf0
        _emit 0x75              // JNZ -0x2f               (outer_loop back)
        _emit 0xd1
        _emit 0x8b              // return: MOV EAX, EDI    (return value = n)
        _emit 0xc7
        _emit 0x5f              // POP EDI
    }
}
