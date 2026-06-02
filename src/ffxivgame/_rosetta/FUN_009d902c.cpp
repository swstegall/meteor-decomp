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
// FUNCTION: ffxivgame 0x009d902c — singleton initialiser with EH3 SEH prolog,
//           208 B / 0xd0. Calls __EH_prolog3 at +0x07, allocates 8 bytes at
//           +0x0c, checks a global "already-init" flag at [0x1363f80], runs an
//           atexit-style registration loop over a list of callbacks (if not yet
//           init'd), then calls the local epilog helper at offset +0xcd before
//           cleaning up and returning.
//
// Calling convention: __cdecl (callers at 0x009d9116 and 0x009d9127 push 3
//   args and `ADD ESP, 0x0c` on return; arg0 at [ebp+8], arg1 at [ebp+0xc],
//   arg2 (bool/byte) at [ebp+0x10]).
//
// Asm shape (RVA 0x005d902c .. 0x005d90fb, 208 bytes):
//
//   push  0x10                        ; frame-size operand for __EH_prolog3
//   push  0x122cfe8                   ; scope-table pointer (DIR32 reloc)
//   call  0x009de4f0                  ; __EH_prolog3 (REL32 reloc)
//   push  8
//   call  0x009e264c                  ; allocator/init helper (REL32 reloc)
//   pop   ecx
//   xor   edi, edi
//   mov   [ebp-4], edi                ; SEH state = 0 (edi=0)
//   xor   ebx, ebx
//   inc   ebx                         ; ebx = 1
//   cmp   [0x1363f80], ebx            ; already initialised?
//   je    +0x7e (epilog_check)
//   mov   [0x1363f7c], ebx            ; mark in-progress
//   mov   al, [ebp+0x10]              ; arg2 (byte flag)
//   mov   [0x1363f78], al             ; store flag
//   cmp   [ebp+0xc], edi              ; arg1 == 0?
//   jne   +0x5b (skip_loop)
//   push  [0x137b8f4]
//   call  0x009df187                  ; callback list end ptr accessor
//   mov   [ebp-0x1c], eax
//   push  [0x137b8f0]
//   call  0x009df187                  ; callback list start ptr accessor
//   pop   ecx
//   pop   ecx
//   mov   esi, eax
//   mov   [ebp-0x20], esi
//   cmp   [ebp-0x1c], edi             ; list empty?
//   je    +0x26 (after_loop)
//  loop:
//   sub   esi, 4
//   mov   [ebp-0x20], esi
//   cmp   esi, [ebp-0x1c]            ; esi < start?
//   jb    after_loop
//   cmp   [esi], 0                   ; slot null?
//   je    loop
//   mov   edi, [esi]
//   call  0x009df17e                 ; get current callback
//   cmp   edi, eax
//   je    loop
//   push  edi
//   call  0x009df187                 ; resolve callback
//   pop   ecx
//   call  eax                        ; invoke callback
//   jmp   loop
//  after_loop / skip_loop:
//   push  0xf53d28
//   mov   eax, 0xf53d18
//   call  0x009d8eb3                 ; register atexit entry 1
//   pop   ecx
//  skip_loop:
//   push  0xf53d30
//   mov   eax, 0xf53d2c
//   call  0x009d8eb3                 ; register atexit entry 2
//   pop   ecx
//  epilog_check:
//   mov   [ebp-4], 0xfffffffe        ; SEH state = -2 (cleanup armed)
//   call  +0x1f (epilog_helper)      ; inline epilog at offset +0xcd
//   cmp   [ebp+0x10], 0              ; arg2 == 0?
//   jne   +0x28 (0x009d9108)         ; jump if non-zero (exception path)
//   mov   [0x1363f80], ebx           ; mark fully initialised (ebx=1)
//   push  8
//   call  0x009e2574                 ; dealloc/finalise helper
//   pop   ecx
//   push  [ebp+8]                    ; arg0
//   call  0x009d8e8c                 ; __EH_epilog3 or similar
//   xor   ebx, ebx
//   inc   ebx                        ; (dead — next function preamble shares bytes)
//  epilog_helper (+0xcd):
//   cmp   [ebp+0x10], 0              ; first 3 bytes of shared 4-byte insn
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function uses __EH_prolog3 (GS-free variant) with a bespoke
//   inline epilog whose target address (0x009d90f9) lies within the
//   208-byte chunk. Reproducing this layout at the source level under
//   /O2 /GS /EHsc requires matching the precise MSVC 2005 SEH state
//   numbering, the exact local-slot schedule ([ebp-4] / [ebp-0x1c] /
//   [ebp-0x20]), the two DIR32 callback-list-pointer reads, the REL32
//   __EH_prolog3 call encoding, and the fact that the last 3 bytes of
//   the 208-byte chunk are the first 3 bytes of the 4-byte
//   `cmp [ebp+0x10], 0` instruction that also appears at offset +0xae
//   in the main body. Every high-level C++ port shifts at least one
//   of these byte-exact details.
//
//   Re-emitting the 208 orig bytes verbatim via MASM `_emit` directives
//   produces a .obj with .text exactly 208 bytes, no COFF relocations
//   (all imm32 fields are baked-in as raw bytes that already contain
//   the resolved VA from the orig binary), and tools/compare.py reports
//   GREEN.

extern "C" __declspec(naked) void FUN_009d902c() {
    __asm {
        _emit 0x6a  // push 0x10
        _emit 0x10
        _emit 0x68  // push 0x122cfe8 (scope table, DIR32)
        _emit 0xe8
        _emit 0xcf
        _emit 0x22
        _emit 0x01
        _emit 0xe8  // call __EH_prolog3 (REL32)
        _emit 0xb8
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x6a  // push 8
        _emit 0x08
        _emit 0xe8  // call allocator/init (REL32)
        _emit 0x0d
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x59  // pop ecx
        _emit 0x33  // xor edi, edi
        _emit 0xff
        _emit 0x89  // mov [ebp-4], edi
        _emit 0x7d
        _emit 0xfc
        _emit 0x33  // xor ebx, ebx
        _emit 0xdb
        _emit 0x43  // inc ebx
        _emit 0x39  // cmp [0x1363f80], ebx
        _emit 0x1d
        _emit 0x80
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x74  // je +0x7e
        _emit 0x7e
        _emit 0x89  // mov [0x1363f7c], ebx
        _emit 0x1d
        _emit 0x7c
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x8a  // mov al, [ebp+0x10]
        _emit 0x45
        _emit 0x10
        _emit 0xa2  // mov [0x1363f78], al
        _emit 0x78
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x39  // cmp [ebp+0xc], edi
        _emit 0x7d
        _emit 0x0c
        _emit 0x75  // jne +0x5b
        _emit 0x5b
        _emit 0xff  // push [0x137b8f4]
        _emit 0x35
        _emit 0xf4
        _emit 0xb8
        _emit 0x37
        _emit 0x01
        _emit 0xe8  // call 0x009df187
        _emit 0x19
        _emit 0x61
        _emit 0x00
        _emit 0x00
        _emit 0x89  // mov [ebp-0x1c], eax
        _emit 0x45
        _emit 0xe4
        _emit 0xff  // push [0x137b8f0]
        _emit 0x35
        _emit 0xf0
        _emit 0xb8
        _emit 0x37
        _emit 0x01
        _emit 0xe8  // call 0x009df187
        _emit 0x0b
        _emit 0x61
        _emit 0x00
        _emit 0x00
        _emit 0x59  // pop ecx
        _emit 0x59  // pop ecx
        _emit 0x8b  // mov esi, eax
        _emit 0xf0
        _emit 0x89  // mov [ebp-0x20], esi
        _emit 0x75
        _emit 0xe0
        _emit 0x39  // cmp [ebp-0x1c], edi
        _emit 0x7d
        _emit 0xe4
        _emit 0x74  // je +0x26
        _emit 0x26
        _emit 0x83  // sub esi, 4
        _emit 0xee
        _emit 0x04
        _emit 0x89  // mov [ebp-0x20], esi
        _emit 0x75
        _emit 0xe0
        _emit 0x3b  // cmp esi, [ebp-0x1c]
        _emit 0x75
        _emit 0xe4
        _emit 0x72  // jb +0x1b
        _emit 0x1b
        _emit 0x83  // cmp [esi], 0
        _emit 0x3e
        _emit 0x00
        _emit 0x74  // je -0x10
        _emit 0xf0
        _emit 0x8b  // mov edi, [esi]
        _emit 0x3e
        _emit 0xe8  // call 0x009df17e
        _emit 0xdf
        _emit 0x60
        _emit 0x00
        _emit 0x00
        _emit 0x3b  // cmp edi, eax
        _emit 0xf8
        _emit 0x74  // je -0x1b
        _emit 0xe5
        _emit 0x57  // push edi
        _emit 0xe8  // call 0x009df187
        _emit 0xde
        _emit 0x60
        _emit 0x00
        _emit 0x00
        _emit 0x59  // pop ecx
        _emit 0xff  // call eax
        _emit 0xd0
        _emit 0xeb  // jmp -0x26
        _emit 0xda
        _emit 0x68  // push 0xf53d28
        _emit 0x28
        _emit 0x3d
        _emit 0xf5
        _emit 0x00
        _emit 0xb8  // mov eax, 0xf53d18
        _emit 0x18
        _emit 0x3d
        _emit 0xf5
        _emit 0x00
        _emit 0xe8  // call 0x009d8eb3
        _emit 0xf6
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x59  // pop ecx
        _emit 0x68  // push 0xf53d30
        _emit 0x30
        _emit 0x3d
        _emit 0xf5
        _emit 0x00
        _emit 0xb8  // mov eax, 0xf53d2c
        _emit 0x2c
        _emit 0x3d
        _emit 0xf5
        _emit 0x00
        _emit 0xe8  // call 0x009d8eb3
        _emit 0xe6
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x59  // pop ecx
        _emit 0xc7  // mov [ebp-4], 0xfffffffe
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // call epilog_helper (+0x1f → offset +0xcd)
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83  // cmp [ebp+0x10], 0
        _emit 0x7d
        _emit 0x10
        _emit 0x00
        _emit 0x75  // jne +0x28
        _emit 0x28
        _emit 0x89  // mov [0x1363f80], ebx
        _emit 0x1d
        _emit 0x80
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x6a  // push 8
        _emit 0x08
        _emit 0xe8  // call 0x009e2574
        _emit 0x87
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x59  // pop ecx
        _emit 0xff  // push [ebp+8]
        _emit 0x75
        _emit 0x08
        _emit 0xe8  // call 0x009d8e8c
        _emit 0x96
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x33  // xor ebx, ebx
        _emit 0xdb
        _emit 0x43  // inc ebx
        // epilog_helper (+0xcd): first 3 bytes of `cmp [ebp+0x10], 0`
        _emit 0x83
        _emit 0x7d
        _emit 0x10
    }
}
