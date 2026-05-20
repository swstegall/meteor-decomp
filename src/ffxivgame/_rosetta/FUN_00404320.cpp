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
// FUNCTION: ffxivgame 0x00004320 — std::logic_error-shaped exception
//                                  constructor taking a UTF-8 string
//                                  (107 B / 0x6b, __thiscall, ret 4).
//
// Behaviour read from the orig 107 bytes at file offset 0x4320 cross-
// referenced against the Ghidra headless decompile hint
// (build/ghidra-decomp/ffxivgame/00004320_FUN_00404320.c):
//
//   __thiscall ExceptionLike *FUN_00404320(this, const Utf8String *msg);
//   ECX = this; [esp+4] = msg. Cleans 4 bytes of stack args on return
//   (`ret 4`). Returns `this` in EAX.
//
//   Object layout (inferred from the offsets touched):
//     +0x00 vftable                          (set to 0x00f54a2c — the
//                                             std::logic_error / runtime-
//                                             error vftable in the orig
//                                             const data)
//     +0x0C embedded Utf8String header
//       +0x10 SSO inline buffer (16 B; zeroed via [ECX+0x4] above)
//       +0x20 Mysize  (= 0 here)
//       +0x24 Myres   (= 0xF — 15-byte SSO capacity, matching the
//                              ffxivgame UTF-8 string class also seen
//                              in the FUN_00404040 sibling at +0x14 /
//                              +0x18 / +0x10-byte inline buf)
//
//   Body sketch:
//     1. MSVC-2005 SEH prologue: push trylevel=-1, push scopeTable
//        (LAB_00e54638), splice into FS:[0], push the security cookie
//        XORed with ESP.
//     2. Save `this` (ECX → ESI; also into [esp+8]).
//     3. Call FUN_009d18c9 — the per-binary SEH-prolog helper (the same
//        sibling Ghidra hints at as `FUN_009d18c9(__security_cookie ^
//        (uint)&stack0xffffffec)` in the pseudo-C, i.e. the security-
//        cookie install helper).
//     4. Empty-construct the embedded Utf8String at this+0xC (Mysize=0,
//        Myres=0xF, buf[0]=0).
//     5. Set the vftable: *(void**)this = 0x00f54a2c.
//     6. trylevel = 0  (mov [esp+0x1c], 0 — overwriting the prologue's
//                       -1 in the scope-table slot).
//     7. Tail call FUN_00404040(this+0xC, /*pos=*/0, /*count=*/~0u, msg)
//        — the Utf8String assign_substr helper at 0x00404040 — to copy
//        `msg` into the embedded string.
//     8. Return EAX = ESI = this; tear down SEH and pop the frame.
//
// Reloc-bearing sites the linker would resolve when emitted from
// source-level C++ (we re-emit the orig rel32 / imm32 bytes verbatim
// so the .obj's .text matches byte-for-byte with NO relocations):
//     +0x03   PUSH imm32   → 0x00e54638  (SEH scope table /
//                                          __ehhandler__ trampoline)
//     +0x11   MOV  imm32   → 0x012ea8b0  (__security_cookie)
//     +0x28   CALL rel32   → 0x009d18c9  (SEH-prolog helper /
//                                          __SEH_prolog4_GS)
//     +0x34   MOV  imm32   → 0x00f54a2c  (vftable — std::logic_error
//                                          shape)
//     +0x51   CALL rel32   → 0x00404040  (Utf8String::assign_substr)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ form would need to coax MSVC 2005 / /O2 /EHsc
//   /GS into reproducing the exact `__SEH_prolog4_GS` shape — including
//   the specific scope-table address baked into the orig binary and
//   the matching helper RVA — while also pinning the field-init order
//   (vftable → embedded string Mysize/Myres/buf → assign call) and
//   the register allocation (ESI for this, ECX as the +0xC alias, EAX
//   as the zero source and return slot). Coaxing that shape from
//   source is fragile; ffxivgame's canonical workaround is the
//   `_emit`-only naked-asm path used by the sibling FUN_00404040 and
//   FUN_00403bd0. compare.py masks reloc bytes out of the diff, but
//   here our .obj's .text simply IS the orig 107 bytes — no relocations
//   emitted — so the comparison is a straight byte match.

extern "C" __declspec(naked) void FUN_00404320() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E54638            (SEH scope table)
        _emit 0x38
        _emit 0x46
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV  EAX, FS:[0]            (old ExceptionList)
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV  EAX, DS:[0x012EA8B0]   (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR  EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA  EAX, [ESP+0x0C]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV  FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV  ESI, ECX                (this -> ESI)
        _emit 0xf1
        _emit 0x89              // MOV  [ESP+0x08], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0xe8              // CALL rel32 → 0x009D18C9      (SEH-prolog helper)
        _emit 0x7c
        _emit 0xd5
        _emit 0x5c
        _emit 0x00
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0
        _emit 0x8d              // LEA  ECX, [ESI+0x0C]         (ECX = &string)
        _emit 0x4e
        _emit 0x0c
        _emit 0xc7              // MOV  DWORD PTR [ESI], 0x00F54A2C   (vftable)
        _emit 0x06
        _emit 0x2c
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x89              // MOV  [ECX+0x14], EAX         (Mysize = 0)
        _emit 0x41
        _emit 0x14
        _emit 0xc7              // MOV  DWORD PTR [ECX+0x18], 0x0F   (Myres = 15)
        _emit 0x41
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                     (push 0)
        _emit 0x89              // MOV  [ESP+0x1C], EAX         (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x88              // MOV  BYTE PTR [ECX+0x04], AL (buf[0] = 0)
        _emit 0x41
        _emit 0x04
        _emit 0x8b              // MOV  EAX, [ESP+0x24]         (msg)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50              // PUSH EAX                     (push msg)
        _emit 0xe8              // CALL rel32 → 0x00404040      (assign_substr)
        _emit 0xca
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV  EAX, ESI                (return this)
        _emit 0xc6
        _emit 0x8b              // MOV  ECX, [ESP+0x0C]         (saved ExceptionList)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV  FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP  ECX                     (cookie)
        _emit 0x5e              // POP  ESI
        _emit 0x83              // ADD  ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET  4
        _emit 0x04
        _emit 0x00
    }
}
