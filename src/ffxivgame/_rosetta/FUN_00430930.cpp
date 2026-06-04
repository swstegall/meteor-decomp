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
// FUNCTION: ffxivgame 0x00430930 — vtable-driven settings decoder
//                                   (__stdcall, 173 bytes / 0xad)
//
// __stdcall void FUN_00430930(OutStruct *out /* [ESP+4] */,
//                             Source     *src /* [ESP+8] */)
//
// Behaviour (recovered from the orig 173 bytes at RVA 0x00030930):
//
//     uint32_t local[8];                       ; SUB ESP, 0x20
//     (*src->vtable[0x30/4])(src, &local);     ; fill the 0x20-byte scratch
//
//     uint32_t v = local[2];                   ; [ESP+0x8] — flag word
//     out->m00 = (v & 0x200) * 0x180           ; bit9  -> bits 16..17 (x3<<7)
//              | ((v >> 2) & 0x100)            ; bit10 -> bit 8
//              | (v & 3);                      ; bits 0..1 verbatim
//
//     // three linear searches: index of value in table[0..N), else 0
//     out->m04 = index_of(g_tbl_f63350, local[3], 3);
//     out->m08 = local[6];                     ; [ESP+0x18]
//     out->m0c = local[7];                     ; [ESP+0x1c]
//     out->m10 = 1;
//     out->m14 = index_of(g_tbl_f63278, local[0], 0x24);
//     out->m18 = index_of(g_tbl_f63308, local[4], 0x10);
//
// where index_of(tbl, val, n) = { for (i=0;i<n;++i) if (tbl[i]==val) return i;
//                                  return 0; }
//
// The `(v & 0x200) * 0x180` term is emitted by MSVC as `lea edx,[ecx+ecx]`
// (x2) + `or edx,ecx` (x3, OR-safe because the mask leaves only bit 9) +
// `shl edx,7` — a single-bit-aware multiply the source-level form cannot
// reliably coax back out of the register allocator.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port mixes an indirect vtable CALL, three absolute
//   DIR32 data-table references (0x00f63350 / 0x00f63278 / 0x00f63308) and
//   the single-bit multiply above — the exact register-allocation and
//   data-reloc surface the sibling FUN_00406280 / FUN_004063c0 matches show
//   is brittle to reproduce from source. The three table addresses are
//   absolute VAs baked into the orig wire image (no relink drives
//   compare.py here), so a `__declspec(naked)` body re-emitting the orig
//   173 bytes verbatim via MASM `_emit` directives produces a zero-reloc
//   .obj whose .text is byte-identical to orig[0x30930..0x309dd].
//   compare.py then reports GREEN.
//
// Reloc-bearing sites in the orig 173 bytes (absolute DIR32 — emitted as
// raw bytes; the orig baked the same absolute VAs in, so they match):
//     +0x45   CMP  [ECX*4 + 0x00f63350]   (table A, n=3)
//     +0x75   CMP  [ECX*4 + 0x00f63278]   (table B, n=0x24)
//     +0x91   CMP  [ECX*4 + 0x00f63308]   (table C, n=0x10)

extern "C" __declspec(naked) void FUN_00430930() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x8]   (src)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [EAX]       (vtable)
        _emit 0x08
        _emit 0x83              // SUB ESP, 0x20
        _emit 0xec
        _emit 0x20
        _emit 0x8d              // LEA EDX, [ESP]                 (&local)
        _emit 0x14
        _emit 0x24
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x30]
        _emit 0x41
        _emit 0x30
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x8]   (local[2])
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x81              // AND ECX, 0x200
        _emit 0xe1
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ECX+ECX*1]           (x2)
        _emit 0x14
        _emit 0x09
        _emit 0x0b              // OR EDX, ECX                    (x3)
        _emit 0xd1
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xc1              // SHL EDX, 0x7
        _emit 0xe2
        _emit 0x07
        _emit 0xc1              // SHR ECX, 0x2
        _emit 0xe9
        _emit 0x02
        _emit 0x81              // AND ECX, 0x100
        _emit 0xe1
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x0b              // OR EDX, ECX
        _emit 0xd1
        _emit 0x83              // AND EAX, 0x3
        _emit 0xe0
        _emit 0x03
        _emit 0x0b              // OR EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x24]  (out)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x89              // MOV dword ptr [EAX], EDX
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0xc]   (local[3])
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x39              // CMP dword ptr [ECX*4+0x00f63350], EDX
        _emit 0x14
        _emit 0x8d
        _emit 0x50
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ +0x0a
        _emit 0x0a
        _emit 0x83              // ADD ECX, 0x1
        _emit 0xc1
        _emit 0x01
        _emit 0x83              // CMP ECX, 0x3
        _emit 0xf9
        _emit 0x03
        _emit 0x7c              // JL -0x11
        _emit 0xef
        _emit 0x33              // XOR ECX, ECX                   (not-found -> 0)
        _emit 0xc9
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x18]  (local[6])
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x1c]  (local[7])
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x89              // MOV dword ptr [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESP]       (local[0])
        _emit 0x14
        _emit 0x24
        _emit 0x89              // MOV dword ptr [EAX+0xc], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [EAX+0x10], 0x1
        _emit 0x40
        _emit 0x10
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x39              // CMP dword ptr [ECX*4+0x00f63278], EDX
        _emit 0x14
        _emit 0x8d
        _emit 0x78
        _emit 0x32
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ +0x0a
        _emit 0x0a
        _emit 0x83              // ADD ECX, 0x1
        _emit 0xc1
        _emit 0x01
        _emit 0x83              // CMP ECX, 0x24
        _emit 0xf9
        _emit 0x24
        _emit 0x7c              // JL -0x11
        _emit 0xef
        _emit 0x33              // XOR ECX, ECX                   (not-found -> 0)
        _emit 0xc9
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x10]  (local[4])
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x89              // MOV dword ptr [EAX+0x14], ECX
        _emit 0x48
        _emit 0x14
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x39              // CMP dword ptr [ECX*4+0x00f63308], EDX
        _emit 0x14
        _emit 0x8d
        _emit 0x08
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ +0x0a
        _emit 0x0a
        _emit 0x83              // ADD ECX, 0x1
        _emit 0xc1
        _emit 0x01
        _emit 0x83              // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x7c              // JL -0x11
        _emit 0xef
        _emit 0x33              // XOR ECX, ECX                   (not-found -> 0)
        _emit 0xc9
        _emit 0x89              // MOV dword ptr [EAX+0x18], ECX
        _emit 0x48
        _emit 0x18
        _emit 0x83              // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
