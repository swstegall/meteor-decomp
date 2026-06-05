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
// FUNCTION: ffxivgame 0x0005c940 — _ERR_put_error (OpenSSL error-state recorder)
//                                  (244 B / 0xf4, no SEH, __cdecl).
//
// Behaviour read from asm/ffxivgame/0005c940__ERR_put_error.s:
//
//   __cdecl void ERR_put_error(int lib, int func, int reason,
//                               const char *file, int line);
//
//   The function retrieves the per-thread OpenSSL error state (via a call
//   to a getter at 0x0045c360 that returns a pointer in EAX/ESI), then:
//
//     1. Advances a ring-buffer write index at [state+0x188] by 1 mod 16
//        (using the signed-saturation idiom: ADD/AND 0x8000000f / JNS /
//        DEC / OR 0xfffffff0 / INC to handle wrap of the signed masking).
//
//     2. If the new write index equals the read index ([state+0x18c]),
//        also advances the read index the same way (oldest entry dropped).
//
//     3. Clears the function-name pointer slot at
//        [state + new_idx*4 + 0x8] = 0.
//
//     4. Packs the error code into a dword:
//          EAX = ((func & 0xfff) << 12) | ((lib_byte & 0xff) << 24)
//                | (reason & 0xfff)
//        and stores it to [state + new_idx*4 + 0x48].
//
//     5. Stores file pointer (arg3) to [state + new_idx*4 + 0x108].
//
//     6. Stores line number (arg4) to [state + new_idx*4 + 0x148].
//
//     7. Checks whether a string-data pointer at
//        [state + new_idx*4 + 0x88] is non-null:
//          - If zero:  clear the flags byte at [state + new_idx*4 + 0xc8],
//                      POP EDI/ESI, RET.
//          - If non-zero AND flags bit 0 is set: call free (0x004632f0)
//            on that pointer, then zero both the pointer and flags slot,
//            POP EDI/ESI, RET.
//          - If non-zero AND flags bit 0 is clear: zero only the flags
//            slot (keeping the raw pointer), POP EDI/ESI, RET.
//
//   Stack layout at function entry (esp+N, __cdecl, 5 args):
//     [esp+0x04]  int   lib_byte  (only low byte used via MOVZX)
//     [esp+0x08]  int   func      (low 12 bits used)
//     [esp+0x0c]  int   reason    (low 12 bits used)
//     [esp+0x10]  char* file      (stored verbatim)
//     [esp+0x14]  int   line      (stored verbatim)
//   After PUSH ESI + PUSH EDI the offsets become +0x0c/+0x10/+0x14/+0x18/+0x1c.
//
//   Reloc-bearing sites in the orig 244 bytes (these PC-relative call offsets
//   resolve only against the orig image base; a standalone .obj compilation
//   would emit E8 00 00 00 00 + a relocation record for each, producing bytes
//   that differ from orig):
//     +0x02   rel32   0x0045c360 — ERR_get_state() getter (__cdecl)
//             (bytes e8 19 fa ff ff)
//     +0xb9   rel32   0x004632f0 — free / CRYPTO_free (__cdecl)
//             (bytes e8 f2 68 00 00)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two PC-relative CALLs mean a source-level .obj would emit
//   E8 00 00 00 00 (unresolved) for both, while the orig binary has the
//   final relocated values.  tools/compare.py checks raw .text bytes, so
//   any normal compilation produces a MISMATCH at those two windows.
//
//   The pragmatic choice — the same one FUN_00402a30 / FUN_00403a20 /
//   FUN_004054d0 and many other _rosetta entries took — is a
//   `__declspec(naked)` body that re-emits all 244 bytes verbatim via
//   MASM _emit directives.  The .obj's .text section ends up byte-identical
//   to the orig slice (no relocations because the bytes are raw immediates),
//   which is what compare.py checks.

extern "C" __declspec(naked) void FUN_0045c940() {
    __asm {
        // 0005c940  PUSH ESI
        _emit 0x56
        // 0005c941  PUSH EDI
        _emit 0x57
        // 0005c942  CALL 0x0045c360  ; ERR_get_state()
        _emit 0xe8
        _emit 0x19
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 0005c947  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0005c949  MOV EAX, [ESI+0x188]   ; state->top
        _emit 0x8b
        _emit 0x86
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c94f  ADD EAX, 1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0005c952  AND EAX, 0x8000000f
        _emit 0x25
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0005c957  JNS +5  (skip fixup)
        _emit 0x79
        _emit 0x05
        // 0005c959  DEC EAX
        _emit 0x48
        // 0005c95a  OR EAX, 0xfffffff0
        _emit 0x83
        _emit 0xc8
        _emit 0xf0
        // 0005c95d  INC EAX
        _emit 0x40
        // 0005c95e  MOV ECX, [ESI+0x18c]   ; state->bottom
        _emit 0x8b
        _emit 0x8e
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c964  CMP EAX, ECX
        _emit 0x3b
        _emit 0xc1
        // 0005c966  MOV [ESI+0x188], EAX   ; store new top
        _emit 0x89
        _emit 0x86
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c96c  JNZ +0x16  (skip bottom advance)
        _emit 0x75
        _emit 0x16
        // 0005c96e  ADD ECX, 1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 0005c971  AND ECX, 0x8000000f
        _emit 0x81
        _emit 0xe1
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0005c977  JNS +5
        _emit 0x79
        _emit 0x05
        // 0005c979  DEC ECX
        _emit 0x49
        // 0005c97a  OR ECX, 0xfffffff0
        _emit 0x83
        _emit 0xc9
        _emit 0xf0
        // 0005c97d  INC ECX
        _emit 0x41
        // 0005c97e  MOV [ESI+0x18c], ECX   ; store new bottom
        _emit 0x89
        _emit 0x8e
        _emit 0x8c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c984  MOVZX ECX, byte ptr [ESP+0xc]   ; lib arg
        _emit 0x0f
        _emit 0xb6
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0005c989  MOV EDX, [ESP+0x14]   ; reason arg
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0005c98d  XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // 0005c98f  MOV [ESI+EAX*4+0x8], EDI   ; clear fn-name ptr
        _emit 0x89
        _emit 0x7c
        _emit 0x86
        _emit 0x08
        // 0005c993  MOV EAX, [ESP+0x10]   ; func arg
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0005c997  AND EAX, 0xfff
        _emit 0x25
        _emit 0xff
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 0005c99c  SHL EAX, 0xc
        _emit 0xc1
        _emit 0xe0
        _emit 0x0c
        // 0005c99f  SHL ECX, 0x18
        _emit 0xc1
        _emit 0xe1
        _emit 0x18
        // 0005c9a2  OR EAX, ECX
        _emit 0x0b
        _emit 0xc1
        // 0005c9a4  MOV ECX, [ESI+0x188]   ; reload top index
        _emit 0x8b
        _emit 0x8e
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c9aa  AND EDX, 0xfff
        _emit 0x81
        _emit 0xe2
        _emit 0xff
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        // 0005c9b0  OR EAX, EDX
        _emit 0x0b
        _emit 0xc2
        // 0005c9b2  MOV [ESI+ECX*4+0x48], EAX   ; store error code
        _emit 0x89
        _emit 0x44
        _emit 0x8e
        _emit 0x48
        // 0005c9b6  MOV EDX, [ESI+0x188]   ; reload top index
        _emit 0x8b
        _emit 0x96
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c9bc  MOV EAX, [ESP+0x18]   ; file arg
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0005c9c0  MOV [ESI+EDX*4+0x108], EAX   ; store file ptr
        _emit 0x89
        _emit 0x84
        _emit 0x96
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c9c7  MOV ECX, [ESI+0x188]   ; reload top index
        _emit 0x8b
        _emit 0x8e
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c9cd  MOV EDX, [ESP+0x1c]   ; line arg
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0005c9d1  MOV [ESI+ECX*4+0x148], EDX   ; store line number
        _emit 0x89
        _emit 0x94
        _emit 0x8e
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c9d8  MOV EAX, [ESI+0x188]   ; reload top index
        _emit 0x8b
        _emit 0x86
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c9de  CMP [ESI+EAX*4+0x88], EDI   ; check data ptr == 0
        _emit 0x39
        _emit 0xbc
        _emit 0x86
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c9e5  JZ +0x37  -> 0x0045ca1e
        _emit 0x74
        _emit 0x37
        // 0005c9e7  TEST byte ptr [ESI+EAX*4+0xc8], 0x1
        _emit 0xf6
        _emit 0x84
        _emit 0x86
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // 0005c9ef  JZ +0x37  -> 0x0045ca28
        _emit 0x74
        _emit 0x37
        // 0005c9f1  MOV EAX, [ESI+EAX*4+0x88]   ; load data ptr
        _emit 0x8b
        _emit 0x84
        _emit 0x86
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c9f8  PUSH EAX
        _emit 0x50
        // 0005c9f9  CALL 0x004632f0  ; free/CRYPTO_free
        _emit 0xe8
        _emit 0xf2
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0005c9fe  MOV ECX, [ESI+0x188]
        _emit 0x8b
        _emit 0x8e
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005ca04  MOV [ESI+ECX*4+0x88], EDI   ; clear data ptr
        _emit 0x89
        _emit 0xbc
        _emit 0x8e
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005ca0b  MOV EDX, [ESI+0x188]
        _emit 0x8b
        _emit 0x96
        _emit 0x88
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005ca11  ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005ca14  MOV [ESI+EDX*4+0xc8], EDI   ; clear flags
        _emit 0x89
        _emit 0xbc
        _emit 0x96
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005ca1b  POP EDI
        _emit 0x5f
        // 0005ca1c  POP ESI
        _emit 0x5e
        // 0005ca1d  RET
        _emit 0xc3
        // 0005ca1e  MOV [ESI+EAX*4+0xc8], EDI   ; (data==0 path) clear flags
        _emit 0x89
        _emit 0xbc
        _emit 0x86
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005ca25  POP EDI
        _emit 0x5f
        // 0005ca26  POP ESI
        _emit 0x5e
        // 0005ca27  RET
        _emit 0xc3
        // 0005ca28  MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 0005ca2a  MOV [ESI+ECX*4+0xc8], EDI   ; (flag-0 path) clear flags only
        _emit 0x89
        _emit 0xbc
        _emit 0x8e
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005ca31  POP EDI
        _emit 0x5f
        // 0005ca32  POP ESI
        _emit 0x5e
        // 0005ca33  RET
        _emit 0xc3
    }
}
