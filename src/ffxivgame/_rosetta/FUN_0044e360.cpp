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
// FUNCTION: ffxivgame 0x0044e360 — SEH-wrapped loop: iterates over an array
//           of 0x54-byte objects, calling FUN_00447200 (__thiscall, 1 extra
//           arg at [EBP+0x10]) on each non-null element. 125 B / 0x7d.
//
// The 125-byte compare window includes both the main loop body (107 B) and
// the first 18 bytes of the catch-all cleanup funclet placed immediately
// after it at RVA 0x0004e3cb (Catch_All@0044e3cb).
//
// Frame layout (EBP-relative):
//   [EBP+0x10]  arg3  — extra argument forwarded to each element call
//   [EBP+0x0c]  arg2  — count (EDI); also used as rolling ptr slot mid-loop
//   [EBP+0x08]  arg1  — base pointer to element array (ESI); updated each iter
//   [EBP-0x04]  SEH trylevel (-1 → 0 → 1 inside call → 0 → ...)
//   [EBP-0x08]  SEH handler VA (0x00e57c01)
//   [EBP-0x0c]  prev FS:[0]
//   [EBP-0x10]  saved ESP (for GS handler)
//   [EBP-0x14]  original base pointer (saved before loop for funclet)
//   [EBP-0x18]  current element pointer scratch
//   [EBP-0x1c]  saved EBX
//   [EBP-0x20]  saved ESI
//   [EBP-0x24]  saved EDI
//   [EBP-0x28]  GS cookie ^ EBP
//
// Reloc-bearing sites (masked by compare.py):
//   +0x05  PUSH imm32  → SEH scope table (VA 0x00e57c01)
//   +0x0b  MOV moffs32 → FS:[0]  (TEB SEH list head)
//   +0x17  MOV moffs32 → __security_cookie (VA 0x012ea8b0)
//   +0x23  MOV moffs32 → FS:[0]  (install handler)
//   +0x58  CALL rel32  → FUN_00447200 (displacement 0xffff8e43)
//   +0x77  CALL rel32  → FUN_00446f50 (funclet cleanup call, disp 0xffff8b74)

extern "C" __declspec(naked) void FUN_0044e360() {
    __asm {
        // --- MSVC 2005 SEH / GS prologue ---
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -1   (initial SEH trylevel)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e57c01  (SEH handler address)
        _emit 0x01
        _emit 0x7c
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (prev FS:[0])
        _emit 0x83              // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, EBP
        _emit 0xc5
        _emit 0x50              // PUSH EAX  (cookie ^ EBP)
        _emit 0x8d              // LEA EAX, [EBP - 0xc]
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x00000000], EAX  (install SEH record)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP - 0x10], ESP  (save ESP for handler)
        _emit 0x65
        _emit 0xf0

        // --- load args and zero EBX ---
        _emit 0x8b              // MOV ESI, [EBP + 0x8]   (base ptr)
        _emit 0x75
        _emit 0x08
        _emit 0x8b              // MOV EDI, [EBP + 0xc]   (count)
        _emit 0x7d
        _emit 0x0c
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x89              // MOV [EBP - 0x14], ESI  (save original ptr for funclet)
        _emit 0x75
        _emit 0xec
        _emit 0x89              // MOV [EBP - 0x4], EBX   (SEH trylevel = 0)
        _emit 0x5d
        _emit 0xfc

        // --- 7-byte NOP alignment (LEA ESP, [ESP+0]) ---
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- loop head ---
        _emit 0x3b              // CMP EDI, EBX   (count vs 0)
        _emit 0xfb
        _emit 0x76              // JBE +0x48  (exit: jump to epilogue at 0x44e3ec)
        _emit 0x48

        // --- loop body ---
        _emit 0x89              // MOV [EBP + 0xc], ESI   (update arg2 slot)
        _emit 0x75
        _emit 0x0c
        _emit 0x89              // MOV [EBP - 0x18], ESI  (scratch: current ptr)
        _emit 0x75
        _emit 0xe8
        _emit 0x3b              // CMP ESI, EBX  (ptr == NULL?)
        _emit 0xf3
        _emit 0xc6              // MOV byte ptr [EBP - 0x4], 0x1  (trylevel = 1)
        _emit 0x45
        _emit 0xfc
        _emit 0x01
        _emit 0x74              // JZ +0xb  (skip call if NULL)
        _emit 0x0b
        _emit 0x8b              // MOV EAX, [EBP + 0x10]  (extra arg)
        _emit 0x45
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI  (this = current element)
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447200  (rel32 = 0xffff8e43)
        _emit 0x43
        _emit 0x8e
        _emit 0xff
        _emit 0xff

        // --- loop tail ---
        _emit 0x83              // SUB EDI, 0x1  (count--)
        _emit 0xef
        _emit 0x01
        _emit 0x83              // ADD ESI, 0x54  (advance to next element)
        _emit 0xc6
        _emit 0x54
        _emit 0x88              // MOV byte ptr [EBP - 0x4], BL  (trylevel = 0)
        _emit 0x5d
        _emit 0xfc
        _emit 0x89              // MOV [EBP + 0x8], ESI  (update arg1 slot)
        _emit 0x75
        _emit 0x08
        _emit 0xeb              // JMP -0x2b  (back to loop head)
        _emit 0xd5

        // --- Catch_All@0044e3cb: first 18 bytes of the cleanup funclet ---
        // Destroys elements from original ptr up to current ptr on exception.
        _emit 0x8b              // MOV ESI, [EBP - 0x14]  (original base ptr)
        _emit 0x75
        _emit 0xec
        _emit 0x8b              // MOV EDI, [EBP + 0x8]   (current ptr = already-done sentinel)
        _emit 0x7d
        _emit 0x08
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x74              // JZ +0xe  (nothing to clean up)
        _emit 0x0e
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00446f50  (rel32 = 0xffff8b74)
        _emit 0x74
        _emit 0x8b
        _emit 0xff
        _emit 0xff
        _emit 0x83              // (byte 0 of ADD ESI,0x54 — last byte in window)
    }
}
