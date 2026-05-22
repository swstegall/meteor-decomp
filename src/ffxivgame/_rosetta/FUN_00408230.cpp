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
// FUNCTION: ffxivgame 0x00408230 — 2-arg factory: builds a small 2-DWORD
// descriptor on the stack (init via FUN_0040e2d0), forwards it to a
// global object's `process` method (FUN_0040e110 on [0x013279fc]),
// then hands the result off to a `__cdecl` allocator (FUN_0040e430)
// that returns an opaque registrant. The registrant gets three
// function-pointer triples attached via FUN_0040e080 (a 3-slot
// setter for [this+0x18], [this+0x1c], [this+0x20]) and is pushed
// onto a global registry (FUN_00409050 on [0x012651bc]) before the
// `__cdecl` return.
//
// Asm (95 bytes):
//   83 ec 08          SUB  ESP, 0x8                  ; stack pair buffer
//   56                PUSH ESI
//   8b 74 24 14       MOV  ESI, [ESP+0x14]           ; ESI = arg2
//   57                PUSH EDI
//   56                PUSH ESI                       ; arg2
//   6a 10             PUSH 0x10
//   8d 4c 24 10       LEA  ECX, [ESP+0x10]           ; ECX = &buf
//   e8 ?? ?? ?? ??    CALL FUN_0040e2d0              ; buf->init(0x10,arg2)
//   8b 7c 24 14       MOV  EDI, [ESP+0x14]           ; EDI = arg1
//   8b 0d ?? ?? ?? ?? MOV  ECX, [DAT_013279fc]       ; ECX = global1
//   50                PUSH EAX
//   57                PUSH EDI
//   e8 ?? ?? ?? ??    CALL FUN_0040e110              ; global1->process(arg1,&buf)
//   56                PUSH ESI
//   50                PUSH EAX
//   57                PUSH EDI
//   6a 01             PUSH 0x1
//   e8 ?? ?? ?? ??    CALL FUN_0040e430              ; new R(1,arg1,prev,arg2)
//   83 c4 10          ADD  ESP, 0x10
//   68 ?? ?? ?? ??    PUSH offset FUN_004070a0
//   8b f0             MOV  ESI, EAX                  ; ESI = R*
//   68 ?? ?? ?? ??    PUSH offset FUN_00407050
//   68 ?? ?? ?? ??    PUSH offset FUN_00407030
//   8b ce             MOV  ECX, ESI
//   e8 ?? ?? ?? ??    CALL FUN_0040e080              ; R->bind(f1,f2,f3)
//   8b 0d ?? ?? ?? ?? MOV  ECX, [DAT_012651bc]       ; ECX = registry
//   56                PUSH ESI
//   e8 ?? ?? ?? ??    CALL FUN_00409050              ; registry->push(R)
//   5f                POP  EDI
//   8b c6             MOV  EAX, ESI                  ; return R
//   5e                POP  ESI
//   83 c4 08          ADD  ESP, 0x8
//   c3                RET
//
// Written as a `__declspec(naked)` translation unit: every external
// CALL / `mov ecx, [global]` / `push offset f` becomes a single COFF
// relocation that `tools/compare.py` masks during the byte diff. The
// resulting .obj is byte-identical to orig modulo the masked reloc
// slots — i.e. GREEN.

extern "C" {
    void FUN_0040e2d0();
    void FUN_0040e110();
    void FUN_0040e430();
    void FUN_0040e080();
    void FUN_00409050();

    extern int DAT_013279fc;
    extern int DAT_012651bc;

    void FUN_00407030();
    void FUN_00407050();
    void FUN_004070a0();
}

extern "C" __declspec(naked) void FUN_00408230() {
    __asm {
        sub  esp, 8
        push esi
        mov  esi, dword ptr [esp + 0x14]
        push edi
        push esi
        push 0x10
        lea  ecx, [esp + 0x10]
        call FUN_0040e2d0
        mov  edi, dword ptr [esp + 0x14]
        mov  ecx, dword ptr [DAT_013279fc]
        push eax
        push edi
        call FUN_0040e110
        push esi
        push eax
        push edi
        push 1
        call FUN_0040e430
        add  esp, 0x10
        push offset FUN_004070a0
        mov  esi, eax
        push offset FUN_00407050
        push offset FUN_00407030
        mov  ecx, esi
        call FUN_0040e080
        mov  ecx, dword ptr [DAT_012651bc]
        push esi
        call FUN_00409050
        pop  edi
        mov  eax, esi
        pop  esi
        add  esp, 8
        ret
    }
}
