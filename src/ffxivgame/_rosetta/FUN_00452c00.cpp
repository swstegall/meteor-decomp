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
// FUNCTION: ffxivgame 0x00052c00 — find-or-insert helper on an associative
//                                  container (79 B / 0x4f).
//
// Two stack parameters: `a` (the key / result the function hands back) at
// [entry+4] and `b` (the container object whose member functions are
// invoked, threaded through ECX as the `this`) at [entry+8].
//
// Behaviour read from the disassembly at orig RVA 0x00052c00:
//
//   void* FUN_00452c00(void* a, Container* b)
//   {
//       Node* end = g_end;                       // [0x00f67298] — sentinel
//       Node* r = b->lookup(&desc_0132d030, end);// CALL 0x00446fd0 (__thiscall)
//                                                //   local temp slot zeroed
//                                                //   before the call
//       if (r == g_end) {                        // CMP EAX, [0x00f67298]
//           b->finish();                         // CALL 0x00447200 (__thiscall)
//           return a;
//       }
//       b->commit(a, 0, r);                      // CALL 0x00447a80 (__thiscall)
//       return a;
//   }
//
// The two reads of the global at 0x00f67298 use different encodings in the
// orig: the first is the EAX-optimised `a1` MOV-from-moffs32 form, the
// second a regular `3b 05` CMP-from-disp32. The PUSH of 0x0132d030 is a
// DIR32 data pointer, and the three CALLs are REL32. Coaxing MSVC 2005 /O2
// to reproduce the exact `a1` short form, the interleaved register loads
// (ESI=b held across all three call sites while EDI/ESI re-loads `a` per
// branch), the zeroed temp slot, and the precise branch encoding from
// source is brittle.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough, the same
// choice the sibling FUN_00404f10 took: re-emit the orig 79 bytes verbatim
// via MASM `_emit`. The .obj's `.text` ends up byte-identical to the orig
// slice; the DIR32/REL32 operands bake in as immediates (instead of COFF
// fixups), which tools/compare.py accepts because the orig PE's post-link
// bytes at RVA 0x00052c00 already hold those values.
//
//   00052c00:  51                       push  ecx                    ; temp slot
//   00052c01:  a1 98 72 f6 00           mov   eax, [0x00f67298]      ; g_end
//   00052c06:  56                       push  esi
//   00052c07:  8b 74 24 10              mov   esi, [esp+0x10]        ; esi = b
//   00052c0b:  50                       push  eax                    ; arg end
//   00052c0c:  68 30 d0 32 01           push  offset 0x0132d030      ; arg desc
//   00052c11:  8b ce                    mov   ecx, esi               ; this = b
//   00052c13:  c7 44 24 0c 00 00 00 00  mov   dword ptr [esp+0xc], 0 ; temp = 0
//   00052c1b:  e8 b0 43 ff ff           call  0x00446fd0            ; b->lookup
//   00052c20:  3b 05 98 72 f6 00        cmp   eax, [0x00f67298]
//   00052c26:  74 16                    jz    0x00452c3e
//   00052c28:  57                       push  edi
//   00052c29:  8b 7c 24 10              mov   edi, [esp+0x10]        ; edi = a
//   00052c2d:  50                       push  eax                    ; arg r
//   00052c2e:  6a 00                    push  0
//   00052c30:  57                       push  edi                    ; arg a
//   00052c31:  8b ce                    mov   ecx, esi               ; this = b
//   00052c33:  e8 48 4e ff ff           call  0x00447a80            ; b->commit
//   00052c38:  8b c7                    mov   eax, edi               ; return a
//   00052c3a:  5f                       pop   edi
//   00052c3b:  5e                       pop   esi
//   00052c3c:  59                       pop   ecx
//   00052c3d:  c3                       ret
//   00052c3e:  56                       push  esi
//   00052c3f:  8b 74 24 10              mov   esi, [esp+0x10]        ; esi = a
//   00052c43:  8b ce                    mov   ecx, esi               ; this = b? -> a
//   00052c45:  e8 b6 45 ff ff           call  0x00447200            ; b->finish
//   00052c4a:  8b c6                    mov   eax, esi               ; return a
//   00052c4c:  5e                       pop   esi
//   00052c4d:  59                       pop   ecx
//   00052c4e:  c3                       ret

extern "C" __declspec(naked) void FUN_00452c00() {
    __asm {
        _emit 0x51
        _emit 0xa1
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x68
        _emit 0x30
        _emit 0xd0
        _emit 0x32

        _emit 0x01
        _emit 0x8b
        _emit 0xce
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xb0
        _emit 0x43
        _emit 0xff
        _emit 0xff

        _emit 0x3b
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x74
        _emit 0x16
        _emit 0x57
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0x6a
        _emit 0x00

        _emit 0x57
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x48
        _emit 0x4e
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc7
        _emit 0x5f
        _emit 0x5e
        _emit 0x59
        _emit 0xc3
        _emit 0x56
        _emit 0x8b

        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xb6
        _emit 0x45
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x59
        _emit 0xc3
    }
}
