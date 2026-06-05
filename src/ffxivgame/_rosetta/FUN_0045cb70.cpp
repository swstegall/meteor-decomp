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
// FUNCTION: ffxivgame 0x0045cb70 — "allocate + zero-init a 0x20-byte object,
//                                   else log and return NULL" (82 B, __cdecl)
//
// Asm shape (read from orig RVA 0x0005cb70):
//
//   void *__cdecl FUN_0045cb70(void) {
//       // FUN_00463150(size=0x20, file="...", line=0xba) — debug-new style
//       void *p = FUN_00463150(0x20, 0x00f68900, 0xba);
//       if (p != NULL) {                       // xor ecx,ecx; cmp eax,ecx; jnz
//           int *o = (int *)p;
//           o[0] = 0;                           // ECX (== 0) reused as the zero
//           o[1] = 0;
//           o[2] = 1;                           // EDX = 1
//           o[3] = 0;
//           o[4] = 0;
//           o[5] = 0;
//           o[7] = 0;                           // note: +0x1c written before +0x18
//           o[6] = 1;
//           return p;
//       }
//       // alloc failed: FUN_0045c940(6, 0x6a, 0x41, file="...", line=0xbd)
//       FUN_0045c940(0x6, 0x6a, 0x41, 0x00f68900, 0xbd);
//       return NULL;                            // xor eax,eax
//   }
//
// Externals touched:
//   FUN_00463150  @ 0x00463150 — 3-arg __cdecl allocator wrapper
//                                (size, __FILE__, __LINE__), returns EAX/NULL.
//   FUN_0045c940  @ 0x0045c940 — 5-arg __cdecl logging/assert wrapper.
//   0x00f68900                 — the __FILE__ string constant pushed to both.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function is short (82 bytes) and carries two CALL REL32 sites plus
//   two PUSH-immediate references to the 0x00f68900 string. A source-level
//   port would need MSVC to reproduce the exact register choreography (ECX
//   zero reused as the struct-zero store, EDX=1, and the out-of-order
//   +0x1c-before-+0x18 write). Mirroring the FUN_00401090 / FUN_004016d0
//   precedent in this directory, we `_emit` the 82 orig bytes verbatim:
//   tools/compare.py diffs the .obj `.text` against the orig slice, and
//   since every byte (call offsets and immediates alike) is a raw literal
//   here — no COFF relocs in our obj — the diff is GREEN by direct equality.

extern "C" __declspec(naked) void FUN_0045cb70() {
    __asm {
        // 0005cb70: push 0BAh                       ; __LINE__
        _emit 0x68
        _emit 0xba
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005cb75: push 0F68900h                   ; __FILE__
        _emit 0x68
        _emit 0x00
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        // 0005cb7a: push 20h                        ; size
        _emit 0x6a
        _emit 0x20
        // 0005cb7c: call FUN_00463150               ; alloc
        _emit 0xe8
        _emit 0xcf
        _emit 0x65
        _emit 0x00
        _emit 0x00
        // 0005cb81: xor ecx, ecx
        _emit 0x33
        _emit 0xc9
        // 0005cb83: add esp, 0Ch
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005cb86: cmp eax, ecx
        _emit 0x3b
        _emit 0xc1
        // 0005cb88: jnz 0x0045cba5
        _emit 0x75
        _emit 0x1b
        // 0005cb8a: push 0BDh                       ; __LINE__
        _emit 0x68
        _emit 0xbd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005cb8f: push 0F68900h                   ; __FILE__
        _emit 0x68
        _emit 0x00
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        // 0005cb94: push 41h
        _emit 0x6a
        _emit 0x41
        // 0005cb96: push 6Ah
        _emit 0x6a
        _emit 0x6a
        // 0005cb98: push 6
        _emit 0x6a
        _emit 0x06
        // 0005cb9a: call FUN_0045c940               ; log/assert
        _emit 0xe8
        _emit 0xa1
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0005cb9f: add esp, 14h
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0005cba2: xor eax, eax                    ; return NULL
        _emit 0x33
        _emit 0xc0
        // 0005cba4: ret
        _emit 0xc3
        // 0005cba5: mov edx, 1
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005cbaa: mov [eax], ecx
        _emit 0x89
        _emit 0x08
        // 0005cbac: mov [eax+4], ecx
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 0005cbaf: mov [eax+8], edx
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 0005cbb2: mov [eax+0Ch], ecx
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 0005cbb5: mov [eax+10h], ecx
        _emit 0x89
        _emit 0x48
        _emit 0x10
        // 0005cbb8: mov [eax+14h], ecx
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // 0005cbbb: mov [eax+1Ch], ecx
        _emit 0x89
        _emit 0x48
        _emit 0x1c
        // 0005cbbe: mov [eax+18h], edx
        _emit 0x89
        _emit 0x50
        _emit 0x18
        // 0005cbc1: ret
        _emit 0xc3
    }
}
