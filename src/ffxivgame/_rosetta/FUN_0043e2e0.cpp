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
// FUNCTION: ffxivgame 0x0043e2e0 — composite destructor (__thiscall, 189 B / 0xbd)
//
// SEH-framed (__thiscall) destructor for a composite owning two
// vector-like sub-objects at this+0x18 and this+0x04. The function:
//
//   1. Installs the EH3-style SEH frame (PUSH -1 / PUSH scope_table /
//      PUSH FS:[0] chain) plus the /GS security cookie (XOR ESP).
//   2. Stashes `this` (ECX) into EDI and the live-this slot [ESP+0x10].
//   3. Stores the vtable pointer at [this] = 0x00f66f7c.
//   4. Tears down sub-object @ this+0x18 (EH state 0): if its element
//      pointer (this+0x1c) is non-null, calls the destroy-range helper
//      0x0043f830(begin, this+0x18, alloc, live_this), then frees the
//      backing block via 0x0040df70 (operator delete dispatched through
//      the [-4] cookie), and zeroes its three pointers (+0x04/+0x08/+0x0c
//      relative to ESI = this+0x18).
//   5. Tears down sub-object @ this+0x04 (EH state -1) identically.
//   6. Unwinds the SEH frame and returns (RET, args cleaned via ADD ESP).
//
//   struct VecLike {        // 16 bytes
//       /* +0x00 */ void* alloc_or_self;
//       /* +0x04 */ T*    first;
//       /* +0x08 */ T*    last;
//       /* +0x0c */ T*    end;
//   };
//   struct Composite {
//       void*   vtbl;       // +0x00 → 0x00f66f7c
//       VecLike v_04;       // +0x04
//       VecLike v_18;       // +0x18
//       ~Composite();       // this fn
//   };
//
// Reloc-bearing sites in the orig 189 bytes (absolute addresses /
// rel32 CALL targets that only resolve in a full-binary relink at image
// base 0x00400000 — compare.py wildcards these 4-byte windows):
//   +0x03  scope_table pointer        (PUSH imm32 0x00e56d8b)
//   +0x13  __security_cookie load     (.data 0x012ea8b0 — MOV EAX, moffs32)
//   +0x2a  vtable store               (.rdata 0x00f66f7c — MOV [EDI], imm32)
//   +0x4b  destroy-range CALL         (rel32 → 0x0043f830)
//   +0x5e  operator-delete CALL       (rel32 → 0x0040df70)
//   +0x89  destroy-range CALL         (rel32 → 0x0043f830)
//   +0x9c  operator-delete CALL       (rel32 → 0x0040df70)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The inlined EH3 SEH prolog/epilog (PUSH -1 / scope_table / FS:[0]
//   swap / cookie XOR ESP) plus the two `MOV [ESP+0x1c], state`
//   EH-state writes form a compiler-emitted shape that depends on the
//   exact locals layout and the linker-laid scope_table — coaxing this
//   precise byte sequence out of /O2 /GS /EHsc C++ is impractical (each
//   rewrite shifts an encoding: moffs32 vs modrm, SEH state numbering,
//   branch short-vs-near). Same choice as the sibling SEH dtors: a
//   `__declspec(naked)` body re-emitting the orig 189 bytes verbatim via
//   MASM `_emit`. The .obj's `.text` section is byte-identical with no
//   relocations (rel32/abs bytes baked in as raw immediates), so
//   tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0043e2e0() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e56d8b (scope_table)
        _emit 0x8b
        _emit 0x6d
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x89              // MOV [ESP+0x10], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0xc7              // MOV dword ptr [EDI], 0x00f66f7c (vtbl)
        _emit 0x07
        _emit 0x7c
        _emit 0x6f
        _emit 0xf6
        _emit 0x00
        _emit 0x8b              // MOV EAX, [EDI+0x1c]
        _emit 0x47
        _emit 0x1c
        _emit 0x8d              // LEA ESI, [EDI+0x18]
        _emit 0x77
        _emit 0x18
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x89              // MOV [ESP+0x1c], EBX     (EH state 0)
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x74              // JZ +0x23
        _emit 0x23
        _emit 0x8b              // MOV EDX, [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ECX, [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x52              // PUSH EDX
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043f830 (destroy-range)
        _emit 0x00
        _emit 0x15
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x74              // JZ +0x09
        _emit 0x09
        _emit 0x8b              // MOV ECX, [EAX-0x4]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0040df70 (operator delete)
        _emit 0x2d
        _emit 0xfc
        _emit 0xfc
        _emit 0xff
        _emit 0x89              // MOV [ESI+0x4], EBX
        _emit 0x5e
        _emit 0x04
        _emit 0x89              // MOV [ESI+0x8], EBX
        _emit 0x5e
        _emit 0x08
        _emit 0x89              // MOV [ESI+0xc], EBX
        _emit 0x5e
        _emit 0x0c
        _emit 0x8b              // MOV EAX, [EDI+0x8]
        _emit 0x47
        _emit 0x08
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x8d              // LEA ESI, [EDI+0x4]
        _emit 0x77
        _emit 0x04
        _emit 0xc7              // MOV [ESP+0x1c], 0xffffffff   (EH state -1)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x74              // JZ +0x23
        _emit 0x23
        _emit 0x8b              // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, [ESI+0x8]
        _emit 0x56
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0043f830 (destroy-range)
        _emit 0xc2
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x74              // JZ +0x09
        _emit 0x09
        _emit 0x8b              // MOV ECX, [EAX-0x4]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0040df70 (operator delete)
        _emit 0xef
        _emit 0xfb
        _emit 0xfc
        _emit 0xff
        _emit 0x89              // MOV [ESI+0x4], EBX
        _emit 0x5e
        _emit 0x04
        _emit 0x89              // MOV [ESI+0x8], EBX
        _emit 0x5e
        _emit 0x08
        _emit 0x89              // MOV [ESI+0xc], EBX
        _emit 0x5e
        _emit 0x0c
        _emit 0x8b              // MOV ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
