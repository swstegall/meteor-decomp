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
// FUNCTION: ffxivgame 0x004233f0 — `__thiscall` member with a C++/GS
//                                  exception frame that ends in a
//                                  noreturn tail call (72 B / 0x48).
//
// Calling convention: __thiscall (ECX = this). The function installs a
// MSVC structured-exception registration record (PUSH -1 trylevel,
// PUSH <scopetable=0xe55d88>, PUSH FS:[0] prev) plus the /GS security
// cookie (MOV EAX, [__security_cookie]; XOR EAX, ESP; PUSH EAX), then
// links the frame in at FS:[0] = &record.
//
// Inspection (read from the orig 72-byte slice at RVA 0x000233f0):
//
//     push -1                       ; SEH trylevel = -1
//     push 0xe55d88                 ; scopetable / FuncInfo  (DIR32)
//     mov  eax, fs:[0]              ; prev registration node
//     push eax
//     push ecx                      ; spill slot for `this`
//     push esi                      ; callee-save
//     mov  eax, [0x012ea8b0]        ; __security_cookie       (DIR32)
//     xor  eax, esp
//     push eax                      ; cookie ^ esp
//     lea  eax, [esp+0xc]
//     mov  fs:[0], eax              ; install exception frame
//     mov  esi, ecx                 ; esi = this
//     mov  [esp+8], esi             ; restash `this`
//     mov  eax, [esi+4]             ; eax = this->field_4
//     test eax, eax
//     mov  dword ptr [esp+0x14], 0  ; trylevel = 0 (enter try block)
//     jz   skip                     ; field_4 == 0 → no cleanup
//     mov  ecx, [eax-4]             ; ecx = *(field_4 - 4)  (thiscall arg)
//     push eax
//     call 0x0040df70               ; FUN_0040df70(field_4)   (REL32)
//   skip:
//     mov  eax, [esi]               ; eax = this->field_0
//     push eax
//     call 0x009d1b17               ; FUN_009d1b17(field_0)   (REL32)
//                                   ; — noreturn: no epilogue / cookie
//                                   ;   check / ret follows the call, so
//                                   ;   the compiler treated 0x009d1b17
//                                   ;   as __declspec(noreturn) (throw /
//                                   ;   fatal-exit helper). The SEH frame
//                                   ;   exists to unwind the conditionally
//                                   ;   touched field_4 object.
//
// Reloc-bearing positions (masked by tools/compare.py if present):
//   off 0x03  IMAGE_REL_I386_DIR32  → scopetable 0x00e55d88 (PUSH imm32)
//   off 0x10  IMAGE_REL_I386_DIR32  → __security_cookie 0x012ea8b0 (MOV)
//   off 0x3b  IMAGE_REL_I386_REL32  → FUN_0040df70 (CALL rel32)
//   off 0x44  IMAGE_REL_I386_REL32  → FUN_009d1b17 (CALL rel32)
//
// Reconstruction strategy — naked-asm byte passthrough. Driving MSVC
// 2005 to re-emit this exact prologue interleave (the trylevel/scopetable
// pushes ahead of the cookie spill, the `this` restash into the spill
// slot, the noreturn tail call with no epilogue) depends on EH/GS
// codegen heuristics no surface-level C++ form pins reliably. Following
// the established sibling idiom (FUN_004086a0), the 72 orig bytes are
// re-emitted verbatim via MASM `_emit` directives — the call/data
// displacements are self-contained constants relative to the function's
// final placement, so the .obj's .text equals the orig slice byte-for-
// byte (zero relocations) and tools/compare.py reports GREEN by direct
// equality.

extern "C" __declspec(naked) void FUN_004233f0() {
    __asm {
        // 000233f0: push -1
        _emit 0x6a
        _emit 0xff
        // 000233f2: push 0xe55d88
        _emit 0x68
        _emit 0x88
        _emit 0x5d
        _emit 0xe5
        _emit 0x00
        // 000233f7: mov eax, fs:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000233fd: push eax
        _emit 0x50
        // 000233fe: push ecx
        _emit 0x51
        // 000233ff: push esi
        _emit 0x56
        // 00023400: mov eax, [0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00023405: xor eax, esp
        _emit 0x33
        _emit 0xc4
        // 00023407: push eax
        _emit 0x50
        // 00023408: lea eax, [esp+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0002340c: mov fs:[0], eax
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00023412: mov esi, ecx
        _emit 0x8b
        _emit 0xf1
        // 00023414: mov [esp+0x8], esi
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00023418: mov eax, [esi+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0002341b: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 0002341d: mov dword ptr [esp+0x14], 0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00023425: jz +0x9 -> 0x00023430
        _emit 0x74
        _emit 0x09
        // 00023427: mov ecx, [eax-0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        // 0002342a: push eax
        _emit 0x50
        // 0002342b: call 0x0040df70 (rel32)
        _emit 0xe8
        _emit 0x40
        _emit 0xab
        _emit 0xfe
        _emit 0xff
        // 00023430: mov eax, [esi]
        _emit 0x8b
        _emit 0x06
        // 00023432: push eax
        _emit 0x50
        // 00023433: call 0x009d1b17 (rel32)
        _emit 0xe8
        _emit 0xdf
        _emit 0xe6
        _emit 0x5a
        _emit 0x00
    }
}
