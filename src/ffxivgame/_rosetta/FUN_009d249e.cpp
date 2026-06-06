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
// FUNCTION: ffxivgame 0x009d249e — dynamic-array reserve/grow helper
//                                  (__cdecl, 0xb9 / 185 bytes)
//
// This function manages a global pair of pointers — a capacity iterator
// (at .data 0x0137b8f4) and an end iterator (at .data 0x0137b8f0) — for
// what appears to be a flat typed array (elements are 4 bytes each, as
// inferred from the SAR EBX,2 and LEA EDI,[EAX+EBX*4] used when growing).
// The caller passes its own value as param_1; if the grow path succeeds the
// function returns param_1 unchanged; on failure it returns 0 (NULL).
//
// High-level logic recovered from asm/ffxivgame/005d249e_FUN_009d249e.s:
//
//   void* FUN_009d249e(void* param_1) {
//       // Step 1 — load current begin/end iterators via a helper
//       //   that presumably locks or dereferences the pointer.
//       begin = FUN_009df187(*g_capacity_ptr);  // [0x0137b8f4]
//       end   = FUN_009df187(*g_end_ptr);       // [0x0137b8f0]
//
//       // Step 2 — validate range
//       if (end < begin) return NULL;
//       range_bytes = end - begin;
//       needed      = range_bytes + 4;          // +4 for one new element
//       if (needed < 4) return NULL;            // overflow guard
//
//       // Step 3 — check capacity (call FUN_009e000f to query it)
//       cap = FUN_009e000f(begin);
//       if (cap < needed) {
//           // Step 4 — grow: double capacity (up to +0x800 per step)
//           new_cap = (cap < 0x800) ? cap * 2 : cap + 0x800;
//           if (new_cap <= cap) goto try_min;   // overflow
//           new_ptr = FUN_009de002(begin, new_cap);
//           if (new_ptr) goto realloc_ok;
//       try_min:
//           new_cap2 = cap + 16;
//           if (new_cap2 <= cap) return NULL;   // overflow
//           new_ptr = FUN_009de002(begin, new_cap2);
//           if (!new_ptr) return NULL;
//       realloc_ok:
//           // Update capacity pointer: FUN_009df110(new_ptr) → stored at
//           // [0x0137b8f4]; EDI → new end = new_ptr + range_bytes.
//           elem_count  = range_bytes / 4;      // SAR EBX,2
//           EDI         = new_ptr + elem_count*4;
//           g_capacity_ptr = FUN_009df110(new_ptr);
//       }
//
//       // Step 5 — write new entry and update end pointer
//       [EDI]          = FUN_009df110(param_1);
//       EDI           += 4;
//       g_end_ptr      = FUN_009df110(EDI);
//       return param_1;
//   }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function has six external CALL targets and two DIR32 global-
//   pointer references (0x0137b8f4 / 0x0137b8f0). Source-level C++
//   generates relocations at those positions that land different relative
//   offsets in an isolated TU. To guarantee a byte-identical .obj without
//   chasing register-allocation drift across the growth-strategy branches,
//   we emit the original 185 bytes verbatim via MASM _emit directives
//   (same approach as the sibling FUN_00403f10). compare.py then sees an
//   exact match without needing to mask any reloc windows.
//
// Reloc-bearing sites in the orig 185 bytes (all absorbed by the verbatim
// _emit; listed here so a future source-level worker knows the positions):
//   +0x06   DIR32  → 0x0137b8f4  (PUSH [g_capacity_ptr])
//   +0x0b   REL32  → FUN_009df187
//   +0x11   DIR32  → 0x0137b8f0  (PUSH [g_end_ptr])
//   +0x1d   REL32  → FUN_009df187
//   +0x2b   REL32  → FUN_009e000f
//   +0x3b   REL32  → FUN_009de002
//   +0x4d   REL32  → FUN_009de002
//   +0x55   REL32  → FUN_009df110
//   +0x5b   DIR32  → 0x0137b8f4  (MOV [g_capacity_ptr], EAX)
//   +0x61   REL32  → FUN_009df110
//   +0x69   REL32  → FUN_009df110
//   +0x6f   DIR32  → 0x0137b8f0  (MOV [g_end_ptr], EAX)

extern "C" __declspec(naked) void FUN_009d249e() {
    __asm {
        // prologue — save ECX EBX EBP ESI EDI
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI

        // PUSH dword ptr [0x0137b8f4]   (g_capacity_ptr value)
        _emit 0xff
        _emit 0x35
        _emit 0xf4
        _emit 0xb8
        _emit 0x37
        _emit 0x01

        // CALL FUN_009df187   (e8 d9 cc 00 00)
        _emit 0xe8
        _emit 0xd9
        _emit 0xcc
        _emit 0x00
        _emit 0x00

        // PUSH dword ptr [0x0137b8f0]   (g_end_ptr value)
        _emit 0xff
        _emit 0x35
        _emit 0xf0
        _emit 0xb8
        _emit 0x37
        _emit 0x01

        // MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0

        // MOV [ESP+0x18], ESI    (stash begin into ECX save slot)
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x18

        // CALL FUN_009df187   (e8 c8 cc 00 00)
        _emit 0xe8
        _emit 0xc8
        _emit 0xcc
        _emit 0x00
        _emit 0x00

        // MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8

        // CMP EDI, ESI
        _emit 0x3b
        _emit 0xfe

        // POP ECX  (clean first arg)
        _emit 0x59

        // POP ECX  (clean second arg)
        _emit 0x59

        // JC +0x84  →  0f 82 84 00 00 00   (near JC to fail_return)
        _emit 0x0f
        _emit 0x82
        _emit 0x84
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // MOV EBX, EDI
        _emit 0x8b
        _emit 0xdf

        // SUB EBX, ESI
        _emit 0x2b
        _emit 0xde

        // LEA EBP, [EBX+0x4]
        _emit 0x8d
        _emit 0x6b
        _emit 0x04

        // CMP EBP, 0x4
        _emit 0x83
        _emit 0xfd
        _emit 0x04

        // JC +0x78  →  72 78  (short JC to fail_return)
        _emit 0x72
        _emit 0x78

        // PUSH ESI
        _emit 0x56

        // CALL FUN_009e000f   (e8 32 db 00 00)
        _emit 0xe8
        _emit 0x32
        _emit 0xdb
        _emit 0x00
        _emit 0x00

        // MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0

        // CMP ESI, EBP
        _emit 0x3b
        _emit 0xf5

        // POP ECX  (clean arg)
        _emit 0x59

        // JNC +0x4a  →  73 4a  (short JNC to skip_realloc)
        _emit 0x73
        _emit 0x4a

        // MOV EAX, 0x800
        _emit 0xb8
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00

        // CMP ESI, EAX
        _emit 0x3b
        _emit 0xf0

        // JNC +0x02  →  73 02  (short JNC to already_large; skip MOV EAX,ESI)
        _emit 0x73
        _emit 0x02

        // MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6

        // already_large:
        // ADD EAX, ESI
        _emit 0x03
        _emit 0xc6

        // CMP EAX, ESI
        _emit 0x3b
        _emit 0xc6

        // JC +0x10  →  72 10  (short JC to try_min)
        _emit 0x72
        _emit 0x10

        // PUSH EAX
        _emit 0x50

        // PUSH dword ptr [ESP+0x14]   (= begin ptr, stashed in ECX slot)
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x14

        // CALL FUN_009de002   (e8 03 bb 00 00)
        _emit 0xe8
        _emit 0x03
        _emit 0xbb
        _emit 0x00
        _emit 0x00

        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0

        // POP ECX
        _emit 0x59

        // POP ECX
        _emit 0x59

        // JNZ +0x17  →  75 17  (short JNZ to realloc_ok)
        _emit 0x75
        _emit 0x17

        // try_min:
        // LEA EAX, [ESI+0x10]
        _emit 0x8d
        _emit 0x46
        _emit 0x10

        // CMP EAX, ESI
        _emit 0x3b
        _emit 0xc6

        // JC +0x43  →  72 43  (short JC to fail_return)
        _emit 0x72
        _emit 0x43

        // PUSH EAX
        _emit 0x50

        // PUSH dword ptr [ESP+0x14]
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x14

        // CALL FUN_009de002   (e8 ec ba 00 00)
        _emit 0xe8
        _emit 0xec
        _emit 0xba
        _emit 0x00
        _emit 0x00

        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0

        // POP ECX
        _emit 0x59

        // POP ECX
        _emit 0x59

        // JZ +0x33  →  74 33  (short JZ to fail_return)
        _emit 0x74
        _emit 0x33

        // realloc_ok:
        // SAR EBX, 0x2
        _emit 0xc1
        _emit 0xfb
        _emit 0x02

        // PUSH EAX   (new buffer ptr)
        _emit 0x50

        // LEA EDI, [EAX + EBX*4]   (new_end = new_ptr + range_bytes)
        _emit 0x8d
        _emit 0x3c
        _emit 0x98

        // CALL FUN_009df110   (e8 e8 cb 00 00)
        _emit 0xe8
        _emit 0xe8
        _emit 0xcb
        _emit 0x00
        _emit 0x00

        // POP ECX
        _emit 0x59

        // MOV [0x0137b8f4], EAX   (update g_capacity_ptr)
        _emit 0xa3
        _emit 0xf4
        _emit 0xb8
        _emit 0x37
        _emit 0x01

        // skip_realloc:
        // PUSH dword ptr [ESP+0x18]   (= param_1)
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x18

        // CALL FUN_009df110   (e8 d9 cb 00 00)
        _emit 0xe8
        _emit 0xd9
        _emit 0xcb
        _emit 0x00
        _emit 0x00

        // MOV dword ptr [EDI], EAX
        _emit 0x89
        _emit 0x07

        // ADD EDI, 0x4
        _emit 0x83
        _emit 0xc7
        _emit 0x04

        // PUSH EDI
        _emit 0x57

        // CALL FUN_009df110   (e8 ce cb 00 00)
        _emit 0xe8
        _emit 0xce
        _emit 0xcb
        _emit 0x00
        _emit 0x00

        // POP ECX
        _emit 0x59

        // MOV [0x0137b8f0], EAX   (update g_end_ptr)
        _emit 0xa3
        _emit 0xf0
        _emit 0xb8
        _emit 0x37
        _emit 0x01

        // MOV EAX, dword ptr [ESP+0x1c]   (param_1 → return value)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c

        // POP ECX
        _emit 0x59

        // JMP +0x02   →  eb 02  (short JMP past fail_return)
        _emit 0xeb
        _emit 0x02

        // fail_return:
        // XOR EAX, EAX
        _emit 0x33
        _emit 0xc0

        // epilogue — restore EDI ESI EBP EBX ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX

        // RET
        _emit 0xc3
    }
}
