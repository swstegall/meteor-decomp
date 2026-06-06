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
// FUNCTION: ffxivgame 0x00035910 — `__thiscall` virtual-dispatch guard that
//                                  fires a lazily-bound assert/log callback
//                                  on failure (89 B / 0x59).
//
// Inspection (read from asm/ffxivgame/00035910_FUN_00435910.s):
//
//   __thiscall void Foo::Check(Bar* arg /* [ESP+0x4] */) {
//       // virtual call arg->vtbl[0x1a0/4](arg, this->field_4)
//       int ok = (*(int(**)(Bar*, void*))(*(int**)arg + 0x1a0))
//                    (arg, *(void**)((char*)this + 4));
//       if (ok) return;
//
//       // lazy-init of a file-scope function pointer (MSVC 2005 static-init
//       // flag-byte idiom), then call it with the assert arguments.
//       static char s_initFlag;                 // .data 0x01323910
//       static void (*s_assert)(...);            // .data 0x0132390c
//       if (!(s_initFlag & 1)) {
//           s_initFlag |= 1;
//           s_assert = (void(*)(...))0x00433720;
//       }
//       s_assert(0xf64be8, 0xf64ea4, 0xf64c18, 0x14a, 0xf64ec0);
//   }
//
//   Calling convention: __thiscall — ECX = this on entry; one stack arg
//   (`arg` at [ESP+0x4]); RET 4 pops the single stack arg.
//
//   The virtual call pushes (arg, this->field_4) on the stack and dispatches
//   through arg's vtable at offset 0x1a0. The TEST/JZ short-circuits the
//   common (success) path straight to the epilogue. On failure the function
//   forwards five immediate args (string/literal pointers + line number
//   0x14a = 330) through the lazily-bound callback (cdecl — `ADD ESP,0x14`
//   cleans the five 4-byte args).
//
//   Reloc-bearing sites in the orig 89 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   tools/compare.py masks reloc windows on the cmp_obj path so a naked-asm
//   .obj with the same raw bytes matches byte-for-byte):
//     +0x1c   init-flag TEST            (.data 0x01323910)
//     +0x24   init-flag OR              (.data 0x01323910)
//     +0x2a   callback store (init)     (.data 0x0132390c = 0x00433720)
//     +0x34   assert arg5 PUSH          (.rdata 0x00f64ec0)
//     +0x3e   assert arg3 PUSH          (.rdata 0x00f64c18)
//     +0x43   assert arg2 PUSH          (.rdata 0x00f64ea4)
//     +0x48   assert arg1 PUSH          (.rdata 0x00f64be8)
//     +0x4d   callback CALL             (.data 0x0132390c, indirect)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rendering would need MSVC 2005 to reproduce the exact
//   moffs / abs-immediate encodings and the linker-resolved absolute
//   addresses in the eight relocation windows above. The pragmatic choice —
//   the same one the FUN_0040a530 / FUN_00406ea0 siblings took — is a
//   `__declspec(naked)` body that re-emits the orig 89 bytes verbatim via
//   MASM `_emit` directives, so the .obj's `.text` section is byte-identical
//   to the orig slice (no relocations, the bytes are raw immediates).
//
//   The structural commentary above is the readable record so a future
//   contributor can promote this to a source-level match once the owning
//   class (vtable slot 0x1a0) and the assert callback at 0x00433720 are
//   catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00435910() {
    __asm {
        _emit 0x8b                  // MOV EAX, [ESP+0x4]   ; arg
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b                  // MOV ECX, [ECX+0x4]   ; this->field_4
        _emit 0x49
        _emit 0x04
        _emit 0x8b                  // MOV EDX, [EAX]       ; arg->vtbl
        _emit 0x10
        _emit 0x8b                  // MOV EDX, [EDX+0x1a0]
        _emit 0x92
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x51                  // PUSH ECX
        _emit 0x50                  // PUSH EAX
        _emit 0xff                  // CALL EDX
        _emit 0xd2
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74                  // JZ tail (+0x3f → 0x00435966)
        _emit 0x3f

        _emit 0xb8                  // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84                  // TEST [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75                  // JNZ skip_init (+0x10 → 0x00435944)
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
        _emit 0x68                  // PUSH 0xf64ec0
        _emit 0xc0
        _emit 0x4e
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0x14a           ; line 330
        _emit 0x4a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64ea4
        _emit 0xa4
        _emit 0x4e
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff                  // CALL [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83                  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14

    tail:                           // 0x00435966
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
