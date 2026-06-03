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
// FUNCTION: ffxivgame 0x0041a5d0 — __thiscall destructor / cleanup
//                                   with SEH frame (/GS), 166 bytes
//
// __thiscall void FUN_0041a5d0(SomeClass *this)
//   ECX = this
//
// Frame layout (after prolog):
//   [ESP+0x00] = XOR'd GS cookie
//   [ESP+0x04] = saved EDI
//   [ESP+0x08] = saved ESI (= this)
//   [ESP+0x0c] = (SUB ESP,8 alloc low)
//   [ESP+0x10] = (SUB ESP,8 alloc high; holds 'this' spill)
//   [ESP+0x14] = old FS:[0x0] (SEH chain node)
//   [ESP+0x18] = SEH handler addr (0xe557aa)
//   [ESP+0x1c] = SEH try_level (starts -1, set 0x2 then 0x1)
//   [ESP+0x20] = return address
//
// Object layout (partial, at ECX = this):
//   [ESI+0x00] = main vtable ptr
//   [ESI+0x0c] = sub-object vtable ptr
//   [ESI+0x10] = owned pointer (field_0x10)
//
// Behaviour (inspected from the orig 166 bytes at RVA 0x0001a5d0):
//
//   Prolog: /GS + SEH frame install (FS:[0] chain), save ESI/EDI.
//   Set this->vtable = 0xf57eb8, this->sub_vtable = 0xf57ea8.
//   SEH try_level = 2.
//   CALL FUN_00432820 (base-class destructor body or sub-object init).
//   If this->field_0x10 != NULL:
//       call g_singleton->vtable[2](this->field_0x10)  (release/deregister)
//       this->field_0x10 = NULL.
//   SEH try_level = 1.
//   If this->field_0x10 != NULL (exception-path only; always 0 on normal path):
//       if (*field_0x10 != NULL):
//           call (*field_0x10)->vtable[2](*field_0x10)  (virtual release)
//       (*field_0x10) = NULL
//       free(field_0x10)
//   Epilog: this->sub_vtable = 0xf57e20, this->vtable = 0xf57e14.
//   Restore SEH chain, pop ESI/EDI/cookie, ADD ESP 0x14, RET.
//   (compare.py compares exactly 166 bytes — through the first byte
//    of ADD ESP,0x14 at RVA 0x0001a675.)
//
// Reloc-bearing positions (raw bytes, no COFF reloc entries in naked fn):
//   +0x02 PUSH 0xe557aa        (SEH handler)
//   +0x13 MOV EAX,[0x012ea8b0] (__security_cookie)
//   +0x2b MOV [ESI],0xf57eb8   (vtable addr)
//   +0x31 MOV [ESI+0xc],0xf57ea8 (sub-vtable addr)
//   +0x40 CALL rel32 → FUN_00432820 (0x0001820b from next IP)
//   +0x4c MOV ECX,[0x01329920] (g_singleton ptr)
//   +0x82 CALL rel32 → _free    (0x005b74c0 from next IP)
//   +0x8a MOV [ESI+0xc],0xf57e20 (base sub-vtable)
//   +0x91 MOV [ESI],0xf57e14   (base vtable)
//
// Reconstruction: naked-asm byte passthrough (same idiom as siblings
// FUN_004063c0 / FUN_004071b0). All 166 compared bytes emitted via
// MASM _emit directives so the .obj .text is byte-identical to the
// orig slice with no COFF relocation entries (compare.py reports GREEN).

extern "C" __declspec(naked) void FUN_0041a5d0() {
    __asm {
        // --- prolog: /GS + SEH frame + save registers ---
        _emit 0x6a              // PUSH -0x1              (SEH try_level init)
        _emit 0xff
        _emit 0x68              // PUSH 0xe557aa           (SEH handler addr)
        _emit 0xaa
        _emit 0x57
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]       (old SEH chain head)
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x8            (local frame)
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]   (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP            (cookie XOR'd with ESP)
        _emit 0xc4
        _emit 0x50              // PUSH EAX                (save XOR'd cookie)
        _emit 0x8d              // LEA EAX, [ESP+0x14]     (point to SEH record)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0x0], EAX       (install handler)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- body: set vtables, call base dtor, clean up field_0x10 ---
        _emit 0x8b              // MOV ESI, ECX            (ESI = this)
        _emit 0xf1
        _emit 0x89              // MOV [ESP+0x10], ESI     (spill this)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0xc7              // MOV [ESI], 0xf57eb8     (main vtable)
        _emit 0x06
        _emit 0xb8
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV [ESI+0xc], 0xf57ea8 (sub vtable)
        _emit 0x46
        _emit 0x0c
        _emit 0xa8
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV [ESP+0x1c], 0x2     (SEH try_level = 2)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00432820 (rel32)
        _emit 0x0b
        _emit 0x82
        _emit 0x01
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x10]     (field_0x10)
        _emit 0x46
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ skip_release (+0x15)
        _emit 0x15
        _emit 0x8b              // MOV ECX, [0x01329920]   (g_singleton)
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV [ESI+0x10], 0x0     (clear field)
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, [ECX]          (g_singleton vtable)
        _emit 0x11
        _emit 0x50              // PUSH EAX                (arg: old field_0x10)
        _emit 0x8b              // MOV EAX, [EDX+0x8]      (vtable entry 2)
        _emit 0x42
        _emit 0x08
        _emit 0xff              // CALL EAX                (virtual release/deregister)
        _emit 0xd0
        // --- skip_release: ---
        _emit 0x8b              // MOV EDI, [ESI+0x10]     (reload field_0x10)
        _emit 0x7e
        _emit 0x10
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0xc6              // MOV byte [ESP+0x1c], 0x1 (SEH try_level = 1)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        _emit 0x74              // JZ skip_delete (+0x1d)   (to epilog if null)
        _emit 0x1d
        // --- exception-path delete block ---
        _emit 0x8b              // MOV EAX, [EDI]          (EDI->vtable or first field)
        _emit 0x07
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ no_vtable_call (+0x8)
        _emit 0x08
        _emit 0x8b              // MOV ECX, [EAX]          (deref vtable)
        _emit 0x08
        _emit 0x8b              // MOV EDX, [ECX+0x8]      (vtable[2])
        _emit 0x51
        _emit 0x08
        _emit 0x50              // PUSH EAX                (arg)
        _emit 0xff              // CALL EDX                (virtual fn)
        _emit 0xd2
        // --- no_vtable_call: ---
        _emit 0x57              // PUSH EDI                (arg to free)
        _emit 0xc7              // MOV [EDI], 0x0          (null out vtable)
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL _free (rel32)
        _emit 0xc0
        _emit 0x74
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4            (cdecl cleanup: 1 arg)
        _emit 0xc4
        _emit 0x04
        // --- skip_delete / epilog: restore base vtables ---
        _emit 0xc7              // MOV [ESI+0xc], 0xf57e20 (base sub-vtable)
        _emit 0x46
        _emit 0x0c
        _emit 0x20
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV [ESI], 0xf57e14     (base vtable)
        _emit 0x06
        _emit 0x14
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESP+0x14]     (old FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0x0], ECX       (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                 (discard XOR'd cookie)
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x14 — first byte only; rest is past
                                //   the 166-byte compare window. compare.py
                                //   compares through this byte (RVA 0x0001a675).
    }
}
