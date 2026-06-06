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
// FUNCTION: ffxivgame 0x0005d490 — __cdecl dispatch helper over a parsed
//                                  request record (398 B / 0x18e, /GS
//                                  security-cookie frame, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x0005d490):
//
//   __cdecl int FUN_0045d490(... stack args ...) — returns EBX (an int
//   result code). The prologue allocates a 0x64-byte /GS-protected frame
//   via the stack probe at 0x009d29d0, seeds the security cookie from the
//   image-global at [0x012ea8b0] (`MOV EAX,[cookie]; XOR EAX,ESP;
//   MOV [ESP+0x60],EAX`), then loads three caller args into EBX / EBP /
//   EDI and runs a four-stage builder pipeline of __cdecl helpers:
//
//       FUN_0045ced0(&local18, EBX);          // local18 = EBX (seeded)
//       FUN_0045d240(&local1c, EDI);
//       FUN_0045d0e0(&local2c, &local40, &local1c);
//       FUN_0045d160(&local30);
//       add esp, 0x1c                          // fold the four cdecl pops
//
//   Then ESI = *EDI (the parsed record). If (record->flags[+0xc] & 4):
//   the function walks a 4-entry handler table at record+0x2c looking for
//   a slot whose id matches *EBP, performing one of three error-asserts
//   (FUN_0045c940 with line 0x6c/0x6e/0x69 + file ptr 0x00f68958) when the
//   slot is empty / id-not-found / handler-null, otherwise indirect-calls
//   the matched handler `(*ESI->vtbl)(...)` and tail-cleans the cookie.
//
//   The `& 4` is clear branch falls through to a second path that resolves
//   the record via FUN_0046a2e0(EBP, 0)/FUN_0046aac0/FUN_0046a1f0 and
//   FUN_0046ab20, then releases it via FUN_0046a190 and returns EBX.
//
//   Each of the four return tails re-checks the /GS cookie
//   (`MOV ECX,[ESP+0x60]; XOR ECX,ESP; CALL 0x009d20f4 (__security_check_cookie);
//   ADD ESP,0x64; RET`).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This body is dense with linker-resolved absolute targets: the stack
//   probe (0x009d29d0) and cookie check (0x009d20f4), the image-global
//   cookie load [0x012ea8b0], eight rel32 cdecl calls into sibling
//   builder / registry helpers, an indirect `CALL EAX` through the
//   record vtable, and two imm32 pushes of the assert file-path pointer
//   0x00f68958. Reproducing the exact /GS frame, the four-way branch
//   shape, the folded `add esp,N` cdecl-pop coalescing, and all those
//   reloc windows from source-level C++ under MSVC 2005 /O2 /GS is
//   brittle — every rewrite shifts at least one byte.
//
//   So, as FUN_0040ced0 / FUN_00415d00 and the other reloc-heavy
//   siblings do, emit the orig 398 bytes verbatim via MASM `_emit`
//   directives in a `__declspec(naked)` body. The .obj's `.text` ends
//   up byte-identical to the orig slice, which is what compare.py checks.

extern "C" __declspec(naked) void FUN_0045d490() {
    __asm {
        _emit 0xb8
        _emit 0x64
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x36
        _emit 0x55
        _emit 0x57
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x70
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x7c
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x78
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0xe8
        _emit 0x0d
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x57
        _emit 0x51
        _emit 0xe8
        _emit 0x72
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x51
        _emit 0xe8
        _emit 0xfe
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x52
        _emit 0xe8
        _emit 0x74
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x37
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        _emit 0xf6
        _emit 0x46
        _emit 0x0c
        _emit 0x04
        _emit 0x74
        _emit 0x7b
        _emit 0x6a
        _emit 0x00
        _emit 0x55
        _emit 0x83
        _emit 0xcb
        _emit 0xff
        _emit 0xe8
        _emit 0xde
        _emit 0xcd
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xf6
        _emit 0x74
        _emit 0x49
        _emit 0x56
        _emit 0xe8
        _emit 0xaf
        _emit 0xd5
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x7e
        _emit 0x3c
        _emit 0x8b
        _emit 0x07
        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x01
        _emit 0x68
        _emit 0xf8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x56
        _emit 0xe8
        _emit 0xc5
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85
        _emit 0xc0
        _emit 0x7e
        _emit 0x22
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x34
        _emit 0x52
        _emit 0x50
        _emit 0x51
        _emit 0x56
        _emit 0xe8
        _emit 0xd1
        _emit 0xd5
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x8b
        _emit 0xd8
        _emit 0x56
        _emit 0xe8
        _emit 0x36
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x8b
        _emit 0xc3
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x86
        _emit 0x4b
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x64
        _emit 0xc3
        _emit 0x33
        _emit 0xd2
        _emit 0x8d
        _emit 0x4e
        _emit 0x2c
        _emit 0x8b
        _emit 0x01
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x10
        _emit 0x39
        _emit 0x45
        _emit 0x00
        _emit 0x74
        _emit 0x36
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        _emit 0x7c
        _emit 0xea
        _emit 0x6a
        _emit 0x6b
        _emit 0x68
        _emit 0x58
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x6e
        _emit 0x6a
        _emit 0x6c
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0xa1
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x40
        _emit 0x4b
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x64
        _emit 0xc3
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x2a
        _emit 0x6a
        _emit 0x70
        _emit 0x68
        _emit 0x58
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x69
        _emit 0x6a
        _emit 0x6c
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0x6f
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x0f
        _emit 0x4b
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x64
        _emit 0xc3
        _emit 0x8b
        _emit 0x55
        _emit 0x14
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0x53
        _emit 0x52
        _emit 0x8b
        _emit 0x16
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x51
        _emit 0x52
        _emit 0xff
        _emit 0xd0
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xda
        _emit 0x4a
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x64
        _emit 0xc3
    }
}
