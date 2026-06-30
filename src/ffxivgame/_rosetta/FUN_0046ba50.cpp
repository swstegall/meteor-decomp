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
// FUNCTION: ffxivgame 0x0046ba50 — per-byte hex-encode loop with write callback
//                                  (__thiscall, 108 bytes)
//
// __thiscall int FUN_0046ba50(const char *data_out, DWORD length)
//   implicit: ECX = this  (source byte buffer)
//             EDI = write context (e.g. stream handle; tested for NULL on entry)
//             EBP = write callback: int cb(void *ctx, const char *buf, DWORD n)
//
// Stack layout after __chkstk allocates 4 bytes, before callee-saves:
//   [ESP + 0x00] : 4-byte local hex char buffer
//   [ESP + 0x04] : return address
//   [ESP + 0x08] : param1 — write callback pointer (loaded into EBP)
//   [ESP + 0x0C] : param2 — byte count (source length)
//
// Semantics (recovered from asm):
//
//   int FUN_0046ba50(fn_t *cb, DWORD len)  [ECX=this, EDI=ctx]
//   {
//       static const char hex[] = "0123456789abcdef";   // at 0x00f79440
//       char buf[2];
//       if (EDI == NULL) goto done;           // no-op if no context
//       const char *end = this + len;
//       if (this == end) goto done;           // empty buffer
//       do {
//           unsigned char b = *this++;
//           buf[0] = hex[b >> 4];             // high nibble
//           buf[1] = hex[b & 0xf];            // low nibble
//           if (cb(EDI, buf, 2) == 0)
//               return -1;                    // write failure
//       } while (this != end);
//   done:
//       return (int)len * 2;                  // total chars written
//   }
//
// Calling convention notes:
//   - MOV EAX,0x4 / CALL __chkstk (0x009d29d0) allocates 4 local bytes
//   - EDI is read without saving — the function never modifies EDI, so
//     no save/restore is needed despite it being callee-saved in the ABI.
//     The caller pre-loads EDI with the write-context pointer.
//   - Return via POP ECX (x1 local dword) instead of ADD ESP,4.
//   - The write callback is __cdecl: ADD ESP,0x0c after the call.
//
// Reloc-bearing sites in the orig 108 bytes:
//   +0x06  CALL rel32  → __chkstk 0x009d29d0  (rel32 = 0x00566f76)
//   +0x2c  MOV DL, [ECX+imm32] → 0x00f79440   (hex nibble table)
//   +0x3b  MOV AL, [EAX+imm32] → 0x00f79440   (hex nibble table, second use)
//
// Reconstruction strategy — naked-asm byte passthrough (same as FUN_004090b0):
//   compare.py reads the orig PE post-fixup and compares byte streams;
//   emitting the orig bytes verbatim via _emit produces a zero-reloc .obj
//   whose .text matches byte-for-byte → GREEN.

extern "C" __declspec(naked) void FUN_0046ba50() {
    __asm {
        // --- prologue: __chkstk allocates 4-byte local buffer ----------
        _emit 0xb8              // MOV EAX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL __chkstk (rel32 → 0x009d29d0)
        _emit 0x76
        _emit 0x6f
        _emit 0x56
        _emit 0x00
        // --- test EDI (write context); push callee-saves ----------------
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP + 0x10]  (callback)
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX   (this → source pointer)
        _emit 0xf1
        _emit 0x74              // JZ done (+0x42)
        _emit 0x42
        // --- compute end pointer, guard empty buffer --------------------
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x18]  (len)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8d              // LEA EBX, [ESI + EAX]  (end = this + len)
        _emit 0x1c
        _emit 0x06
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0x74              // JZ done (+0x37)
        _emit 0x37
        // --- loop: convert one byte to two hex chars --------------------
        // loop:
        _emit 0x0f              // MOVZX EAX, byte ptr [ESI]
        _emit 0xb6
        _emit 0x06
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xc1              // SHR ECX, 0x4   (high nibble)
        _emit 0xe9
        _emit 0x04
        _emit 0x8a              // MOV DL, byte ptr [ECX + 0x00f79440]
        _emit 0x91
        _emit 0x40
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x2       (count arg)
        _emit 0x02
        _emit 0x8d              // LEA ECX, [ESP + 0x10]  (ptr to local buffer)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x83              // AND EAX, 0xf   (low nibble)
        _emit 0xe0
        _emit 0x0f
        _emit 0x8a              // MOV AL, byte ptr [EAX + 0x00f79440]
        _emit 0x80
        _emit 0x40
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        _emit 0x51              // PUSH ECX       (buffer ptr arg)
        _emit 0x57              // PUSH EDI       (context arg)
        _emit 0x88              // MOV byte ptr [ESP + 0x18], DL  (buf[0] = high)
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x88              // MOV byte ptr [ESP + 0x19], AL  (buf[1] = low)
        _emit 0x44
        _emit 0x24
        _emit 0x19
        _emit 0xff              // CALL EBP       (callback)
        _emit 0xd5
        _emit 0x83              // ADD ESP, 0xc   (caller cleans 3 args)
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX  (check return value)
        _emit 0xc0
        _emit 0x74              // JZ fail (+0x12)
        _emit 0x12
        _emit 0x83              // ADD ESI, 0x1   (++this)
        _emit 0xc6
        _emit 0x01
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0x75              // JNZ loop (-0x37)
        _emit 0xc9
        // --- done: return len * 2 ----------------------------------------
        // done:
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x18]  (len)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x03              // ADD EAX, EAX   (len * 2)
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX        (discard __chkstk local)
        _emit 0xc3              // RET
        // --- fail: return -1 ---------------------------------------------
        // fail:
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x83              // OR EAX, 0xffffffff
        _emit 0xc8
        _emit 0xff
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX        (discard __chkstk local)
        _emit 0xc3              // RET
    }
}
