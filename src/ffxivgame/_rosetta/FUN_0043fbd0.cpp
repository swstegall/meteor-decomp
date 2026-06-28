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
// FUNCTION: ffxivgame 0x0043fbd0 — __thiscall buffer-teardown helper
//                                   that optionally calls a pre-free hook
//                                   then notifies the buffer owner before
//                                   zeroing the triple (70 bytes / 0x46).
//
// Object layout (inferred from offsets accessed):
//   [this + 0x04]  void *  buf_ptr   — pointer to an associated buffer
//   [this + 0x08]  DWORD   buf_size  — (zeroed on teardown)
//   [this + 0x0c]  DWORD   buf_cap   — (zeroed on teardown)
//
// Source shape (inferred from asm):
//
//   void Foo::reset() {
//       void *ptr = this->buf_ptr;         // [this+0x04]
//       if (ptr) {
//           FUN_0043f830(ptr, this->buf_size, this, this);
//           ptr = this->buf_ptr;           // reload after hook
//           if (ptr) {
//               // ECX = *(ptr - 4)  (owner object handle)
//               // arg  = ptr
//               FUN_0040df70(ptr);         // notify / release
//           }
//       }
//       this->buf_ptr  = nullptr;   // [this+0x04]
//       this->buf_size = 0;         // [this+0x08]
//       this->buf_cap  = 0;         // [this+0x0c]
//   }
//
// Calling convention: __thiscall (ECX = this), no stack args, void return.
// Prologue: PUSH ECX (saves this as local at [ESP+4] after both pushes) /
//           PUSH ESI / MOV ESI, ECX.
// Epilogue: POP ESI / POP ECX / RET (no immediate — no stack args).
//
// The PUSH ECX at the top is a compiler-generated local slot: MSVC 2005
// saves ECX onto the stack so it can be reloaded into ECX just before the
// four-argument push sequence for FUN_0043f830 — at that point ECX is the
// only scratch register available, so the compiler spills-then-reloads
// rather than simply using ESI (which it already uses for arg3).
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x17  CALL  rel32   → FUN_0043f830  (REL32)
//   +0x2a  CALL  rel32   → FUN_0040df70  (REL32)

extern "C" {

void FUN_0043f830();
void FUN_0040df70();

__declspec(naked) void FUN_0043fbd0()
{
    __asm {
        // 0003fbd0: 51
        push    ecx
        // 0003fbd1: 56
        push    esi
        // 0003fbd2: 8b f1
        mov     esi, ecx
        // 0003fbd4: 8b 46 04
        mov     eax, dword ptr [esi + 0x4]
        // 0003fbd7: 85 c0
        test    eax, eax
        // 0003fbd9: 74 23
        jz      done
        // 0003fbdb: 8b 4c 24 04
        mov     ecx, dword ptr [esp + 0x4]
        // 0003fbdf: 8b 56 08
        mov     edx, dword ptr [esi + 0x8]
        // 0003fbe2: 51
        push    ecx
        // 0003fbe3: 56
        push    esi
        // 0003fbe4: 52
        push    edx
        // 0003fbe5: 50
        push    eax
        // 0003fbe6: e8 45 fc ff ff
        call    FUN_0043f830
        // 0003fbeb: 8b 46 04
        mov     eax, dword ptr [esi + 0x4]
        // 0003fbee: 83 c4 10
        add     esp, 0x10
        // 0003fbf1: 85 c0
        test    eax, eax
        // 0003fbf3: 74 09
        jz      done
        // 0003fbf5: 8b 48 fc
        mov     ecx, dword ptr [eax - 0x4]
        // 0003fbf8: 50
        push    eax
        // 0003fbf9: e8 72 e3 fc ff
        call    FUN_0040df70
    done:
        // 0003fbfe: c7 46 04 00 00 00 00
        mov     dword ptr [esi + 0x4], 0
        // 0003fc05: c7 46 08 00 00 00 00
        mov     dword ptr [esi + 0x8], 0
        // 0003fc0c: c7 46 0c 00 00 00 00
        mov     dword ptr [esi + 0xc], 0
        // 0003fc13: 5e
        pop     esi
        // 0003fc14: 59
        pop     ecx
        // 0003fc15: c3
        ret
    }
}

}  // extern "C"
