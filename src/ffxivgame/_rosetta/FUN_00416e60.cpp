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
// FUNCTION: ffxivgame 0x00016e60 — __thiscall 2-arg wrapper that reads a byte
//                                  field from `this+0x15` as the element count
//                                  and forwards to FUN_00416890 (24 B / 0x18).
//
// Signature (recovered from asm):
//
//   __thiscall bool FUN_00416e60(SomeClass *this, void *p1, void *p2)
//
//   ECX       = this
//   [ESP+4]   = p1  (pointer — forwarded to FUN_00416890 in ECX)
//   [ESP+8]   = p2  (pointer — pushed on stack for FUN_00416890)
//
// Logic:
//
//   count = (unsigned char)(this->byte_at_0x15);   // MOVZX EDX, byte [ECX+0x15]
//   return FUN_00416890(p1, count, p2);             // ECX=p1, EDX=count, [stack]=p2
//
// FUN_00416890 is a byte/dword memory-compare utility (0xdb bytes) that uses
// the non-standard "fastcall-like" convention ECX=ptr1, EDX=count, stack=ptr2,
// with the caller responsible for cleaning the single stack argument.
//
// Asm (24 bytes @ orig RVA 0x00016e60):
//   0f b6 51 15       MOVZX EDX, byte ptr [ECX + 0x15]  ; count
//   8b 44 24 08       MOV   EAX, dword ptr [ESP + 0x8]  ; p2
//   8b 4c 24 04       MOV   ECX, dword ptr [ESP + 0x4]  ; p1 (clobbers this)
//   50                PUSH  EAX                          ; push p2
//   e8 xx xx xx xx    CALL  FUN_00416890                 ; rel32 reloc
//   83 c4 04          ADD   ESP, 0x4                     ; caller cleans p2
//   c2 08 00          RET   0x8                          ; __thiscall: clean 2 args

extern "C" void FUN_00416890();

extern "C" __declspec(naked) void FUN_00416e60() {
    __asm {
        movzx edx, byte ptr [ecx + 0x15]
        mov eax, dword ptr [esp + 8]
        mov ecx, dword ptr [esp + 4]
        push eax
        call FUN_00416890
        add esp, 4
        ret 8
    }
}
