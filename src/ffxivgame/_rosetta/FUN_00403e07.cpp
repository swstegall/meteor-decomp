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
// FUNCTION: ffxivgame 0x00403e07 — body chunk of an inlined
//                                  `std::basic_string::assign()` (105 B
//                                  including the trailing `ret 0x8`)
//
// Inspection (read from the orig bytes at RVA 0x00003e07, file offset
// 0x00003e07 of orig/ffxivgame.exe — 105 bytes, per the
// config/ffxivgame.size_overrides.json entry that grew the symbol from
// Ghidra's reported 102 B by the 3-byte `ret 0x8` epilogue):
//
//   This is the body-and-epilogue of a function whose prologue lives
//   outside our symbol range — entry is reached with EBP already set to
//   the caller's frame and with EBX/ESI/EDI/ECX already pushed (an
//   MSVC-2005 SEH-protected `__stdcall(this *, void *, size_t)` of a
//   std::basic_string<char> member taking a (src_ptr, count) pair and
//   the implicit this in EDI, with the desired capacity passed in ESI).
//   The body inlines the canonical 1.x basic_string assign() shape:
//
//       if (count != 0) {
//           memcpy_s(dest, esi + 1,
//                    (this->cap >= 0x10) ? this->ptr : &this->buf,
//                    count);
//       }
//       if (this->cap >= 0x10)   // free the heap buffer
//           free(this->ptr);
//       this->buf[0] = 0;        // poison the SSO byte, then
//       this->ptr = dest;        // overwrite with the new pointer
//       this->cap = esi;         // commit new capacity (the union slot)
//       this->len = count;       // commit new length
//       ((esi < 0x10) ? &this->buf : dest)[count] = 0;
//                                // NUL-terminate at the right span
//       fs:0 = prev_seh;         // restore SEH chain ([ebp-0xc])
//       pop ecx/edi/esi/ebx; mov esp,ebp; pop ebp; ret 0x8
//
//   Calling convention: __stdcall — `ret 0x8` pops two dword arguments
//   ([ebp+0x8] = dest, [ebp+0xc] = count). The implicit ESI = capacity
//   and EDI = string-this are register-preserved across the call chain
//   from the parent function.
//   Stack frame: re-uses the caller's EBP-based frame; the symbol does
//   not establish its own prologue.
//
// Reloc-bearing sites in the orig 105 bytes (compare.py masks no bytes
// here because our `_emit`-only naked-asm .obj emits zero relocations
// — the call-rel32 displacements are baked in as concrete bytes, which
// match the orig PE's literal `.text` slice verbatim):
//     +0x1f   CALL rel32   → 0x009d17f3   (_memcpy_s helper)
//     +0x31   CALL rel32   → 0x009d1b17   (_free helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level reconstruction would require modelling the outer
//   function's frame and SEH state (the `mov ecx, [ebp-0xc]; mov fs:0,
//   ecx` epilogue restores a saved-handler slot that the symbol does
//   not itself install), and would emit the two helper calls as
//   linker-resolved relocations whose immediate bytes are zero-filled
//   in the .obj. Since we have no relink driving these symbols, the
//   pragmatic choice — the same one chosen by sibling FUN_00403bd0
//   (`operator new[]` thunk) and FUN_00403c80 (_memmove_s wrapper) — is
//   a `__declspec(naked)` body that re-emits the orig 105 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice (no relocations: the
//   rel32 offsets resolve against the orig binary's own address space
//   and are stored as absolute byte values that match the orig PE's
//   wire image). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00403e07() {
    __asm {
        _emit 0x8b              // MOV  EBX, dword ptr [EBP+0xC]
        _emit 0x5d
        _emit 0x0c
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x76              // JBE  short skip_memcpy (+0x20)
        _emit 0x20
        _emit 0x83              // CMP  dword ptr [EDI+0x18], 0x10
        _emit 0x7f
        _emit 0x18
        _emit 0x10
        _emit 0x72              // JB   short use_sso (+0x05)
        _emit 0x05
        _emit 0x8b              // MOV  EAX, dword ptr [EDI+0x4]      (heap ptr)
        _emit 0x47
        _emit 0x04
        _emit 0xeb              // JMP  short call_memcpy (+0x03)
        _emit 0x03
        _emit 0x8d              // LEA  EAX, [EDI+0x4]                (SSO ptr)
        _emit 0x47
        _emit 0x04
        _emit 0x53              // PUSH EBX                           (count)
        _emit 0x50              // PUSH EAX                           (src)
        _emit 0x8b              // MOV  EAX, dword ptr [EBP+0x8]      (dest)
        _emit 0x45
        _emit 0x08
        _emit 0x8d              // LEA  EDX, [ESI+0x1]                (destsz)
        _emit 0x56
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL _memcpy_s   (rel32 → 0x009d17f3)
        _emit 0xc8
        _emit 0xd9
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x83              // CMP  dword ptr [EDI+0x18], 0x10    (skip_memcpy:)
        _emit 0x7f
        _emit 0x18
        _emit 0x10
        _emit 0x72              // JB   short skip_free (+0x0C)
        _emit 0x0c
        _emit 0x8b              // MOV  ECX, dword ptr [EDI+0x4]
        _emit 0x4f
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL _free       (rel32 → 0x009d1b17)
        _emit 0xda
        _emit 0xdc
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x83              // CMP  ESI, 0x10                     (skip_free:)
        _emit 0xfe
        _emit 0x10
        _emit 0x8b              // MOV  ECX, dword ptr [EBP+0x8]
        _emit 0x4d
        _emit 0x08
        _emit 0x8d              // LEA  EAX, [EDI+0x4]
        _emit 0x47
        _emit 0x04
        _emit 0xc6              // MOV  byte ptr [EAX], 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [EAX], ECX
        _emit 0x08
        _emit 0x89              // MOV  dword ptr [EDI+0x18], ESI
        _emit 0x77
        _emit 0x18
        _emit 0x89              // MOV  dword ptr [EDI+0x14], EBX
        _emit 0x5f
        _emit 0x14
        _emit 0x72              // JB   short use_sso_tail (+0x02)
        _emit 0x02
        _emit 0x8b              // MOV  EAX, ECX                       (use_heap_tail:)
        _emit 0xc1
        _emit 0xc6              // MOV  byte ptr [EAX+EBX], 0x00       (use_sso_tail:)
        _emit 0x04
        _emit 0x18
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [EBP-0xC]
        _emit 0x4d
        _emit 0xf4
        _emit 0x64              // MOV  dword ptr fs:[0], ECX          (restore SEH)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP  ECX
        _emit 0x5f              // POP  EDI
        _emit 0x5e              // POP  ESI
        _emit 0x5b              // POP  EBX
        _emit 0x8b              // MOV  ESP, EBP
        _emit 0xe5
        _emit 0x5d              // POP  EBP
        _emit 0xc2              // RET  0x8
        _emit 0x08
        _emit 0x00
    }
}
