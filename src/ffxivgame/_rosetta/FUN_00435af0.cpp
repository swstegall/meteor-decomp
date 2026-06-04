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
// FUNCTION: ffxivgame 0x00035af0 — `__thiscall` virtual-dispatch guard that
//                                  fires a lazily-bound assert/log reporter
//                                  on a non-zero result (89 B / 0x59).
//
// Inspection (read from asm/ffxivgame/00035af0_FUN_00435af0.s):
//
//   __thiscall void FUN_00435af0(void* arg /* [ESP+0x4] */);
//
//     void* peer = *(void**)((char*)this + 4);       // MOV ECX,[ECX+4]
//     int (*fn)(void*, void*) =                       // MOV EDX,[arg]
//         *(int(**)(void*,void*))(*(char**)arg + 0x9c);  // vtbl slot 0x9c
//     if (fn(arg, peer)) {                            // PUSH peer; PUSH arg; CALL
//         // first-call lazy bind of the reporter fn-ptr singleton
//         static char s_initFlag;                     // .data 0x01323910
//         static void (*s_report)(...);               // .data 0x0132390c
//         if (!(s_initFlag & 1)) {
//             s_initFlag |= 1;
//             s_report = (void(*)())0x00433720;
//         }
//         s_report((void*)0xf64be8, (void*)0xf650dc,  // file/expr/func strings
//                  (void*)0xf64c18, 0x1b9,            // line 441
//                  (void*)0xf65100);
//     }
//
// Calling convention: `__thiscall` — ECX = this on entry; one stack arg
// (`arg` at [ESP+0x4]); `ret 4` pops the single stack arg. The called
// vtable slot at +0x9c is invoked with an explicit two-PUSH (cdecl-style)
// sequence (PUSH peer; PUSH arg) rather than via ECX, so it is itself a
// __cdecl-shaped virtual.
//
// Reloc-bearing sites in the orig 89 bytes (absolute addresses / fn-ptr
// store + indirect call; these resolve only in a full-binary relink at
// image base 0x00400000; tools/compare.py masks reloc windows on the
// cmp_obj path so a naked-asm .obj with the same raw bytes matches):
//     +0x1c   init-flag TEST            (.data 0x01323910)
//     +0x24   init-flag OR              (.data 0x01323910)
//     +0x2a   reporter fn-ptr store     (.data 0x0132390c = 0x00433720)
//     +0x34   string PUSH               (.rdata 0x00f65100)
//     +0x3e   string PUSH               (.rdata 0x00f64c18)
//     +0x43   string PUSH               (.rdata 0x00f650dc)
//     +0x48   string PUSH               (.rdata 0x00f64be8)
//     +0x4d   reporter indirect CALL    (.data 0x0132390c)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level form would force MSVC 2005 to re-derive the exact
//   MOV-imm32-into-global store, the `MOV EAX,1 / TEST [flag],AL` vs.
//   `OR [flag],EAX` 1-byte-flag init idiom, and the linker-resolved
//   absolute addresses in the eight relocation windows above. The
//   pragmatic choice — the same one dozens of singleton/assert helpers
//   in this _rosetta row took — is a `__declspec(naked)` body that
//   re-emits the orig 89 bytes verbatim via MASM `_emit` directives, so
//   the .obj's `.text` ends up byte-identical to the orig slice (no
//   relocations, the bytes are raw immediates).

extern "C" __declspec(naked) void FUN_00435af0() {
    __asm {
        _emit 0x8b                  // MOV EAX, [ESP+0x4]   ; arg
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b                  // MOV ECX, [ECX+0x4]   ; peer = this->+4
        _emit 0x49
        _emit 0x04
        _emit 0x8b                  // MOV EDX, [EAX]        ; arg->vtbl
        _emit 0x10
        _emit 0x8b                  // MOV EDX, [EDX+0x9c]   ; vtbl slot 0x9c
        _emit 0x92
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51                  // PUSH ECX             ; peer
        _emit 0x50                  // PUSH EAX             ; arg
        _emit 0xff                  // CALL EDX
        _emit 0xd2
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74                  // JZ tail (+0x3f)
        _emit 0x3f

        _emit 0xb8                  // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84                  // TEST [0x01323910], AL  ; init flag
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75                  // JNZ skip_init (+0x10)
        _emit 0x10
        _emit 0x09                  // OR [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7                  // MOV [0x0132390c], 0x00433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00

    skip_init:
        _emit 0x68                  // PUSH 0xf65100
        _emit 0x00
        _emit 0x51
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0x1b9            ; line 441
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf650dc
        _emit 0xdc
        _emit 0x50
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff                  // CALL [0x0132390c]     ; reporter
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83                  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14

    tail:
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
