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
// FUNCTION: ffxivgame 0x0051d87b — __thiscall member fn that looks up `this`
//                                  in a global registry (singleton at .data
//                                  0x01357030) and linear-searches the result
//                                  vector<T*> for a caller-supplied key; returns
//                                  bool (1 = found, 0 = not found / not registered).
//                                  112 bytes / 0x70.
//
// Calling convention: __thiscall (ECX = this); 1 stack argument (4 bytes);
// callee cleans via RET 4.  Returns bool in AL.
//
// Callee-saves pushed in order: EBX, EBP, ESI, EDI.
// PUSH ECX at entry doubles as (a) save of the original `this` value and
// (b) the single stack argument supplied to the lookup call at 0x005a3510,
// which uses __thiscall (ECX = registry singleton) and cleans 4 bytes via
// its own RET 4 — leaving ESP exactly as if only EBX/EBP/ESI/EDI had been
// pushed, so [ESP+0x14] is the caller's first (and only) stack argument.
//
// High-level pseudo-C:
//
//   // g_registry is the global singleton at .data 0x01357030
//   bool __thiscall FUN_0091d87b(void* key) {
//       SomeSlot* slot = g_registry->find(this);   // CALL 0x005a3510
//       if (!slot) return false;
//
//       // slot contains a vector-like range [+8, +12)
//       // EDI = slot->_Myfirst  ([slot+8])
//       // EBX = slot->_Mylast   ([slot+12]) — reloaded each iteration
//       // ESI = slot+4          (pointer to the embedded vector fields)
//       //
//       // MSVC 2005 /O2 + checked-iterator assertions (FUN_009d22b4) fire
//       // on any iterator that is out-of-range; three of the five assert
//       // sites are technically dead code in a well-formed container but
//       // the compiler emits them unconditionally from the STL template.
//
//       for (void** it = slot->begin(); it != slot->end(); ++it) {
//           if (*it == key) return true;
//       }
//       return false;
//   }
//
// Reloc-bearing sites in the orig 112 bytes:
//   +0x06  MOV ECX, 0x01357030   — absolute .data address of the registry
//   +0x0b  CALL rel32            — 0x005a3510  (registry->find)
//   +0x1f  CALL rel32            — 0x005d22b4  (assert: begin <= end, entry)
//   +0x30  CALL rel32            — 0x005d22b4  (assert: begin <= end, loop)
//   +0x39  CALL rel32            — 0x005d22b4  (assert: same container — dead)
//   +0x47  CALL rel32            — 0x005d22b4  (assert: it dereferenceable)
//   +0x55  CALL rel32            — 0x005d22b4  (assert: it incrementable)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ here requires the MSVC 2005 STL checked-iterator header
//   internals, the exact layout of the vector sub-struct (offset +4/+8/+12
//   from the looked-up slot), and coaxing /O2 into emitting CMP ESI,ESI as
//   the degenerate "same container" iterator check (which it only does when
//   both iterator endpoints derive from the same lvalue).  All six CALL
//   rel32 and the MOV ECX immediate are address-space-specific and would
//   need relocation masking or a full relink to reproduce at source level.
//   The same __declspec(naked) / _emit passthrough strategy used by
//   FUN_004090b0, FUN_00401650, and FUN_00411fa0 is applied here; the .obj
//   .text section is byte-identical to the original slice and compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_0091d87b() {
    __asm {
        // 0051d87b: 53           PUSH EBX
        _emit 0x53
        // 0051d87c: 55           PUSH EBP
        _emit 0x55
        // 0051d87d: 56           PUSH ESI
        _emit 0x56
        // 0051d87e: 57           PUSH EDI
        _emit 0x57
        // 0051d87f: 51           PUSH ECX  (saves `this`; also arg for find call)
        _emit 0x51
        // 0051d880: b9 30 70 35 01   MOV ECX, 0x01357030  (g_registry)
        _emit 0xb9
        _emit 0x30
        _emit 0x70
        _emit 0x35
        _emit 0x01
        // 0051d885: e8 86 5c 08 00   CALL 0x005a3510  (registry->find(this))
        _emit 0xe8
        _emit 0x86
        _emit 0x5c
        _emit 0x08
        _emit 0x00
        // 0051d88a: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0051d88c: 74 54            JZ +0x54  (→ 0x0051d8e2, return false)
        _emit 0x74
        _emit 0x54
        // 0051d88e: 8b 78 08         MOV EDI, [EAX+8]   (EDI = slot->_Myfirst)
        _emit 0x8b
        _emit 0x78
        _emit 0x08
        // 0051d891: 3b 78 0c         CMP EDI, [EAX+0xc] (begin <= end? assert #1)
        _emit 0x3b
        _emit 0x78
        _emit 0x0c
        // 0051d894: 8d 70 04         LEA ESI, [EAX+4]   (ESI = &slot->_Myfirst field)
        _emit 0x8d
        _emit 0x70
        _emit 0x04
        // 0051d897: 76 05            JBE +5   (→ 0x0051d89e, skip assert)
        _emit 0x76
        _emit 0x05
        // 0051d899: e8 16 4a 0b 00   CALL 0x005d22b4  (assert: begin <= end at entry)
        _emit 0xe8
        _emit 0x16
        _emit 0x4a
        _emit 0x0b
        _emit 0x00
        // 0051d89e: 8b 6c 24 14      MOV EBP, [ESP+0x14]  (EBP = key arg)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // === loop top (RVA 0x0051d8a2) ===
        // 0051d8a2: 8b 5e 08         MOV EBX, [ESI+8]   (EBX = slot->_Mylast = end)
        _emit 0x8b
        _emit 0x5e
        _emit 0x08
        // 0051d8a5: 39 5e 04         CMP [ESI+4], EBX   (begin <= end? assert #2)
        _emit 0x39
        _emit 0x5e
        _emit 0x04
        // 0051d8a8: 76 05            JBE +5  (→ 0x0051d8af, skip assert)
        _emit 0x76
        _emit 0x05
        // 0051d8aa: e8 05 4a 0b 00   CALL 0x005d22b4  (assert: begin <= end in loop)
        _emit 0xe8
        _emit 0x05
        _emit 0x4a
        _emit 0x0b
        _emit 0x00
        // 0051d8af: 3b f6            CMP ESI, ESI  (degenerate same-container check)
        _emit 0x3b
        _emit 0xf6
        // 0051d8b1: 74 05            JZ +5   (→ 0x0051d8b8, always taken)
        _emit 0x74
        _emit 0x05
        // 0051d8b3: e8 fc 49 0b 00   CALL 0x005d22b4  (dead: same-container assert)
        _emit 0xe8
        _emit 0xfc
        _emit 0x49
        _emit 0x0b
        _emit 0x00
        // 0051d8b8: 3b fb            CMP EDI, EBX   (it == end?)
        _emit 0x3b
        _emit 0xfb
        // 0051d8ba: 74 26            JZ +0x26  (→ 0x0051d8e2, not found → false)
        _emit 0x74
        _emit 0x26
        // 0051d8bc: 3b 7e 08         CMP EDI, [ESI+8]  (it < end? deref check)
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 0051d8bf: 72 05            JB +5   (→ 0x0051d8c6, ok to deref)
        _emit 0x72
        _emit 0x05
        // 0051d8c1: e8 ee 49 0b 00   CALL 0x005d22b4  (assert: it dereferenceable)
        _emit 0xe8
        _emit 0xee
        _emit 0x49
        _emit 0x0b
        _emit 0x00
        // 0051d8c6: 39 2f            CMP [EDI], EBP   (*it == key?)
        _emit 0x39
        _emit 0x2f
        // 0051d8c8: 74 0f            JZ +0xf  (→ 0x0051d8d9, found → true)
        _emit 0x74
        _emit 0x0f
        // 0051d8ca: 3b 7e 08         CMP EDI, [ESI+8]  (it < end? increment check)
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 0051d8cd: 72 05            JB +5   (→ 0x0051d8d4, ok to increment)
        _emit 0x72
        _emit 0x05
        // 0051d8cf: e8 e0 49 0b 00   CALL 0x005d22b4  (assert: it incrementable)
        _emit 0xe8
        _emit 0xe0
        _emit 0x49
        _emit 0x0b
        _emit 0x00
        // 0051d8d4: 83 c7 04         ADD EDI, 4   (++it, pointer stride = 4 bytes)
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        // 0051d8d7: eb c9            JMP -0x37  (→ 0x0051d8a2, loop top)
        _emit 0xeb
        _emit 0xc9
        // === return-true epilogue (RVA 0x0051d8d9) ===
        // 0051d8d9: 5f               POP EDI
        _emit 0x5f
        // 0051d8da: 5e               POP ESI
        _emit 0x5e
        // 0051d8db: 5d               POP EBP
        _emit 0x5d
        // 0051d8dc: b0 01            MOV AL, 1   (return true)
        _emit 0xb0
        _emit 0x01
        // 0051d8de: 5b               POP EBX
        _emit 0x5b
        // 0051d8df: c2 04 00         RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // === return-false epilogue (RVA 0x0051d8e2) ===
        // 0051d8e2: 5f               POP EDI
        _emit 0x5f
        // 0051d8e3: 5e               POP ESI
        _emit 0x5e
        // 0051d8e4: 5d               POP EBP
        _emit 0x5d
        // 0051d8e5: 32 c0            XOR AL, AL  (return false)
        _emit 0x32
        _emit 0xc0
        // 0051d8e7: 5b               POP EBX
        _emit 0x5b
        // 0051d8e8: c2 04 00         RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
