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
// FUNCTION: ffxivgame 0x00019ce0 — FUN_00419ce0 (__thiscall, 26 B)
//
// Derived-class constructor that:
//  1. Receives `this` in ECX and one argument (init-struct pointer) at [ESP+4].
//  2. Calls FUN_00431710 (__thiscall, ECX=this, arg1=init_struct, arg2=this)
//     to populate the base portion of the object.
//  3. Overwrites the vtable pointer at [this] with 0x0105d5ac (the derived
//     class vftable, overriding the 0xf638d4 base vtable stamped by the
//     base ctor).
//  4. Returns this in EAX.
//
// This is the same derived-ctor pattern seen at 0x00018d00 / 0x00018e00
// (both call FUN_00431710 then stamp the same 0x0105d5ac vftable) but
// stripped of the surrounding factory logic — here the object is already
// allocated and the constructor body is a pure 26-byte thiscall.
//
// Asm (26 bytes @ RVA 0x00019ce0):
//   8b 44 24 04        MOV  EAX, dword ptr [ESP+4]   ; load init-struct arg
//   56                 PUSH ESI                       ; callee-save ESI
//   8b f1              MOV  ESI, ECX                  ; ESI = this
//   56                 PUSH ESI                       ; arg2 for base ctor
//   50                 PUSH EAX                       ; arg1 for base ctor
//   e8 22 7a 01 00     CALL FUN_00431710              ; base ctor (rel32 reloc)
//   c7 06 ac d5 05 01  MOV  dword ptr [ESI], 0x0105d5ac ; derived vtable
//   8b c6              MOV  EAX, ESI                  ; return this
//   5e                 POP  ESI
//   c2 04 00           RET  4                         ; callee cleans 1 arg

extern "C" {

// FUN_00431710: base-class constructor at VA 0x00431710.
// __thiscall (ECX = this); two stack args: arg1 = init-struct ptr, arg2 = parent/this ptr.
// Epilogue is RET 8 (callee cleans both args).
void FUN_00431710();

// Derived vftable at VA 0x0105d5ac.
// Slot 0 is the derived destructor.  Referenced via DIR32 reloc on the MOV immediate.
int vftable_0105d5ac;

__declspec(naked) void FUN_00419ce0() {
    __asm {
        mov     eax, dword ptr [esp + 4]
        push    esi
        mov     esi, ecx
        push    esi
        push    eax
        call    FUN_00431710
        mov     dword ptr [esi], offset vftable_0105d5ac
        mov     eax, esi
        pop     esi
        ret     4
    }
}

}  // extern "C"
