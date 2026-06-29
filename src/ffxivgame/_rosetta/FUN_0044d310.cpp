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
// FUNCTION: ffxivgame 0x0004d310 — Sqex::Login::SqexIdAuthentication scalar
//                                  deleting destructor (39 B / 0x27,
//                                  __thiscall + 1 stack arg)
//
// RTTI: vtable at RVA 0xb6742c = Sqex::Login::SqexIdAuthentication::vftable
//       (1 slot — this is the sole virtual function)
//
// Pattern: vtable-restore + member-dtor-at-0x34 + conditional __cdecl
//   operator-delete variant.  Identical shape to FUN_00403cb0 (the
//   30-byte "bare" scalar-deleting dtor cluster), with 9 extra bytes
//   inserted before the real_dtor CALL for the vtable restore + LEA.
//   Compare to:
//     FUN_00403cb0 (30B, bare: push/mov/call_dtor/test/jz+9/push/call_op_delete/add_esp4/mov_eax/pop/ret4)
//     FUN_004ebbd0 (32B, thiscall-allocator: jz+11 → no_delete before mov eax)
//
// Asm (39 bytes — note: symbols.json said 36; compare.py size_overrides
//      expanded to 39 by detecting the trailing RET imm16 04 00):
//   56                       PUSH ESI                    ; save ESI
//   8b f1                    MOV ESI, ECX                ; ESI = this
//   8d 4e 34                 LEA ECX, [ESI+0x34]         ; ECX = &this->member_0x34
//   c7 06 GG GG GG GG        MOV dword ptr [ESI], vftable ; restore vtable (DIR32 reloc)
//   e8 RR RR RR RR           CALL FUN_00446f50            ; ~member_0x34() (thiscall)
//   f6 44 24 08 01           TEST byte ptr [ESP+8], 0x1  ; test delete flag
//   74 09                    JZ +9  → no_delete (before mov eax,esi)
//   56                       PUSH ESI                    ; arg = this
//   e8 RR RR RR RR           CALL FUN_009d1b17            ; operator delete(this) [__cdecl]
//   83 c4 04                 ADD ESP, 4                  ; clean __cdecl arg
// no_delete:
//   8b c6                    MOV EAX, ESI                ; return value = this
//   5e                       POP ESI                     ; restore ESI
//   c2 04 00                 RET 4                       ; clean 1 stack arg
//
// JZ displacement 9 = sizeof(push esi)[1] + sizeof(call)[5] +
//                     sizeof(add esp,4)[3] = 9.  Identical to the
//   FUN_00403cb0 cluster's JZ+9 pattern (op-delete via __cdecl with
//   caller cleanup).
//
// Reloc-bearing bytes (masked by tools/compare.py during the byte diff):
//   +0x08  vtable address   (4-byte DIR32 reloc → Sqex_Login_SqexIdAuthentication_vftable)
//   +0x0d  CALL FUN_00446f50 rel32 (4-byte REL32 reloc)
//   +0x1a  CALL FUN_009d1b17 rel32 (4-byte REL32 reloc)

// Member destructor — __thiscall, ECX = &this->member_0x34.
extern "C" void FUN_00446f50();

// Operator delete — takes this-pointer as stack arg; callee cleans (thiscall/stdcall).
extern "C" void FUN_009d1b17();

// Sqex::Login::SqexIdAuthentication vtable (RVA 0xb6742c in the orig binary).
// Declared as an extern so the assembler generates a DIR32 reloc for
// the MOV-immediate encoding (c7 06 GG GG GG GG).
extern "C" void* Sqex_Login_SqexIdAuthentication_vftable;

extern "C" __declspec(naked) void FUN_0044d310()
{
    __asm {
        push esi                                            // 56
        mov esi, ecx                                       // 8b f1
        lea ecx, [esi + 0x34]                              // 8d 4e 34
        mov dword ptr [esi], OFFSET Sqex_Login_SqexIdAuthentication_vftable // c7 06 GG GG GG GG
        call FUN_00446f50                                   // e8 RR RR RR RR
        test byte ptr [esp + 8], 1                         // f6 44 24 08 01
        jz no_delete                                       // 74 09
        push esi                                           // 56
        call FUN_009d1b17                                  // e8 RR RR RR RR
        add esp, 4                                         // 83 c4 04
    no_delete:
        mov eax, esi                                       // 8b c6
        pop esi                                            // 5e
        ret 4                                              // c2 04 00
    }
}
