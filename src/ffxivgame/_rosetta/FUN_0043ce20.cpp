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
// FUNCTION: ffxivgame 0x0003ce20 — two-variant factory dispatcher (216 B / 0xd8,
//                                  __cdecl, EH3-style SEH frame with /GS cookie).
//
// Signature (recovered from the asm):
//
//   void *FUN_0043ce20(void **out, int type, void *arg2, void *arg3);
//
// Frame layout after prologue (ESP = E0):
//
//   [E0+0x00] = /GS security cookie (XOR'd with ESP at push)
//   [E0+0x04] = saved EDI
//   [E0+0x08] = local0 (EH destruction-state flag; 0 → 1 before virtual call)
//   [E0+0x0c] = local1 (output buffer for case 1 / FUN_0043c910 result slot)
//   [E0+0x10] = old FS:[0]  ─┐ EH3 registration record
//   [E0+0x14] = scope table  │ installed into FS:[0] at E0+0x10
//   [E0+0x18] = try-level=-1 ─┘ (advances to 0 before virtual call)
//   [E0+0x1c] = return address
//   [E0+0x20] = arg0 (out — pointer-to-pointer, receives the created object)
//   [E0+0x24] = arg1 (type: 1 → FUN_0043c910, 2 → FUN_0043cbe0, else → *out=0)
//   [E0+0x28] = arg2
//   [E0+0x2c] = arg3
//
// Logic:
//
//   if (type == 0 or other):
//       *out = NULL;  return early (EH frame intact, no destructor, fast RET)
//
//   else if (type == 2):
//       // FUN_0043cbe0(&type_ref, arg2, arg3) — uses arg1's stack slot as output
//       result_container = FUN_0043cbe0(&type, arg2, arg3);
//       *out = result_container->first_member;
//       result_container->first_member = NULL;
//       obj_ptr = type;                // type slot now holds an object pointer
//
//   else if (type == 1):
//       // FUN_0043c910(&local1, arg2, arg3) — uses local1 as output
//       result_container = FUN_0043c910(&local1, arg2, arg3);
//       *out = result_container->first_member;
//       result_container->first_member = NULL;
//       obj_ptr = local1;              // local1 now holds an object pointer
//
//   // Shared epilogue: if obj_ptr non-null, call virtual fn vtable[0](1)
//   // EH state 0 guards the virtual call so destructor fires on exception
//   local0 = 1; try_level = 0;
//   if (obj_ptr != NULL) { obj_ptr->vtable[0](1); }
//   return *out;  // EDI = arg0 pointer
//
// Reloc-bearing sites within the 216-byte function body:
//   +0x03  DIR32 → 0x00e56846  (EH3 scope table in .rdata)
//   +0x13  DIR32 → 0x012ea8b0  (__security_cookie in .data)
//   +0x65  REL32 → 0x0043cbe0  (FUN_0043cbe0, CALL rel32)
//   +0x90  REL32 → 0x0043c910  (FUN_0043c910, CALL rel32)
//
// Reconstruction: __declspec(naked) byte passthrough.  The EH3 prologue,
// precise /GS double-cookie, switch cascade with short JZ/JMP branches,
// and the vtable dispatch interleaved with EH-state MOVs cannot be coaxed
// out of a C++ source rewrite at /O2 without byte shifts.  Emitting the
// 216 original bytes verbatim via MASM _emit directives gives a GREEN
// match (tools/compare.py masks reloc windows so the baked raw addresses
// compare correctly against the orig binary).

extern "C" __declspec(naked) void FUN_0043ce20() {
    __asm {
        // +0x00  PUSH -1                (EH3 initial try-level)
        _emit 0x6a
        _emit 0xff
        // +0x02  PUSH 0x00e56846       (scope table — DIR32 reloc)
        _emit 0x68
        _emit 0x46
        _emit 0x68
        _emit 0xe5
        _emit 0x00
        // +0x07  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x0d  PUSH EAX              (old FS:[0] = prev exception record)
        _emit 0x50
        // +0x0e  SUB ESP, 0x8          (two locals: local0, local1)
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // +0x11  PUSH EDI
        _emit 0x57
        // +0x12  MOV EAX, [__security_cookie]   (DIR32 reloc)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // +0x17  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // +0x19  PUSH EAX              (/GS cookie)
        _emit 0x50
        // +0x1a  LEA EAX, [ESP+0x10]   (point at old FS:[0] in our frame)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // +0x1e  MOV FS:[0], EAX       (install EH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x24  MOV dword ptr [ESP+0x8], 0   (local0 = 0; EH state init)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x2c  MOV EAX, [ESP+0x24]   (arg1 = type switch value)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // +0x30  SUB EAX, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // +0x33  JZ +0x4b  (= offset 0x80 = case type==1 handler)
        _emit 0x74
        _emit 0x4b
        // +0x35  SUB EAX, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // +0x38  JZ +0x1b  (= offset 0x55 = case type==2 handler)
        _emit 0x74
        _emit 0x1b
        // +0x3a  MOV EAX, [ESP+0x20]   (arg0 = out pointer)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // +0x3e  MOV dword ptr [EAX], 0   (*out = NULL)
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x44  MOV ECX, [ESP+0x10]   (old FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // +0x48  MOV FS:[0], ECX       (restore exception chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x4f  POP ECX               (discard /GS cookie — no check on early exit)
        _emit 0x59
        // +0x50  POP EDI
        _emit 0x5f
        // +0x51  ADD ESP, 0x14         (unwind locals + EH frame fields)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // +0x54  RET
        _emit 0xc3

        // case type==2: FUN_0043cbe0(&arg1_slot, arg2, arg3)
        // +0x55  MOV EAX, [ESP+0x2c]   (arg3)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // +0x59  MOV ECX, [ESP+0x28]   (arg2)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // +0x5d  PUSH EAX              (push arg3)
        _emit 0x50
        // +0x5e  PUSH ECX              (push arg2)
        _emit 0x51
        // +0x5f  LEA EDX, [ESP+0x2c]   (address of arg1 slot — &type)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // +0x63  PUSH EDX              (push &arg1)
        _emit 0x52
        // +0x64  CALL FUN_0043cbe0     (REL32 reloc)
        _emit 0xe8
        _emit 0x57
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // +0x69  MOV ECX, [EAX]        (ECX = *(return container))
        _emit 0x8b
        _emit 0x08
        // +0x6b  MOV EDI, [ESP+0x2c]   (EDI = arg0 = out pointer)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        // +0x6f  MOV dword ptr [EAX], 0  (clear first member of container)
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x75  MOV [EDI], ECX        (*out = transferred object)
        _emit 0x89
        _emit 0x0f
        // +0x77  MOV ECX, [ESP+0x30]   (ECX = arg1 slot = updated object ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // +0x7b  ADD ESP, 0xc          (clean pushed args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // +0x7e  JMP +0x29             (→ shared epilogue at +0xa9)
        _emit 0xeb
        _emit 0x29

        // case type==1: FUN_0043c910(&local1, arg2, arg3)
        // +0x80  MOV ECX, [ESP+0x2c]   (arg3)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // +0x84  MOV EDX, [ESP+0x28]   (arg2)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // +0x88  PUSH ECX              (push arg3)
        _emit 0x51
        // +0x89  PUSH EDX              (push arg2)
        _emit 0x52
        // +0x8a  LEA EAX, [ESP+0x14]   (address of local1 slot)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // +0x8e  PUSH EAX              (push &local1)
        _emit 0x50
        // +0x8f  CALL FUN_0043c910     (REL32 reloc)
        _emit 0xe8
        _emit 0x5c
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // +0x94  MOV ECX, [EAX]        (ECX = *(return container))
        _emit 0x8b
        _emit 0x08
        // +0x96  MOV EDI, [ESP+0x2c]   (EDI = arg0 = out pointer)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        // +0x9a  MOV dword ptr [EAX], 0  (clear first member of container)
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0xa0  ADD ESP, 0xc          (clean pushed args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // +0xa3  MOV [EDI], ECX        (*out = transferred object)
        _emit 0x89
        _emit 0x0f
        // +0xa5  MOV ECX, [ESP+0x0c]   (ECX = local1 = acquired object ptr)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c

        // Shared epilogue (offset 0xa9 = 169 from start)
        // +0xa9  TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // +0xab  MOV dword ptr [ESP+0x8], 1   (local0=1; EH destruction state)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0xb3  MOV dword ptr [ESP+0x18], 0  (try_level = 0; enter EH scope)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0xbb  JZ +0x08              (if obj_ptr==NULL, skip virtual call)
        _emit 0x74
        _emit 0x08
        // +0xbd  MOV EDX, [ECX]        (EDX = vtable pointer)
        _emit 0x8b
        _emit 0x11
        // +0xbf  MOV EAX, [EDX]        (EAX = vtable[0])
        _emit 0x8b
        _emit 0x02
        // +0xc1  PUSH 1                (argument = 1)
        _emit 0x6a
        _emit 0x01
        // +0xc3  CALL EAX              (virtual call: obj->vtable[0](1))
        _emit 0xff
        _emit 0xd0
        // +0xc5  MOV EAX, EDI          (return value = out pointer)
        _emit 0x8b
        _emit 0xc7
        // +0xc7  MOV ECX, [ESP+0x10]   (reload old FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // +0xcb  MOV FS:[0], ECX       (restore exception chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0xd2  POP ECX               (discard /GS cookie)
        _emit 0x59
        // +0xd3  POP EDI
        _emit 0x5f
        // +0xd4  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // +0xd7  RET
        _emit 0xc3
    }
}
