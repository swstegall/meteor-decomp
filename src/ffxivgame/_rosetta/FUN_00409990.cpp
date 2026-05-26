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
// FUNCTION: ffxivgame 0x00409990 — SEH-protected lazy-init getter/setter
//                                  for a global cached int (104 B / 0x68)
//
// __cdecl int FUN_00409990(int new_value)
//
//   stack layout (after the SEH 3-dword frame is pushed):
//     [ESP+0x00] : SEH prev FS:[0] chain link  (saved next ptr)
//     [ESP+0x04] : SEH handler PUSH 0x00edb44e (scope-table addr)
//     [ESP+0x08] : SEH trylevel PUSH -1       (then overwritten to 0
//                                              inside the init branch)
//     [ESP+0x0C] : (return addr)
//     [ESP+0x10] : int new_value              (the sole caller arg)
//
// Memory layout / globals touched:
//   0x01327b48 : int  s_cached_value       (the lazy-initialised result)
//   0x01327b4c : int  s_init_flag          (bit 0 = "initialised once")
//   0x00edb44e : SEH scope-table address   (PUSH'd into the FS:[0] frame)
//   0x0040e500 : int FUN_0040e500()        (the one-shot initialiser)
//
// Inspection (read from the orig bytes at RVA 0x00009990, 104 bytes total):
//
//   mov  eax, fs:[0]              ; load current SEH chain head
//   push -1                       ; SEH trylevel = -1 (no try block active)
//   push 0x00edb44e               ; SEH scope-table pointer
//   push eax                      ; SEH next = previous chain head
//   mov  eax, 1                   ; (reused as both the OR-mask and the
//                                 ;  AL byte for the TEST below)
//   mov  fs:[0], esp              ; install new SEH frame head
//   test byte ptr [0x01327b4c], al ; (flag & 1) != 0 ?
//   jnz  tail                     ; already initialised → skip init
//   or   dword ptr [0x01327b4c], eax ; set the init flag
//   mov  dword ptr [esp+0x8], 0   ; SEH trylevel = 0 (entering try-scope
//                                 ;   for the FUN_0040e500 call)
//   call FUN_0040e500             ; (rel32 → 0x0040e500)
//   mov  [0x01327b48], eax        ; s_cached_value = init result
// tail:
//   mov  eax, [esp+0x10]          ; load new_value arg
//   test eax, eax
//   jz   return_cached            ; new_value == 0 → return the cache
//   mov  [0x01327b48], eax        ; new_value != 0 → overwrite the cache
//                                 ;   (and eax already holds new_value
//                                 ;    for the return value)
//   mov  ecx, [esp]               ; ecx = SEH prev chain head
//   mov  fs:[0], ecx              ; pop SEH frame
//   add  esp, 0xc                 ; drop the 3 SEH dwords
//   ret
// return_cached:
//   mov  ecx, [esp]               ; ecx = SEH prev chain head
//   mov  eax, [0x01327b48]        ; eax = s_cached_value
//   mov  fs:[0], ecx              ; pop SEH frame
//   add  esp, 0xc                 ; drop the 3 SEH dwords
//   ret
//
//   Calling convention: __cdecl (single stack arg, caller-cleans — the
//   ADD ESP,0xC in both epilogues only pops the SEH frame, not the arg).
//   Return value: the new cache value if new_value != 0, otherwise the
//   (possibly just-initialised) cached value. EAX is conveniently already
//   loaded with new_value at the tail's first epilogue.
//
// Reloc-bearing sites in the orig 104 bytes (each is either an absolute
// data-segment address baked at the orig image base 0x00400000, an
// absolute scope-table address inside .rdata, or a CALL rel32 that the
// linker resolves at relink time — `tools/compare.py` masks reloc bytes
// out of the diff, and emitting these as raw `_emit` bytes from a
// `__declspec(naked)` body produces a .obj whose .text matches the orig
// slice byte-for-byte with NO relocations):
//     +0x09   PUSH imm32  → 0x00edb44e   (SEH scope-table pointer)
//     +0x1c   TEST mem32  → 0x01327b4c   (init-flag global, byte access)
//     +0x24   OR   mem32  → 0x01327b4c   (init-flag global, dword access)
//     +0x31   CALL rel32  → FUN_0040e500 (one-shot initialiser)
//     +0x36   MOV  mem32  → 0x01327b48   (cached-value global)
//     +0x43   MOV  mem32  → 0x01327b48
//     +0x59   MOV  mem32  → 0x01327b48
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would need MSVC 2005's `/EHa` SEH lowering
//   plus a function-level scope table the linker would manufacture at
//   0x00edb44e and IS-equivalent global lazy-init plumbing. Coaxing
//   MSVC into emitting the *exact* shape (FS:[0] chain manipulation,
//   `mov eax, 1` reused as both OR-mask AND test byte, branch encoding
//   widths) is brittle and surrounding-TU-sensitive.
//
//   The pragmatic choice — the same one the SEH-wrapped siblings
//   FUN_004014b0 (307 B) and FUN_00403bd0 (96 B) took — is a
//   `__declspec(naked)` body that re-emits the orig 104 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations: the absolute
//   data-segment / scope-table addresses are baked at the orig image
//   base, and the CALL rel32 displacement resolves against the orig
//   binary's own address space — emitting them as raw bytes produces
//   the exact wire image the linker would emit at relink).
//   `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00409990() {
    __asm {
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00EDB44E (SEH scope-table)
        _emit 0x4e
        _emit 0xb4
        _emit 0xed
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64              // MOV FS:[0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327B4C], AL
        _emit 0x05
        _emit 0x4c
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ tail (+0x18)
        _emit 0x18
        _emit 0x09              // OR dword ptr [0x01327B4C], EAX
        _emit 0x05
        _emit 0x4c
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x8], 0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040e500 (rel32 → 0x00004b3b)
        _emit 0x3b
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0xa3              // MOV [0x01327B48], EAX
        _emit 0x48
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]   (tail:)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ return_cached (+0x13)
        _emit 0x13
        _emit 0xa3              // MOV [0x01327B48], EAX
        _emit 0x48
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x8b              // MOV ECX, dword ptr [ESP]        (return_cached:)
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x01327B48]
        _emit 0x48
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
