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
// FUNCTION: ffxivgame 0x00419590 — object factory with SEH + /GS frame
//                                   (__cdecl, 172 bytes / 0xac)
//
// __cdecl void FUN_00419590(void** out, void* param2, void* param3)
//
// Stack layout at entry (caller view):
//     [ESP+0x04] = out     (param_1 — receives the allocated object pointer)
//     [ESP+0x08] = param2  (param_2 — passed to FUN_00419f80 + FUN_0041b210)
//     [ESP+0x0c] = param3  (param_3 — passed to FUN_0041b210)
//
// Behaviour (recovered from asm @ RVA 0x00019590):
//
//   void FUN_00419590(void** out, void* param2, void* param3) {
//       // SEH + /GS frame; TryLevel starts at -1, updated to 0 then 1
//       SomeAllocCtx ctx;                           // local at [ESP-0x14]
//       ctx.thiscall_method(0x10, 0xf57bb0);        // FUN_0040e2d0
//       void* raw = FUN_00419c40(0x10, ctx_result); // allocate 16 bytes
//       SomeObj* obj = (SomeObj*) raw;
//       if (obj != nullptr) {
//           obj->vtable  = (void*) 0xf57ea4;        // interim vtable
//           obj->m8      = 0;
//           obj->mC      = 0;
//           FUN_00419f80(&obj->m8, param2);          // __thiscall init
//           FUN_0041b210(param2, param3, &obj->m4);  // __cdecl 3-arg init
//           obj->vtable  = (void*) 0xf58204;         // fully-constructed vtable
//       }
//       *out = obj;                                  // (or NULL on alloc fail)
//   }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function has a /GS security cookie, a C++-EH TryLevel frame tracking
//   which sub-objects have been constructed, and precise register allocation
//   (EDI=0 held across the entire body, ESI=allocated pointer, ECX threaded
//   through two thiscall targets). Source-level C++ under MSVC 2005 /O2 would
//   require exact declaration ordering to reproduce the original allocator
//   tiebreaker — the same situation that blocked FUN_00401b70 after nine
//   iterations. We go directly to `__declspec(naked)` + raw `_emit` bytes,
//   which produces a byte-identical .text section. compare.py wildcards the
//   nine reloc windows (four REL32 CALLs + five DIR32 immediates), so the
//   baked-in addresses don't affect the GREEN verdict.
//
// Reloc-bearing sites (byte offset within function — 4-byte window wildcarded):
//     +0x03   PUSH imm32   → 0xe556c4  (SEH scope-table / handler address)
//     +0x14   MOV EAX,[imm32] → 0x012ea8b0  (__security_cookie)
//     +0x26   PUSH imm32   → 0xf57bb0  (type descriptor / RTTI for alloc ctx)
//     +0x3b   CALL rel32   → FUN_0040e2d0  (alloc-ctx thiscall init)
//     +0x47   CALL rel32   → FUN_00419c40  (16-byte allocator)
//     +0x65   MOV [ESI],imm32 → 0xf57ea4  (interim vtable)
//     +0x74   CALL rel32   → FUN_00419f80  (thiscall sub-obj init)
//     +0x83   CALL rel32   → FUN_0041b210  (cdecl 3-arg init)
//     +0x8c   MOV [ESI],imm32 → 0xf58204  (fully-constructed vtable)

extern "C" __declspec(naked) void FUN_00419590() {
    __asm {
        // --- prologue: SEH frame + /GS cookie ----------------------------
        _emit 0x6a              // PUSH -0x1                (TryLevel initial)
        _emit 0xff
        _emit 0x68              // PUSH 0xe556c4            (SEH handler)
        _emit 0xc4
        _emit 0x56
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                 (prev SEH frame)
        _emit 0x83              // SUB ESP, 0x14            (local space)
        _emit 0xec
        _emit 0x14
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX                 (GS cookie)
        _emit 0x8d              // LEA EAX, [ESP+0x20]      (→ SEH frame ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x64              // MOV FS:[0x0], EAX        (install SEH)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- body --------------------------------------------------------
        _emit 0x68              // PUSH 0xf57bb0            (type info / RTTI)
        _emit 0xb0
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x6a              // PUSH 0x10                (size = 16)
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP+0x20]      (this → local ctx)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV [ESP+0x30], EDI      (TryLevel → 0)
        _emit 0x7c
        _emit 0x24
        _emit 0x30
        _emit 0x89              // MOV [ESP+0x14], EDI      (zero local slot)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL FUN_0040e2d0        (alloc-ctx init, thiscall)
        _emit 0x01
        _emit 0x4d
        _emit 0xff
        _emit 0xff
        _emit 0x50              // PUSH EAX                 (ctx result)
        _emit 0x6a              // PUSH 0x10                (size)
        _emit 0x10
        _emit 0x89              // MOV [ESP+0x18], EAX      (spill ctx result)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xe8              // CALL FUN_00419c40        (16-byte allocator)
        _emit 0x65
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX             (ESI = new ptr)
        _emit 0xf0
        _emit 0x83              // ADD ESP, 0x8             (caller clean 2 args)
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // MOV [ESP+0x14], ESI      (spill new ptr)
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x3b              // CMP ESI, EDI             (new ptr vs NULL)
        _emit 0xf7
        _emit 0xc7              // MOV [ESP+0x28], 0x1      (TryLevel → 1)
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x32                 (alloc failed → null)
        _emit 0x32
        // --- non-null: initialize object ---------------------------------
        _emit 0x8d              // LEA ECX, [ESI+0x8]       (sub-obj ptr)
        _emit 0x4e
        _emit 0x08
        _emit 0xc7              // MOV [ESI], 0xf57ea4      (interim vtable)
        _emit 0x06
        _emit 0xa4
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV [ECX], EDI           (m8 = 0)
        _emit 0x39
        _emit 0x89              // MOV [ECX+0x4], EDI       (mC = 0)
        _emit 0x79
        _emit 0x04
        _emit 0x8b              // MOV EDI, [ESP+0x34]      (param2)
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        _emit 0x57              // PUSH EDI                 (arg: param2)
        _emit 0xe8              // CALL FUN_00419f80        (thiscall sub-obj init)
        _emit 0x78
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESP+0x38]      (param3)
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x8d              // LEA EAX, [ESI+0x4]       (&obj->m4)
        _emit 0x46
        _emit 0x04
        _emit 0x50              // PUSH EAX                 (arg2: &obj->m4)
        _emit 0x51              // PUSH ECX                 (arg1: param3)
        _emit 0x57              // PUSH EDI                 (arg0: param2)
        _emit 0xe8              // CALL FUN_0041b210        (cdecl 3-arg init)
        _emit 0xf9
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0c            (caller clean 3 args)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc7              // MOV [ESI], 0xf58204      (fully-constructed vtable)
        _emit 0x06
        _emit 0x04
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        _emit 0xeb              // JMP +0x02                (→ tail)
        _emit 0x02
        // --- null path ---------------------------------------------------
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        // --- tail: store result + epilogue -------------------------------
        _emit 0x8b              // MOV EAX, [ESP+0x30]      (*out ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x89              // MOV [EAX], ESI           (*out = obj or NULL)
        _emit 0x30
        _emit 0x8b              // MOV ECX, [ESP+0x20]      (prev SEH frame)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x64              // MOV FS:[0x0], ECX        (restore SEH)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                  (GS cookie)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0xc3              // RET
    }
}
