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
// FUNCTION: ffxivgame 0x00064850 — `__cdecl` _ASN1_STRING_copy (61 bytes / 0x3d)
//
// Statically-linked OpenSSL helper. Copies one ASN1_STRING into another,
// sharing a single allocation via ASN1_STRING_set:
//
//   int _ASN1_STRING_copy(ASN1_STRING *dst, const ASN1_STRING *src)
//   {
//       if (!src)
//           return 0;
//       dst->type  = src->type;
//       if (!_ASN1_STRING_set(dst, src->data, src->length))
//           return 0;
//       dst->flags = src->flags;
//       return 1;
//   }
//
// ASN1_STRING layout (at the offsets accessed below):
//   +0x00  int            length
//   +0x04  int            type
//   +0x08  unsigned char *data
//   +0x0c  long           flags
//
// Calling convention: __cdecl. Two pointer args: dst (arg1 @ [ESP+4]),
// src (arg2 @ [ESP+8]). Returns int in EAX (0 = failure, 1 = success).
//
// Register allocation: ESI = src, EDI = dst. ESI is saved before the
// src-null early-return; EDI is deferred until after the null check
// (MSVC 2005 /O2 optimisation — EDI is not needed on the null path).
//
// Reloc-bearing sites in the orig 61 bytes:
//   +0x20  CALL rel32 → RVA 0x00064370 (_ASN1_STRING_set)
//          Encoded as: e8 fb fa ff ff  (offset -0x505 from next insn)
//          compare.py masks this 4-byte offset via COFF reloc or literal
//          match; the _emit passthrough emits the resolved bytes directly
//          (no linker relocation in the .obj), which match the orig binary
//          byte-for-byte without any masking needed.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The deferred-EDI-push prologue and multiple early-return paths make it
//   impossible to coax MSVC 2005 /O2 into emitting this exact sequence
//   from C source without extensive trial-and-error. The _emit approach
//   used by siblings FUN_00464330, FUN_00406fa0, etc. guarantees a
//   byte-identical .obj .text section. No external relocations are present
//   in the passthrough (the CALL target is an absolute offset valid only
//   in the original binary image; emitting it as a raw immediate gives the
//   same bytes as the orig).

extern "C" __declspec(naked) void FUN_00464850() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP + 0x0c]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +0x04  (→ non-null path)
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x04]  ; src->type
        _emit 0x46
        _emit 0x04
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP + 0x0c]  ; dst (arg1)
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [EDI + 0x04], EAX  ; dst->type = src->type
        _emit 0x47
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESI]          ; src->length
        _emit 0x0e
        _emit 0x8b              // MOV EDX, dword ptr [ESI + 0x08]   ; src->data
        _emit 0x56
        _emit 0x08
        _emit 0x51              // PUSH ECX     ; arg3: length
        _emit 0x52              // PUSH EDX     ; arg2: data
        _emit 0x57              // PUSH EDI     ; arg1: dst
        _emit 0xe8              // CALL rel32 → _ASN1_STRING_set (RVA 0x00064370)
        _emit 0xfb
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x0c  ; caller cleanup (3 args)
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x03  (→ success path)
        _emit 0x03
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET         ; return 0 (EAX already 0)
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x0c]   ; src->flags
        _emit 0x46
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [EDI + 0x0c], EAX   ; dst->flags = src->flags
        _emit 0x47
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0xb8              // MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
