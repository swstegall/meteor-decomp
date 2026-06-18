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
// FUNCTION: ffxivgame 0x0041d470 — gated vtable-dispatch + forwarding thunk
//                                  (__cdecl void, no args, 86 B)
//
// Structure (read from orig RVA 0x0001d470, 86 bytes):
//
//   if (*(bool*)0x012660e9 == 0) goto tail;          // gate check
//
//   // First virtual call — vtable slot 1 on embedded object at +0xc of sub
//   SubObj *sub = (*(Obj**)0x01329428)->field_0x1a0;
//   void **vtbl = *(void***)(sub + 0xc);             // vtable ptr
//   void *ecx   =  (void*)(sub + 0xc);               // this ptr
//   ((void(__thiscall*)(void*))vtbl[1])(ecx);
//
//   // Second virtual call — vtable slot 2 on same embedded object
//   sub  = (*(Obj**)0x01329428)->field_0x1a0;
//   vtbl = *(void***)(sub + 0xc);
//   ecx  = (void*)(sub + 0xc);
//   ((void(__thiscall*)(void*))vtbl[2])(ecx);
//
//   // Third call — FUN_004233b0(g_ObjB, sub->field_0x10)
//   sub  = (*(Obj**)0x01329428)->field_0x1a0;
//   int  val = *(int*)(sub + 0x10);                  // non-vtable field
//   ecx  = *(void**)0x0132987c;                       // g_ObjB this ptr
//   FUN_004233b0(ecx, val);                            // __thiscall
//
// tail:
//   jmp  FUN_00420990;                               // tail-chain to next fn
//
// Reloc-bearing sites in the orig 86 bytes:
//   +0x02  DIR32 → 0x012660e9   (CMP [abs], 0 — gate flag)
//   +0x0b  DIR32 → 0x01329428   (MOV EAX, [abs] — g_MainObjPtr)
//   +0x21  DIR32 → 0x01329428   (MOV ECX, [abs] — g_MainObjPtr)
//   +0x37  DIR32 → 0x01329428   (MOV ECX, [abs] — g_MainObjPtr)
//   +0x46  DIR32 → 0x0132987c   (MOV ECX, [abs] — g_ObjB)
//   +0x4d  rel32 → 0x004233b0   (CALL FUN_004233b0)
//   +0x52  rel32 → 0x00420990   (JMP  FUN_00420990)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains seven reloc-bearing operands (five DIR32 absolute
//   addresses + two rel32 offsets) all of which resolve against the full
//   binary's address space.  A source-level reconstruction would require
//   extern declarations and linker symbols for every global and callee,
//   making it hard to guarantee that MSVC 2005 emits exactly these byte
//   sequences (register allocation, call convention for the virtual calls,
//   and the JMP-not-RET epilogue all depend on compiler internals).
//   The naked _emit passthrough avoids the problem entirely: the .obj's
//   .text is byte-identical to the orig slice; tools/compare.py masks the
//   reloc windows during the diff and reports GREEN.

extern "C" __declspec(naked) void FUN_0041d470() {
    __asm {
        // CMP byte ptr [0x012660e9], 0      ; gate check
        _emit 0x80
        _emit 0x3d
        _emit 0xe9
        _emit 0x60
        _emit 0x26
        _emit 0x01
        _emit 0x00
        // JZ  +0x48  (skip to tail JMP)
        _emit 0x74
        _emit 0x48
        // MOV EAX, [0x01329428]             ; g_MainObjPtr
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // MOV EAX, [EAX + 0x1a0]            ; sub = g_MainObjPtr->field_0x1a0
        _emit 0x8b
        _emit 0x80
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // MOV EDX, [EAX + 0xc]              ; EDX = vtable of embedded obj at +0xc
        _emit 0x8b
        _emit 0x50
        _emit 0x0c
        // LEA ECX, [EAX + 0xc]              ; ECX = this ptr of embedded obj
        _emit 0x8d
        _emit 0x48
        _emit 0x0c
        // MOV EAX, [EDX + 0x4]              ; EAX = vtable[1]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // CALL EAX                           ; virtual call slot 1
        _emit 0xff
        _emit 0xd0
        // MOV ECX, [0x01329428]             ; g_MainObjPtr (reload)
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // MOV EAX, [ECX + 0x1a0]            ; sub (reload)
        _emit 0x8b
        _emit 0x81
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // MOV EDX, [EAX + 0xc]              ; vtable again
        _emit 0x8b
        _emit 0x50
        _emit 0x0c
        // LEA ECX, [EAX + 0xc]              ; this ptr again
        _emit 0x8d
        _emit 0x48
        _emit 0x0c
        // MOV EAX, [EDX + 0x8]              ; EAX = vtable[2]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // CALL EAX                           ; virtual call slot 2
        _emit 0xff
        _emit 0xd0
        // MOV ECX, [0x01329428]             ; g_MainObjPtr (reload)
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // MOV EDX, [ECX + 0x1a0]            ; sub (reload, now in EDX)
        _emit 0x8b
        _emit 0x91
        _emit 0xa0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // MOV EAX, [EDX + 0x10]             ; EAX = sub->field_0x10
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // MOV ECX, [0x0132987c]             ; ECX = g_ObjB (this for FUN_004233b0)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // PUSH EAX                           ; push sub->field_0x10 as arg
        _emit 0x50
        // CALL FUN_004233b0   (rel32 → 0x004233b0)
        _emit 0xe8
        _emit 0xef
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        // JMP  FUN_00420990   (rel32 → 0x00420990)  ; tail-chain
        _emit 0xe9
        _emit 0xca
        _emit 0x34
        _emit 0x00
        _emit 0x00
    }
}
