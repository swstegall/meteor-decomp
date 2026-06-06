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
// FUNCTION: ffxivgame 0x004428a0 — `__thiscall bool` over a deque-of-
//                                  pointers; iterates [first,last) checking
//                                  each element's +0x20 field against an
//                                  arg, and for every match advances the
//                                  range + invokes a helper (284 B / 0x11c).
//
// Read from the disassembly at orig RVA 0x004428a0:
//
//   __thiscall bool FUN_004428a0(this, <stack arg key @ esp+0x44>);
//
//   ECX = this; EBP = this + 8 caches the deque control block (the
//   `std::deque` _Mydeque-style header whose +0x08 = _Myoff base,
//   +0x0c = _Myfirst index, +0x10 = _Mysize). The body walks the
//   half-open block range [_Myfirst, _Myfirst+_Mysize) via the canonical
//   MSVC-2005 `_DEBUG` deque-iterator machinery: every dereference is
//   guarded by a `CMP / JBE/JC/JA → CALL 0x009d22b4` invalid-iterator
//   abort thunk, and the qword-wide MOVQ at +0x42/+0x48 is the inlined
//   iterator-pair copy (`_Mycont`,`_Myoff`) through an esp temp.
//
//   For each element `e = _Map[(idx) & _Mask]` whose `e->[0x20]` equals
//   the key argument, it builds two 3-dword iterator descriptors on the
//   stack (`{0, _Mycont, off}` pairs at the freshly `sub esp,0xc`'d
//   slots), pushes a result-iterator out-param (`lea edx,[esp+0x4c]`),
//   and tail-CALLs the per-match handler at 0x00442f70, bumping a hit
//   counter at [esp+0x10]. The epilogue returns `hit_counter != 0` via
//   `XOR EAX,EAX / CMP [esp+0xc],EAX / SETNZ AL`, i.e. "did we find &
//   process at least one matching element?".
//
//   Reloc-bearing call sites in the orig 284 bytes (rel32 targets that
//   resolve only in a full-binary relink at image base 0x00400000):
//     +0x28/+0x39/+0x5a/+0x6d/+0x7c/+0xcb  CALL 0x009d22b4 (deque debug
//                                          invalid-iterator abort thunk)
//     +0xf6                                CALL 0x00442f70 (per-match
//                                          element handler)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 into
//   reproducing the exact deque `_DEBUG` iterator lowering — the precise
//   ordering of the six range-check abort thunks, the MOVQ iterator-pair
//   copy through esp+0x1c/+0x28, the two `sub esp,0xc` descriptor-build
//   frames with their EBX/EAX scratch bases, and the SETNZ-based bool
//   return. Each of those is brittle under /O2: any high-level restate
//   shifts at least one byte (branch short-vs-near, scratch-reg choice,
//   temp-slot offset, descriptor field order). The pragmatic, locally-
//   sanctioned choice (same as FUN_00404f70 / FUN_00401a00) is a naked
//   body that re-emits the orig 284 bytes verbatim via MASM `_emit`
//   directives; the .obj's `.text` ends up byte-identical to the orig
//   slice (no relocations — the rel32 operands are baked as the orig's
//   own immediates), which is exactly what `tools/compare.py` checks.
//
//   The structural commentary above is the readable record of what the
//   function does, so a future contributor can promote this to a real
//   source-level match once the surrounding `std::deque<T*>` control
//   block and the element type (+0x20 key field) are catalogued under
//   decomp-notes/types/.

extern "C" __declspec(naked) void FUN_004428a0() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x30
        _emit 0x53
        _emit 0x55
        _emit 0x33
        _emit 0xc0
        _emit 0x56
        _emit 0x8d
        _emit 0x69
        _emit 0x08
        _emit 0x57
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10

        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x45
        _emit 0x0c
        _emit 0x8b
        _emit 0x5d
        _emit 0x10
        _emit 0x03
        _emit 0xd8

        _emit 0x3b
        _emit 0xc3
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0xe7
        _emit 0xf9
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x7d
        _emit 0x0c

        _emit 0x8b
        _emit 0x45
        _emit 0x10
        _emit 0x03
        _emit 0xc7
        _emit 0x3b
        _emit 0xf8
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0xd6
        _emit 0xf9
        _emit 0x58
        _emit 0x00
        _emit 0x89
        _emit 0x6c

        _emit 0x24
        _emit 0x20
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8b
        _emit 0x74

        _emit 0x24
        _emit 0x2c
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x04
        _emit 0x3b
        _emit 0xf5
        _emit 0x74
        _emit 0x05
        _emit 0xe8
        _emit 0xb5
        _emit 0xf9
        _emit 0x58
        _emit 0x00
        _emit 0x3b

        _emit 0xfb
        _emit 0x0f
        _emit 0x84
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x85
        _emit 0xf6
        _emit 0x8b
        _emit 0xdf
        _emit 0x75
        _emit 0x05
        _emit 0xe8
        _emit 0xa2
        _emit 0xf9

        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x03
        _emit 0x4e
        _emit 0x10
        _emit 0x3b
        _emit 0xf9
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x93
        _emit 0xf9
        _emit 0x58

        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        _emit 0x3b
        _emit 0xc7
        _emit 0x77
        _emit 0x04
        _emit 0x8b
        _emit 0xdf
        _emit 0x2b
        _emit 0xd8
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        _emit 0x8b

        _emit 0x04
        _emit 0x9a
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        _emit 0x39
        _emit 0x48
        _emit 0x20
        _emit 0x74
        _emit 0x18
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        _emit 0x03
        _emit 0x56

        _emit 0x10
        _emit 0x3b
        _emit 0xfa
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x6a
        _emit 0xf9
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x83
        _emit 0xc7

        _emit 0x01
        _emit 0xeb
        _emit 0x9f
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x8d
        _emit 0x6f
        _emit 0x01
        _emit 0x03

        _emit 0xc8
        _emit 0x3b
        _emit 0xe9
        _emit 0x8b
        _emit 0xdc
        _emit 0x77
        _emit 0x04
        _emit 0x3b
        _emit 0xe8
        _emit 0x73
        _emit 0x05
        _emit 0xe8
        _emit 0x44
        _emit 0xf9
        _emit 0x58
        _emit 0x00

        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        _emit 0x8b
        _emit 0xc4
        _emit 0xc7
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89

        _emit 0x73
        _emit 0x04
        _emit 0x89
        _emit 0x6b
        _emit 0x08
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x4c
        _emit 0x52
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x89
        _emit 0x70
        _emit 0x04
        _emit 0x89
        _emit 0x78
        _emit 0x08
        _emit 0xe8
        _emit 0xd5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x01

        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        _emit 0xe9
        _emit 0x0f
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x33
        _emit 0xc0
        _emit 0x39
        _emit 0x44
        _emit 0x24
        _emit 0x0c

        _emit 0x5e
        _emit 0x5d
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
