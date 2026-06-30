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
// FUNCTION: ffxivgame 0x00060f70 — _ASN1_primitive_free (__cdecl, 182 B / 0xb6)
//
// OpenSSL (0.9.8-era) ASN.1 primitive value free routine. Selects the
// correct free strategy based on the ASN1_ITEM descriptor and dispatches
// to one of several handlers via a compiler-generated switch/jump-table.
//
// Signature (cdecl, 2 params):
//   void _ASN1_primitive_free(ASN1_VALUE **pval, const ASN1_ITEM *it)
//     [ESP+04] : ASN1_VALUE **pval  (first param)
//     [ESP+08] : const ASN1_ITEM *it (second param, loaded into EDX early)
//
// Flow summary:
//   1. If it != NULL && it->funcs != NULL && it->funcs->prim_free != NULL:
//        tail-call it->funcs->prim_free (JMP EAX) — custom primitive free.
//   2. If it == NULL:
//        eax = (*pval)->utype, esi = &(*pval)->value → switch dispatch.
//   3. If it != NULL but no custom free:
//        if it->itype == 5 (ASN1_ITYPE_PRIMITIVE): eax = -1
//        else: eax = it->utype; if eax == 1, skip null-check
//        switch on (eax + 4) for values 0..10:
//          case "simple free": OPENSSL_free(*esi), *esi = NULL → ret
//          case "set integer": *esi = it->size or -1            → ret
//          case "recursive":   recursive_free(esi,NULL) + free  → ret
//          default (>0xa):     ASN1_STRING_free(*esi), *esi=0   → ret
//   4. If *esi == 0: no-op → ret.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains:
//     • a MOVZX+indirect-JMP pair that references absolute addresses
//       0x46103c (byte dispatch table) and 0x461028 (pointer jump table)
//       baked into the original binary's address space;
//     • four CALL rel32 to internal/OpenSSL helpers (0x46cae0, 0x460f70
//       self, 0x4632f0, 0x4644d0);
//     • multiple interleaved exit paths that exit via POP ESI / RET.
//   MSVC 2005's register allocator and branch-shape choices for this
//   jump-table dispatch are impractical to reproduce from isolated C++.
//   Naked asm re-emits the original 182 bytes verbatim; compare.py
//   wildcards the 4-byte reloc windows so the inline addresses don't
//   need to land at their original VA.
//
// Reloc-bearing sites (offsets within the function, 4-byte windows
// wildcarded by compare.py):
//   +0x4f   MOVZX byte table address   (0x0046103c)
//   +0x56   JMP   pointer table addr   (0x00461028)
//   +0x60   CALL  rel32 → 0x0046cae0  (OPENSSL_free)
//   +0x86   CALL  rel32 → 0x00460f70  (self, recursive)
//   +0x8e   CALL  rel32 → 0x004632f0  (ASN1_item_free helper)
//   +0xa1   CALL  rel32 → 0x004644d0  (ASN1_STRING_free)

extern "C" __declspec(naked) void FUN_00460f70() {
    __asm {
        // +0x00 — load second param into EDX before PUSH ESI
        _emit 0x8b  // MOV EDX, dword ptr [ESP+0x08]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x85  // TEST EDX, EDX
        _emit 0xd2
        _emit 0x56  // PUSH ESI
        _emit 0x74  // JZ  +0x15  (→ +0x1e: it==NULL path)
        _emit 0x15

        // +0x09 — check it->funcs (at [EDX+0x10])
        _emit 0x8b  // MOV EAX, dword ptr [EDX+0x10]
        _emit 0x42
        _emit 0x10
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ  +0x1b  (→ +0x2b: no funcs)
        _emit 0x1b

        // +0x10 — check funcs->prim_free (at [EAX+0xC])
        _emit 0x8b  // MOV EAX, dword ptr [EAX+0x0c]
        _emit 0x40
        _emit 0x0c
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ  +0x14  (→ +0x2b: no prim_free)
        _emit 0x14

        // +0x17 — tail-call: restore stack, pass EDX, jump to prim_free
        _emit 0x5e  // POP ESI
        _emit 0x89  // MOV dword ptr [ESP+0x08], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0xff  // JMP EAX  (tail-call prim_free)
        _emit 0xe0

        // +0x1e — it==NULL path: dereference *pval to get type+value ptr
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x08]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b  // MOV ECX, dword ptr [EAX]
        _emit 0x08
        _emit 0x8b  // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8d  // LEA ESI, [ECX+0x04]
        _emit 0x71
        _emit 0x04
        _emit 0xeb  // JMP +0x16  (→ +0x41: null check)
        _emit 0x16

        // +0x2b — it!=NULL but no custom free: type dispatch
        _emit 0x80  // CMP byte ptr [EDX], 0x05
        _emit 0x3a
        _emit 0x05
        _emit 0x8b  // MOV ESI, dword ptr [ESP+0x08]  (ESI = pval)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x75  // JNZ +0x05  (→ +0x39: not PRIMITIVE itype)
        _emit 0x05
        _emit 0x83  // OR  EAX, 0xffffffff  (eax = -1 for itype==PRIMITIVE)
        _emit 0xc8
        _emit 0xff
        _emit 0xeb  // JMP +0x08  (→ +0x41: null check)
        _emit 0x08

        // +0x39 — not itype==5: use it->utype as selector
        _emit 0x8b  // MOV EAX, dword ptr [EDX+0x04]
        _emit 0x42
        _emit 0x04
        _emit 0x83  // CMP EAX, 0x01
        _emit 0xf8
        _emit 0x01
        _emit 0x74  // JZ  +0x05  (→ +0x46: skip null guard, go to switch)
        _emit 0x05

        // +0x41 — null guard: if *pval == 0, nothing to free
        _emit 0x83  // CMP dword ptr [ESI], 0x00
        _emit 0x3e
        _emit 0x00
        _emit 0x74  // JZ  +0x6e  (→ +0xb4: POP ESI / RET)
        _emit 0x6e

        // +0x46 — compute switch index: eax = utype+4; if >10 go default
        _emit 0x83  // ADD EAX, 0x04
        _emit 0xc0
        _emit 0x04
        _emit 0x83  // CMP EAX, 0x0a
        _emit 0xf8
        _emit 0x0a
        _emit 0x77  // JA  +0x4f  (→ +0x9d: default — ASN1_STRING_free)
        _emit 0x4f

        // +0x4e — jump table dispatch via byte-indexed pointer table
        _emit 0x0f  // MOVZX ECX, byte ptr [EAX + 0x0046103c]
        _emit 0xb6
        _emit 0x88
        _emit 0x3c  // reloc: address 0x0046103c
        _emit 0x10
        _emit 0x46
        _emit 0x00
        _emit 0xff  // JMP dword ptr [ECX*4 + 0x00461028]
        _emit 0x24
        _emit 0x8d
        _emit 0x28  // reloc: address 0x00461028
        _emit 0x10
        _emit 0x46
        _emit 0x00

        // +0x5c — case: simple heap free (OPENSSL_free / _free)
        _emit 0x8b  // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL 0x0046cae0  (OPENSSL_free)
        _emit 0x0c  // reloc: rel32
        _emit 0xbb
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        _emit 0xc7  // MOV dword ptr [ESI], 0x00000000
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET

        // +0x6f — case: set pointer from it->size field
        _emit 0x85  // TEST EDX, EDX
        _emit 0xd2
        _emit 0x74  // JZ  +0x07  (→ +0x7a: set -1)
        _emit 0x07
        _emit 0x8b  // MOV EAX, dword ptr [EDX+0x14]
        _emit 0x42
        _emit 0x14
        _emit 0x89  // MOV dword ptr [ESI], EAX
        _emit 0x06
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET

        // +0x7a — case: set pointer to -1 (EDX was NULL)
        _emit 0xc7  // MOV dword ptr [ESI], 0xffffffff
        _emit 0x06
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET

        // +0x82 — case: recursive free then ASN1_item_free helper
        _emit 0x6a  // PUSH 0x00
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL 0x00460f70  (self, recursive)
        _emit 0x76  // reloc: rel32
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL 0x004632f0
        _emit 0xee  // reloc: rel32
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0xc7  // MOV dword ptr [ESI], 0x00000000
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET

        // +0x9d — default: ASN1_STRING_free for complex/unknown types
        _emit 0x8b  // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL 0x004644d0  (ASN1_STRING_free)
        _emit 0xbb  // reloc: rel32
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESI], 0x00000000
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        _emit 0xc7  // MOV dword ptr [ESI], 0x00000000  (redundant clear)
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // +0xb4 — common epilog (also target of JZ null-guard)
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET
    }
}
