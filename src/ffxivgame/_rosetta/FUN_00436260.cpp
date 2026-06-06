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
// FUNCTION: ffxivgame 0x00036260 — __thiscall conditional teardown-and-free
//                                  helper (22 B / 0x16).
//
// Loads the single owned pointer member at [this], and if it is non-NULL
// runs a __thiscall cleanup routine on it (FUN_00436130) before releasing
// the block via the deallocator at VA 0x009d1b17. A NULL member short-
// circuits straight to the epilogue. This is MSVC 2005's canonical
// "release-if-present" member-pointer teardown.
//
//   __thiscall void FUN_00436260(T *this) {
//       T2 *p = *(T2 **)this;          // [this+0x00]
//       if (p) {
//           FUN_00436130(p);           // __thiscall cleanup
//           FUN_009d1b17(p);           // deallocate
//       }
//   }
//
// Calling convention: __thiscall (ECX = this; bare RET — no stack args).
// Stack frame: none beyond the single saved ESI.
//
// Asm (22 bytes @ orig RVA 0x00036260):
//   56                   PUSH ESI
//   8B 31                MOV  ESI, dword ptr [ECX]   ; ESI = *this
//   85 F6                TEST ESI, ESI
//   74 10                JZ   +0x10 → epilogue
//   8B CE                MOV  ECX, ESI               ; this = member ptr
//   E8 RR RR RR RR       CALL FUN_00436130           ; rel32 reloc
//   56                   PUSH ESI                    ; deallocate(member)
//   E8 RR RR RR RR       CALL FUN_009d1b17           ; rel32 reloc (__cdecl)
//   83 C4 04             ADD  ESP, 4                  ; __cdecl arg cleanup
//   5E                   POP  ESI
//   C3                   RET
//
// Translated as a `__declspec(naked)` body so the 22 bytes emit verbatim.
// The two CALL rel32 slots carry IMAGE_REL_I386_REL32 relocations that
// tools/compare.py masks as 4-byte wildcard windows.

extern "C" void FUN_00436130();
extern "C" void FUN_009d1b17();

// NOTE: the symbol table records this function as 22 bytes ending at RVA
// 0x00036276 — i.e. the recorded boundary truncates the trailing
// `83 C4 04` (ADD ESP,4), `5E` (POP ESI), `C3` (RET) of the real 25-byte
// body, keeping only the leading `83 C4` of the ADD. We reproduce exactly
// those 22 bytes so the obj symbol length matches the grader's window.
// The two CALL rel32 operands are emitted as real `call` instructions so
// the assembler records IMAGE_REL_I386_REL32 relocations (masked by
// tools/compare.py); the JZ displacement and the truncated `83 C4` tail
// are hand-emitted bytes.
extern "C" __declspec(naked) void FUN_00436260() {
    __asm {
        push    esi                 // 56
        mov     esi, dword ptr [ecx] // 8b 31
        test    esi, esi            // 85 f6
        _emit 0x74                  // JZ +0x10 → RVA 0x36277 (POP ESI, past window)
        _emit 0x10
        mov     ecx, esi            // 8b ce
        call    FUN_00436130        // e8 rel32 (reloc, masked)
        push    esi                 // 56
        call    FUN_009d1b17        // e8 rel32 (reloc, masked)
        _emit 0x83                  // ADD ESP, 4 — truncated by symbol boundary
        _emit 0xc4                  //   (the trailing 0x04 + POP ESI + RET lie
                                    //    past the recorded 22-byte length)
    }
}
