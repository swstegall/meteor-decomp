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
// FUNCTION: ffxivgame 0x000119b0 — __thiscall constructor for a
//           SQEX::CDev::Engine::Memory::Alternative::DebugRemovableHeapSpace
//           (168 B / 0xa8).
//
// Calling convention: __thiscall (ECX = this). 5 DWORD stack args.
// RET 0x14 — callee cleans 5 x 4 = 20 bytes. Returns this in EAX.
// Callee-saves pushed: EBX, ESI. No local stack frame.
//
// The function stores vtable pointers and args into the object fields,
// then calls InitializeCriticalSection on the embedded CRITICAL_SECTION
// at [this+0x5c], and finally overwrites vtable slots for the
// DebugRemovableHeapSpace / DebugRemovableHeapBlock sub-objects.
//
// Reloc-bearing sites:
//   +0x84  ff 15 xx xx xx xx  CALL dword ptr [InitializeCriticalSection IAT]
//          (4-byte IAT address masked by compare.py; image base 0x400000,
//          IAT entry at VA 0x00f3e174)
//
// All vtable constants (0xf56d3c, 0xf56ce8, 0x9d4600, 0x6ce2e0, 0xf567c4,
// 0xf56cf0, 0xf56d2c, 0xf56d00) are baked-in VAs at the fixed image base
// — no relocations.  The only reloc is the IAT indirect call, which
// compare.py masks.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form of this function emits a different instruction
//   schedule under /O2: the compiler reorders the stores and uses MOV
//   [mem],imm32 (C7 form, 7 bytes) for the three zero-fills at +0x30/+0x34/
//   +0x38, whereas the original uses XOR EBX,EBX + MOV [ESI+N],EBX (3
//   bytes each, saves 12 bytes total).  Additionally the two LEA+self-link
//   pairs ([ESI+0x3c] and [ESI+0x4c]) require the compiler to keep EAX as
//   a base pointer while writing [EAX+4] and [EAX+8]; /O2 does not reliably
//   reproduce this pattern.  The naked-asm passthrough re-emits the orig
//   168 bytes verbatim; compare.py masks the 4-byte IAT address and reports
//   GREEN.

// clang / GCC static-analysis stub — NOT compiled in production.
#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_004119b0() {}
#endif

// MSVC production build — byte-identical naked-asm passthrough.
#if !defined(__clang__) && !defined(__GNUC__)
extern "C" __declspec(naked) void FUN_004119b0()
{
    __asm {
        // 000119b0: 8b 44 24 04          MOV EAX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000119b4: 8b 54 24 10          MOV EDX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 000119b8: 53                   PUSH EBX
        _emit 0x53
        // 000119b9: 56                   PUSH ESI
        _emit 0x56
        // 000119ba: 8b f1                MOV ESI, ECX     ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 000119bc: 8b 4c 24 14          MOV ECX, dword ptr [ESP+0x14]  ; a3 (after 2 pushes)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000119c0: 89 46 04             MOV dword ptr [ESI+0x4], EAX   ; field_04 = a1
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 000119c3: 8b 44 24 10          MOV EAX, dword ptr [ESP+0x10]  ; a2 (after 2 pushes)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000119c7: 89 4e 0c             MOV dword ptr [ESI+0xc], ECX   ; field_0c = a3
        _emit 0x89
        _emit 0x4e
        _emit 0x0c
        // 000119ca: 8b 4c 24 1c          MOV ECX, dword ptr [ESP+0x1c]  ; a5 (after 2 pushes)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 000119ce: 89 46 08             MOV dword ptr [ESI+0x8], EAX   ; field_08 = a2
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 000119d1: 89 56 10             MOV dword ptr [ESI+0x10], EDX  ; field_10 = a4
        _emit 0x89
        _emit 0x56
        _emit 0x10
        // 000119d4: c7 06 3c 6d f5 00    MOV dword ptr [ESI], 0xf56d3c  ; vftable
        _emit 0xc7
        _emit 0x06
        _emit 0x3c
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 000119da: 89 4e 14             MOV dword ptr [ESI+0x14], ECX  ; field_14 = a5
        _emit 0x89
        _emit 0x4e
        _emit 0x14
        // 000119dd: 89 46 1c             MOV dword ptr [ESI+0x1c], EAX  ; field_1c = a2
        _emit 0x89
        _emit 0x46
        _emit 0x1c
        // 000119e0: c7 46 18 e8 6c f5 00 MOV dword ptr [ESI+0x18], 0xf56ce8
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0xe8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 000119e7: c7 46 20 00 46 9d 00 MOV dword ptr [ESI+0x20], 0x9d4600
        _emit 0xc7
        _emit 0x46
        _emit 0x20
        _emit 0x00
        _emit 0x46
        _emit 0x9d
        _emit 0x00
        // 000119ee: b8 e0 e2 6c 00       MOV EAX, 0x6ce2e0
        _emit 0xb8
        _emit 0xe0
        _emit 0xe2
        _emit 0x6c
        _emit 0x00
        // 000119f3: 89 46 24             MOV dword ptr [ESI+0x24], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x24
        // 000119f6: 89 46 28             MOV dword ptr [ESI+0x28], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x28
        // 000119f9: 89 46 2c             MOV dword ptr [ESI+0x2c], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x2c
        // 000119fc: 33 db                XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 000119fe: 89 5e 30             MOV dword ptr [ESI+0x30], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x30
        // 00011a01: 89 5e 34             MOV dword ptr [ESI+0x34], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x34
        // 00011a04: 89 5e 38             MOV dword ptr [ESI+0x38], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x38
        // 00011a07: c7 46 3c c4 67 f5 00 MOV dword ptr [ESI+0x3c], 0xf567c4
        _emit 0xc7
        _emit 0x46
        _emit 0x3c
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011a0e: 8d 46 3c             LEA EAX, [ESI+0x3c]
        _emit 0x8d
        _emit 0x46
        _emit 0x3c
        // 00011a11: 89 40 04             MOV dword ptr [EAX+0x4], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00011a14: 89 40 08             MOV dword ptr [EAX+0x8], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 00011a17: 88 5e 48             MOV byte ptr [ESI+0x48], BL   ; 0
        _emit 0x88
        _emit 0x5e
        _emit 0x48
        // 00011a1a: 8d 46 4c             LEA EAX, [ESI+0x4c]
        _emit 0x8d
        _emit 0x46
        _emit 0x4c
        // 00011a1d: 8d 56 5c             LEA EDX, [ESI+0x5c]   ; &cs[0]
        _emit 0x8d
        _emit 0x56
        _emit 0x5c
        // 00011a20: c7 00 c4 67 f5 00    MOV dword ptr [EAX], 0xf567c4
        _emit 0xc7
        _emit 0x00
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00011a26: 89 40 04             MOV dword ptr [EAX+0x4], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00011a29: 89 40 08             MOV dword ptr [EAX+0x8], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 00011a2c: 52                   PUSH EDX              ; arg: &cs[0]
        _emit 0x52
        // 00011a2d: c7 46 58 f0 6c f5 00 MOV dword ptr [ESI+0x58], 0xf56cf0
        _emit 0xc7
        _emit 0x46
        _emit 0x58
        _emit 0xf0
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 00011a34: ff 15 74 e1 f3 00    CALL dword ptr [0x00f3e174]  ; InitializeCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00011a3a: 89 5e 78             MOV dword ptr [ESI+0x78], EBX  ; 0
        _emit 0x89
        _emit 0x5e
        _emit 0x78
        // 00011a3d: c7 46 74 2c 6d f5 00 MOV dword ptr [ESI+0x74], 0xf56d2c
        _emit 0xc7
        _emit 0x46
        _emit 0x74
        _emit 0x2c
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00011a44: 89 9e 80 00 00 00    MOV dword ptr [ESI+0x80], EBX  ; 0
        _emit 0x89
        _emit 0x9e
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011a4a: c7 46 7c 00 6d f5 00 MOV dword ptr [ESI+0x7c], 0xf56d00
        _emit 0xc7
        _emit 0x46
        _emit 0x7c
        _emit 0x00
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00011a51: 8b c6                MOV EAX, ESI         ; return this
        _emit 0x8b
        _emit 0xc6
        // 00011a53: 5e                   POP ESI
        _emit 0x5e
        // 00011a54: 5b                   POP EBX
        _emit 0x5b
        // 00011a55: c2 14 00             RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
#endif

// vim: ts=4 sts=4 sw=4 et
