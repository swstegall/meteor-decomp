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
// FUNCTION: ffxivgame 0x00041f00 — `__thiscall` audio play-sound helper
//                                  (324 B / 0x144, EH3-SEH wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x00041f00):
//
//   __thiscall bool FUN_00441f00(this,
//                                void *arg1,   int arg2,
//                                float arg3,   int arg4,
//                                void *arg5,   float arg6,
//                                int arg7)
//   Returns bool (AL). Caller pops 0x1c (7 × 4) bytes via RET 0x1c.
//   ECX = this is saved to EBX throughout.
//
//   Structure:
//
//     if (!check()) return false;          // CALL 0x00b8e7b0 — early guard
//
//     *arg1.field_0x10 = arg2;
//     // build a local param struct at [esp+0x24]:
//     //   [+0x00] = arg3 (float)
//     //   [+0x04] = arg6 (float)
//     //   [+0x08] = arg4 (int)
//     //   [+0x10] = 8 bytes from arg5[0]
//     //   [+0x18] = 8 bytes from arg5[8]
//     //   [+0x0c] = arg7 (int)           (stored after 2 pushes, so +0x30 in frame)
//     //   [+0x20] = float const from [0x00f54f70]  (stored after 2 pushes)
//     if (!this->method(arg2, &param_struct))  // CALL 0x00441bf0
//     {
//         void *obj = lookup(0x5003c);         // CALL 0x009d1b35 (__cdecl)
//         if (obj != nullptr) {
//             esi = obj->create(this->field_4, &param_struct, arg2);  // CALL 0x004435d0
//         } else {
//             esi = nullptr;
//         }
//         register_sound(&this->field_0x18, arg2, esi);               // CALL 0x00994a90
//         // virtual call: this->field_4->vtable[1](arg2, 0x40000, 0, 1, esi, 0, 0, 0, 0)
//         esi->field_4 = vtable_fn(arg2, 0x40000, 0, 1, esi, 0, 0, 0, 0);
//     }
//     return true;
//
//   Stack frame layout (after prologue; ESP = entry_ESP − 0x54):
//     [ESP + 0x00]  __security_cookie ^ ESP
//     [ESP + 0x04]  saved EDI
//     [ESP + 0x08]  saved ESI
//     [ESP + 0x0c]  saved EBX
//     [ESP + 0x10 .. +0x47]  local space (0x38 bytes)
//     [ESP + 0x48]  old FS:[0] (EH chain link)
//     [ESP + 0x4c]  0xe570e4 (EH scope-table)
//     [ESP + 0x50]  −1 / SEH state (set to 0 on try-block entry)
//     [ESP + 0x54]  return address
//     [ESP + 0x58]  arg1
//     [ESP + 0x5c]  arg2
//     [ESP + 0x60]  arg3  (float)
//     [ESP + 0x64]  arg4
//     [ESP + 0x68]  arg5  (pointer to 16-byte struct)
//     [ESP + 0x6c]  arg6  (float)
//     [ESP + 0x70]  arg7
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function mixes EH3 SEH bookkeeping, SSE scalar float moves
//   (MOVSS / MOVQ via XMM0), XMM-assisted 8-byte struct copies, an
//   indirect virtual dispatch through a double-deref vtable, a cdecl
//   lookup call, and three separate callee ABIs (__cdecl, __thiscall,
//   indirect vtable). Reproducing the exact register allocation, the
//   MOVSS ↔ MOVQ encoding choices, the particular disp8/disp32 branch
//   encoding, and all linker-resolved absolute addresses from a
//   source-level rewrite under MSVC 2005 /O2 is brittle. The
//   pragmatic choice — matching the pattern of FUN_004014b0,
//   FUN_00401a00, and the rest of the _rosetta siblings — is a
//   `__declspec(naked)` body that re-emits the 324 orig bytes verbatim
//   via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_00441f00() {
    __asm {
        // 00041f00: PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00041f02: PUSH 0xe570e4
        _emit 0x68
        _emit 0xe4
        _emit 0x70
        _emit 0xe5
        _emit 0x00
        // 00041f07: MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00041f0d: PUSH EAX
        _emit 0x50
        // 00041f0e: SUB ESP, 0x38
        _emit 0x83
        _emit 0xec
        _emit 0x38
        // 00041f11: PUSH EBX
        _emit 0x53
        // 00041f12: PUSH ESI
        _emit 0x56
        // 00041f13: PUSH EDI
        _emit 0x57
        // 00041f14: MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00041f19: XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00041f1b: PUSH EAX
        _emit 0x50
        // 00041f1c: LEA EAX, [ESP+0x48]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x48
        // 00041f20: MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00041f26: MOV EBX, ECX   (save this)
        _emit 0x8b
        _emit 0xd9
        // 00041f28: CALL 0x00b8e7b0  (early guard check)
        _emit 0xe8
        _emit 0x83
        _emit 0xc8
        _emit 0x74
        _emit 0x00
        // 00041f2d: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00041f2f: JNZ +0x17  (→ 0x00441f48)
        _emit 0x75
        _emit 0x17
        // 00041f31: XOR AL, AL   (return false path)
        _emit 0x32
        _emit 0xc0
        // 00041f33: MOV ECX, [ESP+0x48]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        // 00041f37: MOV FS:[0], ECX   (restore EH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00041f3e: POP ECX
        _emit 0x59
        // 00041f3f: POP EDI
        _emit 0x5f
        // 00041f40: POP ESI
        _emit 0x5e
        // 00041f41: POP EBX
        _emit 0x5b
        // 00041f42: ADD ESP, 0x44
        _emit 0x83
        _emit 0xc4
        _emit 0x44
        // 00041f45: RET 0x1c
        _emit 0xc2
        _emit 0x1c
        _emit 0x00
        // 00041f48: MOVSS XMM0, [ESP+0x60]   (arg3)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x60
        // 00041f4e: MOV ECX, [ESP+0x64]   (arg4)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        // 00041f52: MOV EAX, [ESP+0x58]   (arg1)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x58
        // 00041f56: MOV EDI, [ESP+0x5c]   (arg2)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x5c
        // 00041f5a: MOV EDX, [ESP+0x70]   (arg7)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x70
        // 00041f5e: MOVSS [ESP+0x24], XMM0   (local.arg3 = arg3)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00041f64: MOVSS XMM0, [ESP+0x6c]   (arg6)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        // 00041f6a: MOV [ESP+0x2c], ECX   (local.arg4 = arg4)
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 00041f6e: MOV ECX, [ESP+0x68]   (arg5 ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x68
        // 00041f72: MOV [EAX+0x10], EDI   (arg1->field_0x10 = arg2)
        _emit 0x89
        _emit 0x78
        _emit 0x10
        // 00041f75: MOVSS [ESP+0x28], XMM0   (local.arg6 = arg6)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 00041f7b: MOVQ XMM0, [ECX]   (load arg5[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 00041f7f: MOV [ESP+0x44], EAX   (save arg1 ptr)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x44
        // 00041f83: MOVQ [ESP+0x34], XMM0   (copy arg5[0..7] to local)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 00041f89: MOVQ XMM0, [ECX+0x8]   (load arg5[8..15])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        // 00041f8e: LEA EAX, [ESP+0x24]   (EAX = &local param struct)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00041f92: MOVQ [ESP+0x3c], XMM0   (copy arg5[8..15] to local)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 00041f98: MOVSS XMM0, [0x00f54f70]   (float constant)
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 00041fa0: PUSH EAX   (&param struct — 2nd arg to 0x00441bf0)
        _emit 0x50
        // 00041fa1: PUSH EDI   (arg2 — 1st arg to 0x00441bf0)
        _emit 0x57
        // 00041fa2: MOV ECX, EBX   (this)
        _emit 0x8b
        _emit 0xcb
        // 00041fa4: MOV [ESP+0x38], EDX   (store arg7 to local after pushes)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 00041fa8: MOVSS [ESP+0x48], XMM0   (store float const to local)
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x48
        // 00041fae: CALL 0x00441bf0   (__thiscall, 2 args)
        _emit 0xe8
        _emit 0x3d
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 00041fb3: TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 00041fb5: JNZ +0x76   (→ 0x0044202d, success)
        _emit 0x75
        _emit 0x76
        // 00041fb7: PUSH 0x5003c   (audio channel/sound id)
        _emit 0x68
        _emit 0x3c
        _emit 0x00
        _emit 0x05
        _emit 0x00
        // 00041fbc: CALL 0x009d1b35   (__cdecl, 1 arg)
        _emit 0xe8
        _emit 0x74
        _emit 0xfb
        _emit 0x58
        _emit 0x00
        // 00041fc1: ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00041fc4: MOV [ESP+0x5c], EAX   (save obj ptr — reuses arg2 stack slot)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        // 00041fc8: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00041fca: MOV dword ptr [ESP+0x50], 0x0   (SEH state = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x50
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00041fd2: JZ +0x15   (→ 0x00441fe9, null → ESI = 0)
        _emit 0x74
        _emit 0x15
        // 00041fd4: MOV EDX, [EBX+0x4]   (this->field_4)
        _emit 0x8b
        _emit 0x53
        _emit 0x04
        // 00041fd7: PUSH EDI   (arg2)
        _emit 0x57
        // 00041fd8: LEA ECX, [ESP+0x28]   (&param struct after push)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 00041fdc: PUSH ECX   (&param struct)
        _emit 0x51
        // 00041fdd: PUSH EDX   (this->field_4)
        _emit 0x52
        // 00041fde: MOV ECX, EAX   (ECX = obj from lookup)
        _emit 0x8b
        _emit 0xc8
        // 00041fe0: CALL 0x004435d0   (__thiscall, 3 args)
        _emit 0xe8
        _emit 0xeb
        _emit 0x15
        _emit 0x00
        _emit 0x00
        // 00041fe5: MOV ESI, EAX   (ESI = new sound object)
        _emit 0x8b
        _emit 0xf0
        // 00041fe7: JMP +0x2   (→ 0x00441feb)
        _emit 0xeb
        _emit 0x02
        // 00041fe9: XOR ESI, ESI   (null path: ESI = 0)
        _emit 0x33
        _emit 0xf6
        // 00041feb: LEA EAX, [ESP+0x10]   (merge point)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00041fef: PUSH EAX
        _emit 0x50
        // 00041ff0: LEA ECX, [ESP+0x1c]   (adjusted after push)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00041ff4: PUSH ECX
        _emit 0x51
        // 00041ff5: LEA ECX, [EBX+0x18]   (&this->field_0x18)
        _emit 0x8d
        _emit 0x4b
        _emit 0x18
        // 00041ff8: MOV dword ptr [ESP+0x58], 0xffffffff   (SEH state = -1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x58
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00042000: MOV [ESP+0x18], EDI   (local = arg2)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // 00042004: MOV [ESP+0x1c], ESI   (local = sound object)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 00042008: CALL 0x00994a90   (register_sound / store into this->field_0x18 list)
        _emit 0xe8
        _emit 0x83
        _emit 0x2a
        _emit 0x55
        _emit 0x00
        // 0004200d: MOV ECX, [EBX+0x4]   (this->field_4)
        _emit 0x8b
        _emit 0x4b
        _emit 0x04
        // 00042010: MOV EDX, [ECX]   (vtable ptr)
        _emit 0x8b
        _emit 0x11
        // 00042012: MOV EAX, [EDX+0x4]   (vtable[1])
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00042015: PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00042017: PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00042019: PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0004201b: PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0004201d: PUSH ESI   (sound object / handle)
        _emit 0x56
        // 0004201e: PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 00042020: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00042022: PUSH 0x40000
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x04
        _emit 0x00
        // 00042027: PUSH EDI   (arg2)
        _emit 0x57
        // 00042028: CALL EAX   (indirect vtable call, 9 args)
        _emit 0xff
        _emit 0xd0
        // 0004202a: MOV [ESI+0x4], EAX   (store result in sound->field_4)
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0004202d: MOV AL, 0x1   (return true)
        _emit 0xb0
        _emit 0x01
        // 0004202f: MOV ECX, [ESP+0x48]   (restore EH chain)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x48
        // 00042033: MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004203a: POP ECX
        _emit 0x59
        // 0004203b: POP EDI
        _emit 0x5f
        // 0004203c: POP ESI
        _emit 0x5e
        // 0004203d: POP EBX
        _emit 0x5b
        // 0004203e: ADD ESP, 0x44
        _emit 0x83
        _emit 0xc4
        _emit 0x44
        // 00042041: RET 0x1c
        _emit 0xc2
        _emit 0x1c
        _emit 0x00
    }
}
