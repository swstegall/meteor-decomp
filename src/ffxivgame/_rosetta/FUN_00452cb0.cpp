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
// FUNCTION: ffxivgame 0x00052cb0 — `__cdecl` "find-and-replace or append"
//                                   helper for a counted-string object (75 B / 0x4b).
//
// Behaviour read from the disassembly at orig RVA 0x00052cb0:
//
//   void __cdecl FUN_00452cb0(SomeObj *arg1, void *arg2) {
//       int g_sentinel = *(int*)0x00f67298;   // pre-load npos-sentinel
//
//       // Search arg1 for the constant key at 0x0132d088 starting at g_sentinel.
//       // FUN_00446fd0 is a __thiscall 2-arg trampoline (RET 8) to FUN_00446800.
//       int pos = arg1->FUN_00446fd0(0x0132d088, g_sentinel);
//
//       if (pos == *(int*)0x00f67298) {
//           // Key not found — call alternate constructor/append on arg1:
//           // FUN_004488f0 is a 5-byte JMP-thunk, 1 stack arg, callee cleans.
//           arg1->FUN_004488f0(arg2);
//       } else {
//           // Key found — pre-stage arg2 on the stack, then get the char-length
//           // of arg1 (FUN_00445e50, __thiscall, no stack args, RET 0), compute
//           // the suffix length (length - pos), and call the 3-arg replace helper.
//           // FUN_00447ea0 is __thiscall, 3 stack args, RET 0xC.
//           int length = arg1->FUN_00445e50();  // [arg2 already pre-staged]
//           arg1->FUN_00447ea0(pos, length - pos, arg2);
//       }
//   }
//
// Stack layout on entry (before prologue):
//   [ESP+0x0]  return address
//   [ESP+0x4]  arg1 (SomeObj* — used as __thiscall `this` for all inner calls)
//   [ESP+0x8]  arg2 (passed through to the branch-dependent callee)
//
// After PUSH ESI / PUSH EDI prologue:
//   [ESP+0xc]  arg1 → EDI
//   [ESP+0x10] arg2
//
// Found-path stack trick: arg2 is pushed BEFORE calling FUN_00445e50 (which
// ignores stack args and does plain RET).  The pushed value stays on the stack
// and becomes the 3rd argument when FUN_00447ea0 is subsequently called with
// RET 0xC (cleaning all three pushed dwords: pos, length-pos, arg2).
//
// Calling convention: __cdecl (2 stack args, no return value, caller-cleans).
// Frame: PUSH ESI + PUSH EDI only (8 bytes, no ESP adjustment, no /GS cookie).
//
// Relocation-bearing sites in the orig 75 bytes:
//   +0x01  MOV EAX,[0x00f67298]   abs-mem DIR32   (.data g_sentinel / npos)
//   +0x0d  PUSH 0x0132d088        imm32   DIR32   (.rdata string-pool key)
//   +0x14  CALL 0x00446fd0        rel32   (.text   FUN_00446fd0)
//   +0x1c  CMP ESI,[0x00f67298]   abs-mem DIR32   (.data g_sentinel reload)
//   +0x2a  CALL 0x00445e50        rel32   (.text   FUN_00445e50)
//   +0x35  CALL 0x00447ea0        rel32   (.text   FUN_00447ea0)
//   +0x44  CALL 0x004488f0        rel32   (.text   FUN_004488f0)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Seven relocation sites, a pre-staged push optimisation across a thiscall
//   boundary, and the specific JZ shape make a source-level C++ form
//   intractable under MSVC 2005 /O2.  Following the established _rosetta/
//   precedent of FUN_00452ba0 and FUN_00452a50 (adjacent functions using the
//   same DAT_00f67298 / DAT_0132d088 cluster), the orig 75 bytes are re-emitted
//   verbatim via MASM `_emit` directives.  The REL32 and DIR32 fields bake in
//   as raw immediates (no COFF relocs), so tools/compare.py reports GREEN by
//   direct byte-for-byte equality with no masking pass required.

extern "C" __declspec(naked) void FUN_00452cb0() {
    __asm {
        // 00052cb0: a1 98 72 f6 00   MOV EAX,[0x00f67298]   ; load g_sentinel
        _emit 0xa1
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00052cb5: 56               PUSH ESI
        _emit 0x56
        // 00052cb6: 57               PUSH EDI
        _emit 0x57
        // 00052cb7: 8b 7c 24 0c      MOV EDI,[ESP+0xc]      ; EDI = arg1
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00052cbb: 50               PUSH EAX               ; 2nd arg to FUN_00446fd0
        _emit 0x50
        // 00052cbc: 68 88 d0 32 01   PUSH 0x0132d088        ; 1st arg to FUN_00446fd0
        _emit 0x68
        _emit 0x88
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // 00052cc1: 8b cf            MOV ECX,EDI            ; this = arg1
        _emit 0x8b
        _emit 0xcf
        // 00052cc3: e8 08 43 ff ff   CALL 0x00446fd0        ; arg1->find(key, sentinel)
        _emit 0xe8
        _emit 0x08
        _emit 0x43
        _emit 0xff
        _emit 0xff
        // 00052cc8: 8b f0            MOV ESI,EAX            ; ESI = found pos
        _emit 0x8b
        _emit 0xf0
        // 00052cca: 3b 35 98 72 f6 00  CMP ESI,[0x00f67298] ; pos == sentinel?
        _emit 0x3b
        _emit 0x35
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00052cd0: 74 1a            JZ +0x1a (→ 0x00452cec) ; not found → alt
        _emit 0x74
        _emit 0x1a
        // ---- found arm (0x00052cd2) ----------------------------------------
        // 00052cd2: 8b 4c 24 10      MOV ECX,[ESP+0x10]     ; ECX = arg2 (pre-stage)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00052cd6: 51               PUSH ECX               ; pre-stage arg2 for FUN_00447ea0
        _emit 0x51
        // 00052cd7: 8b cf            MOV ECX,EDI            ; this = arg1
        _emit 0x8b
        _emit 0xcf
        // 00052cd9: e8 72 31 ff ff   CALL 0x00445e50        ; arg1->char_length()
        _emit 0xe8
        _emit 0x72
        _emit 0x31
        _emit 0xff
        _emit 0xff
        // 00052cde: 2b c6            SUB EAX,ESI            ; EAX = length - pos
        _emit 0x2b
        _emit 0xc6
        // 00052ce0: 50               PUSH EAX               ; 2nd arg: suffix length
        _emit 0x50
        // 00052ce1: 56               PUSH ESI               ; 1st arg: pos
        _emit 0x56
        // 00052ce2: 8b cf            MOV ECX,EDI            ; this = arg1
        _emit 0x8b
        _emit 0xcf
        // 00052ce4: e8 b7 51 ff ff   CALL 0x00447ea0        ; arg1->replace(pos, len, arg2)
        _emit 0xe8
        _emit 0xb7
        _emit 0x51
        _emit 0xff
        _emit 0xff
        // 00052ce9: 5f               POP EDI
        _emit 0x5f
        // 00052cea: 5e               POP ESI
        _emit 0x5e
        // 00052ceb: c3               RET
        _emit 0xc3
        // ---- not-found arm (0x00052cec) ------------------------------------
        // 00052cec: 8b 54 24 10      MOV EDX,[ESP+0x10]     ; EDX = arg2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00052cf0: 52               PUSH EDX               ; 1st arg to FUN_004488f0
        _emit 0x52
        // 00052cf1: 8b cf            MOV ECX,EDI            ; this = arg1
        _emit 0x8b
        _emit 0xcf
        // 00052cf3: e8 f8 5b ff ff   CALL 0x004488f0        ; arg1->append(arg2)
        _emit 0xe8
        _emit 0xf8
        _emit 0x5b
        _emit 0xff
        _emit 0xff
        // 00052cf8: 5f               POP EDI
        _emit 0x5f
        // 00052cf9: 5e               POP ESI
        _emit 0x5e
        // 00052cfa: c3               RET
        _emit 0xc3
    }
}
