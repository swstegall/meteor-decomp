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
// FUNCTION: ffxivgame 0x0043bc60 — __thiscall teardown method that
//                                   destroys an embedded CRITICAL_SECTION
//                                   and optionally notifies an owner object
//                                   before zeroing the buffer triple
//                                   (52 bytes / 0x34).
//
// Object layout (inferred from offsets accessed):
//   [this + 0x04]  void *  buf_ptr    — pointer to an associated buffer
//   [this + 0x08]  DWORD   buf_size   — (zeroed on teardown)
//   [this + 0x0c]  DWORD   buf_cap    — (zeroed on teardown)
//   [this + 0x1c]  CRITICAL_SECTION cs (28 B embedded)
//
// Source shape (inferred from asm):
//
//   void Foo::teardown() {
//       DeleteCriticalSection(&this->cs);           // [this+0x1c]
//       void *ptr = this->buf_ptr;                  // [this+0x04]
//       if (ptr) {
//           // ECX = *(ptr - 4)  (owner object "this")
//           // arg  = ptr
//           (*(Owner**)((char*)ptr - 4))->notify(ptr); // FUN_0040df70
//       }
//       this->buf_ptr  = nullptr;   // [this+0x04]
//       this->buf_size = 0;         // [this+0x08]
//       this->buf_cap  = 0;         // [this+0x0c]
//   }
//
// Calling convention: __thiscall (ECX = this), no stack args, void return.
// Epilogue: POP ESI / RET (no immediate — no caller-visible stack args).
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x07  CALL [imm32]  → [0x00F3E170]  (IAT: DeleteCriticalSection)
//   +0x13  CALL  rel32   → FUN_0040df70  (REL32)
//
// Reconstruction strategy — naked asm:
//   The function is simple enough that MASM mnemonics reproduce the
//   exact encoding without _emit byte stuffing.  The single IAT call
//   uses __declspec(dllimport) so the assembler emits the correct
//   ff 15 [DIR32] indirect-call form (same pattern as FUN_00414640).
//   FUN_0040df70 is declared extern "C" so the CALL emits a REL32.
//   Both reloc bytes are masked by compare.py during the byte diff.

extern "C" {

__declspec(dllimport) void __stdcall DeleteCriticalSection(void *lpCriticalSection);
void FUN_0040df70();

__declspec(naked) void FUN_0043bc60()
{
    __asm {
        // 0003bc60: 56
        push    esi
        // 0003bc61: 8b f1
        mov     esi, ecx
        // 0003bc63: 8d 46 1c
        lea     eax, [esi + 0x1c]
        // 0003bc66: 50
        push    eax
        // 0003bc67: ff 15 70 e1 f3 00
        call    dword ptr [DeleteCriticalSection]
        // 0003bc6d: 8b 46 04
        mov     eax, dword ptr [esi + 0x4]
        // 0003bc70: 85 c0
        test    eax, eax
        // 0003bc72: 74 09
        jz      done
        // 0003bc74: 8b 48 fc
        mov     ecx, dword ptr [eax - 0x4]
        // 0003bc77: 50
        push    eax
        // 0003bc78: e8 f3 22 fd ff
        call    FUN_0040df70
    done:
        // 0003bc7d: c7 46 04 00 00 00 00
        mov     dword ptr [esi + 0x4], 0
        // 0003bc84: c7 46 08 00 00 00 00
        mov     dword ptr [esi + 0x8], 0
        // 0003bc8b: c7 46 0c 00 00 00 00
        mov     dword ptr [esi + 0xc], 0
        // 0003bc92: 5e
        pop     esi
        // 0003bc93: c3
        ret
    }
}

}  // extern "C"
