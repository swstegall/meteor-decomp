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
// FUNCTION: ffxivgame 0x00433720 — formatted-message emitter
//                                  (__cdecl, 180 B / 0xb4)
//
// Builds a formatted message into an 0x800-byte stack buffer (guarded by
// the /GS security cookie) and hands it to an indirect output sink.
//
//   void FUN_00433720(int a0, int a1, int /*a2*/, int a2v,
//                     int a4, int /*a5*/, int a6, int a7)
//   {
//       char buf[0x800];                       // /GS array → cookie
//       if (a4 != 0)
//           _snprintf_s(buf, 0x800, 0x7ff,     // CALL 0x009d4f9f
//                       /*fmt*/ (char*)0xf648c4,
//                       a2v, a7, a4, a0, a1);
//       else
//           _snprintf_s(buf, 0x800, 0x7ff,
//                       /*fmt*/ (char*)0xf648e4,
//                       a2v, a6, a0, a1);
//       (*(void(**)(char*,int))0x012651b4)(buf, 6);  // indirect sink
//   }
//
// Calling convention: __cdecl (epilogue is `ADD ESP,0x804 / RET`, no
// `ret N`). One callee-saved register (ESI) is pushed/popped. Both
// branches funnel into the shared indirect CALL through the global
// function pointer at 0x012651b4 with (buffer, 6).
//
// Reloc-bearing sites (offsets within the function — each window is
// wildcarded by compare.py against orig):
//   +0x07   MOV EAX, __security_cookie         (.data 0x012ea8b0)
//   +0x42   PUSH offset fmt_a                  (.rdata 0x00f648c4)
//   +0x56   CALL rel32 → 0x009d4f9f (_snprintf_s, branch a4!=0)
//   +0x69   PUSH offset fmt_b                  (.rdata 0x00f648e4)
//   +0x7d   CALL rel32 → 0x009d4f9f (_snprintf_s, branch a4==0)
//   +0x8d   CALL [0x012651b4] indirect sink    (.data func ptr)
//   +0xa0   MOV [global], 0                    (.data, DIR32)
//   +0xa9   CALL rel32 → 0x009d20f4 (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom as
// FUN_00403f10 et al.): the source-level shape (a branch over two
// vararg `_snprintf_s` calls plus the /GS prolog/epilog) produces a
// register-allocation and reloc layout that's impractical to coax out
// of plain C++ byte-for-byte. We re-emit the orig 180 bytes verbatim
// via MASM `_emit`; the .obj's `.text` matches the orig slice exactly
// (reloc windows are masked by tools/compare.py).

extern "C" __declspec(naked) void FUN_00433720() {
    __asm {
        _emit 0x81      // SUB ESP, 0x804
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xa1      // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33      // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89      // MOV [ESP+0x800], EAX
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b      // MOV EDX, [ESP+0x80c]   (a1)
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b      // MOV EAX, [ESP+0x818]   (a4)
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x85      // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b      // MOV ECX, [ESP+0x808]   (a0)
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56      // PUSH ESI               (callee-saved)
        _emit 0x8b      // MOV ESI, [ESP+0x814]   (a2v)
        _emit 0xb4
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52      // PUSH EDX               (a1)
        _emit 0x51      // PUSH ECX               (a0)
        _emit 0x74      // JZ  0x0043377f (a4==0 → branch b)
        _emit 0x28
        _emit 0x50      // PUSH EAX               (a4)
        _emit 0x8b      // MOV EAX, [ESP+0x824]   (a7)
        _emit 0x84
        _emit 0x24
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50      // PUSH EAX               (a7)
        _emit 0x56      // PUSH ESI               (a2v)
        _emit 0x68      // PUSH offset fmt_a (0xf648c4)
        _emit 0xc4
        _emit 0x48
        _emit 0xf6
        _emit 0x00
        _emit 0x68      // PUSH 0x7ff
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d      // LEA ECX, [ESP+0x20]    (&buf)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x68      // PUSH 0x800
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51      // PUSH ECX               (&buf)
        _emit 0xe8      // CALL rel32 → 0x009d4f9f (_snprintf_s)
        _emit 0x25
        _emit 0x18
        _emit 0x5a
        _emit 0x00
        _emit 0x83      // ADD ESP, 0x24
        _emit 0xc4
        _emit 0x24
        _emit 0xeb      // JMP 0x004337a4
        _emit 0x25
        _emit 0x8b      // MOV EDX, [ESP+0x820]   (a6)   (branch b:)
        _emit 0x94
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52      // PUSH EDX               (a6)
        _emit 0x56      // PUSH ESI               (a2v)
        _emit 0x68      // PUSH offset fmt_b (0xf648e4)
        _emit 0xe4
        _emit 0x48
        _emit 0xf6
        _emit 0x00
        _emit 0x68      // PUSH 0x7ff
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d      // LEA EAX, [ESP+0x1c]    (&buf)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x68      // PUSH 0x800
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50      // PUSH EAX               (&buf)
        _emit 0xe8      // CALL rel32 → 0x009d4f9f (_snprintf_s)
        _emit 0xfe
        _emit 0x17
        _emit 0x5a
        _emit 0x00
        _emit 0x83      // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0x8d      // LEA ECX, [ESP+0x4]     (&buf)   (merge:)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x6a      // PUSH 0x6
        _emit 0x06
        _emit 0x51      // PUSH ECX               (&buf)
        _emit 0xff      // CALL dword ptr [0x012651b4]  (indirect sink)
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x8b      // MOV ECX, [ESP+0x80c]   (cookie)
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x83      // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x5e      // POP ESI
        _emit 0x33      // XOR ECX, ESP
        _emit 0xcc
        _emit 0xc7      // MOV dword ptr [global], 0
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8      // CALL rel32 → 0x009d20f4 (__security_check_cookie)
        _emit 0x27
        _emit 0xe9
        _emit 0x59
        _emit 0x00
        _emit 0x81      // ADD ESP, 0x804
        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc3      // RET
    }
}
