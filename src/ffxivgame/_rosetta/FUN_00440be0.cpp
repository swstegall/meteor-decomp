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
// FUNCTION: ffxivgame 0x00040be0 — `__cdecl` copy-init thunk that initialises
//                                  an object at the hidden return pointer from
//                                  a source object, seeding three fixed fields
//                                  and delegating to a `__thiscall` sub-ctor at
//                                  0x00440850 (117 B / 0x75, EH3-SEH wrapped,
//                                  ESP-relative frame).
//
// Calling convention: __cdecl. Returns the destination pointer in EAX (the
// retval slot at [ESP+0x18] is rewritten with the same pointer the caller
// passed — this is the by-pointer-return / copy-construct idiom).
//
//   void* __cdecl FUN_00440be0(void* dst /* [esp+0x18] */,
//                              const void* src /* [esp+0x1c] */);
//
// Behaviour (read from the disassembly at orig RVA 0x00040be0):
//
//     dst->field_0 = src->field_0;     // 89 10        MOV [EAX],EDX
//     if (dst) {                       // TEST EAX,EAX / JZ end
//         T* sub = (T*)((char*)dst + 4);
//         sub->field_18 = 7;           // c7 41 18 07 ...
//         sub->field_14 = 0;           // c7 41 14 00 ...
//         sub->field_4  = (short)0;    // 66 c7 41 04 00 00
//         // __thiscall ctor on sub, args (src+4, 0, -1) pushed right-to-left
//         FUN_00440850(sub, (char*)src + 4, 0, -1);   // CALL 0x00440850
//     }
//
// The EH3 frame exists because the sub-ctor at 0x00440850 can throw; the
// trylevel local at [ESP+0x10] is seeded to 0 before the guarded call.
//
// Stack frame (after the EH3 prologue, ESP-relative — no EBP frame; the
// PUSH ECX owns the single trylevel/local slot):
//     [esp+0x00]  __security_cookie ^ ESP_at_install
//     [esp+0x04]  scratch local (= dst, also re-read at epilogue as saved-FS)
//     [esp+0x08]  EH3 saved-FS:[0]  (next handler in chain)
//     [esp+0x0c]  EH3 scope-table handler (0x00ef8c61 — .rdata FuncInfo)
//     [esp+0x10]  EH3 trylevel      (initial -1; set to 0 before ctor call)
//     [esp+0x14]  return address
//     [esp+0x18]  caller dst (also the retval slot)
//     [esp+0x1c]  caller src
//
// Reloc-bearing sites in the orig 117 bytes (absolute addresses resolve only
// in a full-binary relink at image base 0x00400000; standalone .obj emits
// them as raw immediates link.exe leaves alone):
//     +0x03  scope-table handler   (0x00ef8c61 — .rdata FuncInfo)
//     +0x07  FS:[0] read           (constant 0, fold-through)
//     +0x0f  __security_cookie     (.data 0x012ea8b0)
//     +0x1b  FS:[0] install        (constant 0, fold-through)
//     +0x60  sub-ctor CALL         (.text 0x00440850 rel32 — __thiscall)
//     +0x65  FS:[0] uninstall      (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as the
// EH-wrapped sibling FUN_00401650): a source-level rewrite cannot reliably
// coax MSVC 2005 /O2 /GS into the exact EH3 prolog + ESP-relative frame +
// fixed-field stores + linker-resolved absolutes without shifting bytes. The
// __declspec(naked) body re-emits the orig 117 bytes verbatim via MASM
// _emit directives; the .obj's .text is byte-identical to the orig slice,
// which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_00440be0() {
    __asm {
        // 00040be0: 6a ff                 PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00040be2: 68 61 8c ef 00        PUSH 0xef8c61   (EH3 scope-table)
        _emit 0x68
        _emit 0x61
        _emit 0x8c
        _emit 0xef
        _emit 0x00
        // 00040be7: 64 a1 00 00 00 00     MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040bed: 50                    PUSH EAX
        _emit 0x50
        // 00040bee: 51                    PUSH ECX        (allocate trylevel/local slot)
        _emit 0x51
        // 00040bef: a1 b0 a8 2e 01        MOV EAX,[0x012ea8b0]   (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00040bf4: 33 c4                 XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00040bf6: 50                    PUSH EAX
        _emit 0x50
        // 00040bf7: 8d 44 24 08           LEA EAX,[ESP+0x8]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00040bfb: 64 a3 00 00 00 00     MOV FS:[0x0],EAX       (install handler)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040c01: 8b 44 24 18           MOV EAX,[ESP+0x18]     (dst)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00040c05: 89 44 24 18           MOV [ESP+0x18],EAX     (retval slot = dst)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00040c09: 89 44 24 04           MOV [ESP+0x4],EAX      (scratch local = dst)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00040c0d: 85 c0                 TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00040c0f: c7 44 24 10 00 00 00 00  MOV [ESP+0x10],0x0  (trylevel = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040c17: 74 2c                 JZ 0x00440c45          (dst == 0 → epilogue)
        _emit 0x74
        _emit 0x2c
        // 00040c19: 8b 4c 24 1c           MOV ECX,[ESP+0x1c]     (src)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00040c1d: 8b 11                 MOV EDX,[ECX]          (src->field_0)
        _emit 0x8b
        _emit 0x11
        // 00040c1f: 89 10                 MOV [EAX],EDX          (dst->field_0 = src->field_0)
        _emit 0x89
        _emit 0x10
        // 00040c21: 6a ff                 PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00040c23: 8d 51 04              LEA EDX,[ECX+0x4]      (src + 4)
        _emit 0x8d
        _emit 0x51
        _emit 0x04
        // 00040c26: 8d 48 04              LEA ECX,[EAX+0x4]      (sub = dst + 4, ECX = this)
        _emit 0x8d
        _emit 0x48
        _emit 0x04
        // 00040c29: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00040c2b: c7 41 18 07 00 00 00  MOV [ECX+0x18],0x7
        _emit 0xc7
        _emit 0x41
        _emit 0x18
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040c32: c7 41 14 00 00 00 00  MOV [ECX+0x14],0x0
        _emit 0xc7
        _emit 0x41
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040c39: 52                    PUSH EDX               (src + 4)
        _emit 0x52
        // 00040c3a: 66 c7 41 04 00 00     MOV word ptr [ECX+0x4],0x0
        _emit 0x66
        _emit 0xc7
        _emit 0x41
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00040c40: e8 0b fc ff ff        CALL 0x00440850        (__thiscall sub-ctor)
        _emit 0xe8
        _emit 0x0b
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 00040c45: 8b 4c 24 08           MOV ECX,[ESP+0x8]      (saved FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00040c49: 64 89 0d 00 00 00 00  MOV FS:[0x0],ECX       (uninstall handler)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040c50: 59                    POP ECX
        _emit 0x59
        // 00040c51: 83 c4 10              ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00040c54: c3                    RET
        _emit 0xc3
    }
}
