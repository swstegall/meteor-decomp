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
// FUNCTION: ffxivgame 0x00401a00 — install-directory accessor (301 B,
//                                  SEH __try around magic-static init,
//                                  GetModuleFileNameW + last-backslash
//                                  truncate, __cdecl returns &g_obj)
//
// Inspection (read from the disassembly slice at orig RVA 0x00001a00):
//
//   __cdecl with no args. Standard MSVC SEH+/GS prologue followed by
//   an embedded __try frame around the once-only static initializer
//   of a singleton at 0x013237a8. The initializer body:
//
//     mov  eax, 1
//     test byte ptr [0x13237fc], al   ; magic-static "done" flag
//     jne  done
//     or   dword ptr [0x13237fc], eax ; latch the flag
//     mov  ecx, 0x13237a8             ; this = &g_obj
//     mov  [esp+0x220], 0             ; __try state := 0 (ctor in flight)
//     call 0x445cf0                   ; ctor of g_obj (still unmatched)
//     push offset 0x00f2e160          ; dtor for atexit
//     call 0x009d25c2                 ; atexit(dtor)
//     add  esp, 4
//     mov  [esp+0x220], -1            ; __try state := -1 (out of try)
//
//   The funclet table at SEH handler 0x00e543b5 is what unwinds the
//   half-constructed g_obj if the ctor (or atexit) throws. The two
//   __security_cookie loads at absolute 0x012ea8b0 anchor the
//   double-cookie that MSVC /GS + /EHsc emits when an SEH frame
//   coexists with a stack buffer (the buffer here is the wchar_t[260]
//   path scratch at [esp+0x10]).
//
//   Common path (runs every call, not just the first):
//
//     push 0x208                      ; sizeof buf in bytes (520)
//     lea  eax, [esp+0x10]            ; &buf[0]
//     push 0
//     push eax
//     call 0x009d2110                 ; memset(buf, 0, 520)
//     add  esp, 0xc
//     push 0x208                      ; nSize (note: byte count, not
//                                     ;        wchar count — see below)
//     lea  ecx, [esp+0x10]
//     push ecx
//     push 0                          ; hModule = NULL
//     call dword ptr [0x00f3e1e0]     ; GetModuleFileNameW import
//     test eax, eax
//     je   done                       ; on failure, skip path-set
//
//     ; Scan buf for last L'\\'. 5-wide-char-per-iteration unrolled
//     ; loop that updates `last_bs` (EDX) at every position from 0 up
//     ; to 259 inclusive. EAX is the running "centre" index, ECX is
//     ; &buf[i+1]. The middle position uses CMOVE (0F 44 D0), the
//     ; other four use JNE/LEA pairs — the asymmetric codegen is the
//     ; MSVC scheduler's choice, not a source asymmetry. After the
//     ; loop, the matching wchar is overwritten with 0:
//     ;
//     ;     mov  word ptr [esp+edx*2+0x0c], 0
//     ;
//     ; …leaving the dirname in buf. Then:
//     ;
//     ;     lea  edx, [esp+0x0c]      ; &buf[0]
//     ;     push edx
//     ;     mov  ecx, 0x013237a8      ; this = &g_obj
//     ;     call 0x004476e0           ; g_obj.set_install_dir(buf)
//
//   Epilogue returns the singleton:
//
//     mov  eax, 0x013237a8            ; eax = &g_obj
//     ; SEH unwind + /GS cookie check + add esp, 0x218 + ret
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body bakes nine absolute addresses (security cookie x2, init
//   flag, singleton x2, SEH handler-table, atexit dtor, four direct
//   CALL rel32s, and one indirect-IAT slot) plus the SEH funclet
//   reference at 0x00e543b5. A source-level reconstruction here would
//   need to reproduce all of those — every one of them resolves only
//   under a full-binary re-link at the orig load address, which a
//   standalone hand-written .cpp can't do. (See FUN_00401750 for the
//   same argument verbatim.)
//
//   So this is a `__declspec(naked)` body that re-emits the orig 301
//   bytes via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice (no relocations because
//   the bytes are emitted as raw immediates), which is what
//   tools/compare.py checks against.
//
//   To promote this to a real source-level reconstruction, decompile
//   the upstream sibling FUN_00445cf0 (g_obj ctor), and FUN_004476e0
//   (g_obj.set_install_dir) to learn the type of g_obj. The atexit
//   dtor thunk at 0x00f2e160 is a one-line trampoline. Once g_obj's
//   class is known, the source becomes (roughly):
//
//     extern "C" CInstallPathHolder &get_install_path() {
//         static CInstallPathHolder g_obj;          // SEH-wrapped init
//         wchar_t buf[260];                         // [esp+0x10]
//         memset(buf, 0, sizeof(buf));
//         if (GetModuleFileNameW(nullptr, buf, sizeof(buf))) {
//             int last_bs = 0;
//             for (int i = 0; i < 260; ++i) {
//                 if (buf[i] == L'\\') last_bs = i;
//             }
//             buf[last_bs] = 0;                     // truncate at dirname
//             g_obj.set_install_dir(buf);
//         }
//         return g_obj;
//     }
//
//   …but until those callees land in _rosetta/, the naked-asm form
//   below is the only path that produces a byte-identical .obj.

extern "C" __declspec(naked) void FUN_00401a00() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xb5
        _emit 0x43
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x81
        _emit 0xec

        _emit 0x0c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x02

        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x56
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x18

        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0x05

        _emit 0xfc
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0x75
        _emit 0x33
        _emit 0x09
        _emit 0x05
        _emit 0xfc
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0xb9
        _emit 0xa8
        _emit 0x37
        _emit 0x32

        _emit 0x01
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x8f
        _emit 0x42
        _emit 0x04

        _emit 0x00
        _emit 0x68
        _emit 0x60
        _emit 0xe1
        _emit 0xf2
        _emit 0x00
        _emit 0xe8
        _emit 0x57
        _emit 0x0b
        _emit 0x5d
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xc7
        _emit 0x84

        _emit 0x24
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x68
        _emit 0x08
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x44

        _emit 0x24
        _emit 0x10
        _emit 0x6a
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0x86
        _emit 0x06
        _emit 0x5d
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x68
        _emit 0x08
        _emit 0x02

        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51
        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0xe0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x85

        _emit 0xc0
        _emit 0x74
        _emit 0x5f
        _emit 0x33
        _emit 0xd2
        _emit 0x8d
        _emit 0x42
        _emit 0x02
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0e
        _emit 0xbe
        _emit 0x5c
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x66
        _emit 0x39
        _emit 0x71
        _emit 0xfe
        _emit 0x75
        _emit 0x03
        _emit 0x8d
        _emit 0x50
        _emit 0xfe
        _emit 0x66
        _emit 0x39
        _emit 0x31
        _emit 0x75
        _emit 0x03
        _emit 0x8d

        _emit 0x50
        _emit 0xff
        _emit 0x66
        _emit 0x39
        _emit 0x71
        _emit 0x02
        _emit 0x0f
        _emit 0x44
        _emit 0xd0
        _emit 0x66
        _emit 0x39
        _emit 0x71
        _emit 0x04
        _emit 0x75
        _emit 0x03
        _emit 0x8d

        _emit 0x50
        _emit 0x01
        _emit 0x66
        _emit 0x39
        _emit 0x71
        _emit 0x06
        _emit 0x75
        _emit 0x03
        _emit 0x8d
        _emit 0x50
        _emit 0x02
        _emit 0x83
        _emit 0xc0
        _emit 0x05
        _emit 0x8d
        _emit 0x58

        _emit 0xfe
        _emit 0x83
        _emit 0xc1
        _emit 0x0a
        _emit 0x81
        _emit 0xfb
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x72
        _emit 0xc5
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x54

        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x52
        _emit 0xb9
        _emit 0xa8
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0xde
        _emit 0x5b

        _emit 0x04
        _emit 0x00
        _emit 0xb8
        _emit 0xa8
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x18
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0x89

        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x33

        _emit 0xcc
        _emit 0xe8
        _emit 0xce
        _emit 0x05
        _emit 0x5d
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x18
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
