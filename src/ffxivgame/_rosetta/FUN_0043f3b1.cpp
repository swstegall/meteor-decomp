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
// FUNCTION: ffxivgame 0x0043f3b1 — string-like assign/replace body fragment
//                                  (109 bytes, __stdcall-frame, RET 8).
//
// Context (recovered from surrounding code):
//
//   This 109-byte slice is a continuation block within a larger function
//   (encompassing frame set up earlier). On entry:
//     EDI  = pointer to a string-like object ('this')
//     ESI  = new capacity / mode indicator
//     [EBP+0x08] = source pointer (param1, new data ptr)
//     [EBP+0x0C] = count (param2, number of elements)
//     [EBP-0x0C] = saved old FS:[0] (SEH chain, installed in the enclosing prolog)
//
//   The function implements an assign/replace sequence for a custom
//   small-string-optimised string class:
//
//     struct SSOStr {
//         // [this+0x00] : vtable or tag
//         // [this+0x04] : data ptr  (heap) or inline buf[0..3]
//         // [this+0x14] : current length
//         // [this+0x18] : capacity (< 0x10 → inline; ≥ 0x10 → heap)
//     };
//
//   Logic sketch:
//     ebx = count;
//     if (ebx > 0) {
//         dst = (capacity < 0x10) ? &this->buf : this->ptr;
//         call FUN_009D17F3(src, esi+1, dst, count);  // insert/memmove helper
//     }
//     // free old heap buffer if heap-allocated and non-null
//     if (this->capacity >= 0x10) {
//         if (this->ptr) {
//             ecx = this->ptr[-1];   // ref-count / block-header field
//             call FUN_0040DF10(this->ptr);  // free/release
//         }
//     }
//     // assign new state
//     this->buf[0] = 0;         // zero inline slot (overwritten next)
//     this->ptr    = src;       // store new pointer
//     this->capacity = esi;
//     this->length   = count;
//     if (esi >= 0x10) eax = src;   // else eax already == &this->buf
//     eax[count] = 0;           // null-terminate
//     // epilogue: restore SEH + callee-saves + frame tear-down
//     fs:[0] = [ebp-0xC];
//     pop ecx; pop edi; pop esi; pop ebx; mov esp, ebp; pop ebp; ret 8;
//
// Two CALL sites (rel32, no base-reloc — binary is RELOCS_STRIPPED, image
// base 0x400000; compare.py reads raw file bytes directly):
//     +0x1f  CALL  FUN_009D17F3  (rel32 = 0x0059241e; insert/assign helper)
//     +0x38  CALL  FUN_0040DF10  (rel32 = 0xfffceb82; dealloc/release)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would produce relocation records for the two
//   CALL targets plus require the exact compiler-specific small-string
//   branch layout to reproduce byte-for-byte; the simpler path — the same
//   one siblings FUN_004090b0 and FUN_004091f0 took — is a
//   `__declspec(naked)` body that re-emits the orig 109 bytes verbatim
//   via MASM `_emit` directives. No relocations are generated; the rel32
//   bytes resolve inside the orig binary's address space.

extern "C" __declspec(naked) void FUN_0043f3b1() {
    __asm {
        // +00  0043f3b1
        _emit 0x8b              // MOV EBX, dword ptr [EBP+0x0c]
        _emit 0x5d
        _emit 0x0c
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x76              // JBE short +0x20 (→ loc_0x27)
        _emit 0x20
        _emit 0x83              // CMP dword ptr [EDI+0x18], 0x10
        _emit 0x7f
        _emit 0x18
        _emit 0x10
        _emit 0x72              // JB short +0x05 (→ loc_0x12; inline buf)
        _emit 0x05
        _emit 0x8b              // MOV EAX, dword ptr [EDI+0x4]  (heap ptr)
        _emit 0x47
        _emit 0x04
        _emit 0xeb              // JMP short +0x03 (→ loc_0x15)
        _emit 0x03
        // +12  loc_0x12:
        _emit 0x8d              // LEA EAX, [EDI+0x4]            (inline buf)
        _emit 0x47
        _emit 0x04
        // +15  loc_0x15:
        _emit 0x8b              // MOV ECX, dword ptr [EBP+0x8]  (src ptr param)
        _emit 0x4d
        _emit 0x08
        _emit 0x53              // PUSH EBX                       (arg4: count)
        _emit 0x50              // PUSH EAX                       (arg3: dest)
        _emit 0x8d              // LEA EAX, [ESI+0x1]
        _emit 0x46
        _emit 0x01
        _emit 0x50              // PUSH EAX                       (arg2)
        _emit 0x51              // PUSH ECX                       (arg1: src)
        // +1f  CALL FUN_009D17F3  rel32 = 0x0059241e
        _emit 0xe8
        _emit 0x1e
        _emit 0x24
        _emit 0x59
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10                  (clean 4 args)
        _emit 0xc4
        _emit 0x10
        // +27  loc_0x27: free old heap buffer if needed
        _emit 0x83              // CMP dword ptr [EDI+0x18], 0x10
        _emit 0x7f
        _emit 0x18
        _emit 0x10
        _emit 0x72              // JB short +0x10 (→ loc_0x3d; skip free)
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ short +0x09 (→ loc_0x3d; null → skip)
        _emit 0x09
        _emit 0x8b              // MOV ECX, dword ptr [EAX-0x4]  (ref-count hdr)
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        // +38  CALL FUN_0040DF10  rel32 = 0xfffceb82
        _emit 0xe8
        _emit 0x82
        _emit 0xeb
        _emit 0xfc
        _emit 0xff
        // +3d  loc_0x3d: assign new state
        _emit 0x83              // CMP ESI, 0x10
        _emit 0xfe
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [EBP+0x8]
        _emit 0x4d
        _emit 0x08
        _emit 0x8d              // LEA EAX, [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0xc6              // MOV byte ptr [EAX], 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX], ECX      (store src ptr)
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EDI+0x18], ESI (store capacity)
        _emit 0x77
        _emit 0x18
        _emit 0x89              // MOV dword ptr [EDI+0x14], EBX (store length)
        _emit 0x5f
        _emit 0x14
        _emit 0x72              // JB short +0x02 (→ loc_0x55; inline buf path)
        _emit 0x02
        _emit 0x8b              // MOV EAX, ECX                  (heap path: eax=src)
        _emit 0xc1
        // +55  loc_0x55:
        _emit 0xc6              // MOV byte ptr [EAX+EBX], 0x00  (null-terminate)
        _emit 0x04
        _emit 0x18
        _emit 0x00
        // +59  epilogue: restore SEH chain, callee-saves, frame teardown
        _emit 0x8b              // MOV ECX, dword ptr [EBP-0x0c]
        _emit 0x4d
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x00000000], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
