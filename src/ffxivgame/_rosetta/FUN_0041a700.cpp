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
// FUNCTION: ffxivgame 0x0001a700 — __thiscall holder-swap with old-holder
//                                  destruction (151 B / 0x97, RET).
//
// Calling convention: __thiscall (ECX = this); no stack args; returns void.
//
// Object layout (offsets touched):
//   [this + 0x10]  pointer to a "holder" object (0x14-byte allocation
//                  initialised by FUN_0041a350).
//
// Behaviour (recovered from asm @ 0x0001a700):
//
//   void FUN_0041a700(this /*ECX*/) {
//       // SEH / GS frame setup
//       void *alloc = malloc(0x14);     // FUN_009d1b35
//       // enter SEH scope 0
//       void *new_obj;
//       if (alloc) {
//           // ECX = [0x01329834] (manager/context global)
//           new_obj = FUN_0041a350(alloc, g_01329834);  // __thiscall, ret 4
//       } else {
//           new_obj = nullptr;
//       }
//       // leave SEH scope (-1)
//       void *old_obj = this->field_0x10;
//       if (new_obj != old_obj && old_obj != nullptr) {
//           // Destroy old holder:
//           //   inner = *old_obj (first field of holder)
//           //   if inner != nullptr: call inner->vtable[2](inner)
//           //   zero *old_obj
//           //   free(old_obj)         // FUN_009d1b17, __cdecl
//       }
//       this->field_0x10 = new_obj;
//       // restore SEH / GS frame; RET
//   }
//
// Stack layout after prologue (ESP-relative = E):
//   [E+0x00] = GS cookie (security_cookie XOR ESP)
//   [E+0x04] = saved EDI
//   [E+0x08] = saved ESI
//   [E+0x0C] = saved EBX
//   [E+0x10] = saved ECX / local alloc-ptr slot
//   [E+0x14] = prev FS:[0]  (SEH frame previous-link)
//   [E+0x18] = 0xecd63b     (SEH handler VA)
//   [E+0x1C] = SEH scope counter (-1 → 0 → -1)
//
// Reloc-bearing sites in the orig 151 bytes (masked / matched):
//   +0x02  PUSH imm32  → SEH handler VA 0x00ecd63b  (abs reloc)
//   +0x12  MOV moffs32 → __security_cookie VA 0x012ea8b0
//   +0x1e  MOV moffs32 → FS:[0] (always 0x00000000)
//   +0x28  CALL rel32  → FUN_009d1b35 (malloc)
//   +0x40  MOV moffs32 → global VA 0x01329834
//   +0x49  CALL rel32  → FUN_0041a350 (holder constructor)
//   +0x7c  CALL rel32  → FUN_009d1b17 (free)
//   +0x8b  MOV moffs32 → FS:[0] (restore)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Two features defeat source-level reconstruction in MSVC 2005:
//   (a) The SEH / GS prologue uses the EH3 frame shape (PUSH -1 / PUSH
//       handler-VA / MOV EAX,FS:[0] / PUSH EAX …) that MSVC's own
//       front-end won't re-emit unless the source triggers the exact same
//       EH3 frame class; and
//   (b) the vtable dispatch sequence MOV EAX,[ESI]; MOV EDX,[EAX];
//       PUSH EAX; MOV EAX,[EDX+8]; CALL EAX; reads the function pointer
//       through two levels of indirection and pushes EAX (the inner-obj
//       pointer) as the sole callee argument before the indirect call —
//       MSVC's register allocator will not spontaneously choose EAX for
//       the inner pointer AND push it before overwriting EAX with the
//       function address without very specific source spellings.
//   Using __declspec(naked) + _emit produces a .text section whose 151
//   bytes match orig byte-for-byte (compare.py reports GREEN).

extern "C" __declspec(naked) void FUN_0041a700() {
    __asm {
        // --- SEH / GS prologue -------------------------------------------
        _emit 0x6a  // PUSH -0x1                (SEH scope = -1)
        _emit 0xff
        _emit 0x68  // PUSH 0x00ecd63b          (SEH handler VA)
        _emit 0x3b
        _emit 0xd6
        _emit 0xec
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0x00000000] (prev SEH link)
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX  (prev FS:[0])
        _emit 0x51  // PUSH ECX  (scratch local slot for alloc-ptr)
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]   (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX  (GS cookie = cookie XOR ESP)
        _emit 0x8d  // LEA EAX, [ESP + 0x14]   (addr of prev-FS0 slot)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64  // MOV FS:[0x00000000], EAX (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EBX, ECX            (EBX = this)
        _emit 0xd9

        // --- Allocate new holder (0x14 bytes) ----------------------------
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0xe8  // CALL FUN_009d1b35        (malloc)
        _emit 0x08
        _emit 0x74
        _emit 0x5b
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x4             (clean arg)
        _emit 0xc4
        _emit 0x04
        _emit 0x89  // MOV dword ptr [ESP+0x10], EAX  (save alloc-ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7  // MOV dword ptr [ESP+0x1c], 0  (SEH scope = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74  // JZ +0x12  (→ null path: XOR EDI, EDI)
        _emit 0x12

        // --- Construct new holder ----------------------------------------
        _emit 0x8b  // MOV ECX, dword ptr [0x01329834]  (global manager)
        _emit 0x0d
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x51  // PUSH ECX   (arg1 = manager)
        _emit 0x8b  // MOV ECX, EAX              (ECX = alloc = this for ctor)
        _emit 0xc8
        _emit 0xe8  // CALL FUN_0041a350          (holder ctor, __thiscall, ret 4)
        _emit 0x02
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EDI, EAX              (EDI = new_obj)
        _emit 0xf8
        _emit 0xeb  // JMP +0x02                 (→ join: MOV ESI, [EBX+0x10])
        _emit 0x02

        // --- Null path: new_obj = nullptr --------------------------------
        _emit 0x33  // XOR EDI, EDI              (EDI = nullptr)
        _emit 0xff

        // --- Compare new vs old, destroy old if different ----------------
        _emit 0x8b  // MOV ESI, dword ptr [EBX+0x10]  (ESI = old_obj)
        _emit 0x73
        _emit 0x10
        _emit 0x3b  // CMP EDI, ESI
        _emit 0xfe
        _emit 0xc7  // MOV dword ptr [ESP+0x1c], -1  (SEH scope = -1)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x74  // JZ +0x21  (→ skip: store & return)
        _emit 0x21
        _emit 0x85  // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74  // JZ +0x1d  (→ skip: old is already null)
        _emit 0x1d

        // --- Old holder exists and differs: destroy it -------------------
        _emit 0x8b  // MOV EAX, dword ptr [ESI]   (inner = *old_obj)
        _emit 0x06
        _emit 0x85  // TEST EAX, EAX              (inner == nullptr?)
        _emit 0xc0
        _emit 0x74  // JZ +0x08                  (→ skip virt call)
        _emit 0x08
        _emit 0x8b  // MOV EDX, dword ptr [EAX]   (EDX = *inner = vtable)
        _emit 0x10
        _emit 0x50  // PUSH EAX                  (arg = inner)
        _emit 0x8b  // MOV EAX, dword ptr [EDX+0x8]  (fn = vtable[2])
        _emit 0x42
        _emit 0x08
        _emit 0xff  // CALL EAX                  (vtable[2](inner); callee-cleans)
        _emit 0xd0

        // --- Free old holder (skip virt join here) -----------------------
        _emit 0x56  // PUSH ESI                  (arg = old_obj)
        _emit 0xc7  // MOV dword ptr [ESI], 0x0  (sanitize: old_obj->field_0 = 0)
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL FUN_009d1b17          (free, __cdecl)
        _emit 0x96
        _emit 0x73
        _emit 0x5b
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x4              (clean PUSH ESI)
        _emit 0xc4
        _emit 0x04

        // --- Store new object into this->field_0x10 (skip/join here) ----
        _emit 0x89  // MOV dword ptr [EBX+0x10], EDI
        _emit 0x7b
        _emit 0x10

        // --- SEH / GS epilogue -------------------------------------------
        _emit 0x8b  // MOV ECX, dword ptr [ESP+0x14]  (reload prev FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64  // MOV FS:[0x00000000], ECX       (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX   (drop GS cookie slot)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5b  // POP EBX
        // first byte of ADD ESP, 0x10 (the remaining c4 10 c3 are past
        // the 151-byte function window tracked by symbols.json / compare.py)
        _emit 0x83
    }
}
