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
// FUNCTION: ffxivgame 0x00419020 — __cdecl two-arg dispatch helper (0x44 / 68 bytes)
//
// Calling convention: __cdecl (two int arguments; RET with no stack cleanup).
// Module: _unknown
//
// __cdecl void FUN_00419020(int arg1, int arg2)
//
// Body (from asm at RVA 0x00019020):
//
//   Prologue loads arg2 into ESI (read as [esp+0xc] after 1 push) and
//   arg1 into EDI (read as [esp+0xc] after 2 pushes) — unusual interleaved
//   MSVC register assignment pattern.
//
//   // Call FUN_00419b60 (__thiscall) with:
//   //   this  = g_array_1328ff4[arg1]  (element at stride 64 / 0x40)
//   //   arg   = arg2                   (pushed on stack; callee cleans via RET 4)
//   ECX = (arg1 << 6) + 0x1328ff4;   // pointer to 64-byte slot in global array
//   push arg2;
//   call FUN_00419b60;
//
//   // Optional second dispatch through global registry at 0x01328db4.
//   EAX = g_registry_1328db4;         // pointer to registry struct (may be NULL)
//   if (EAX != NULL) {
//       ECX = EAX[arg1 + 2];          // 4-byte-stride table at EAX+8: [EAX + arg1*4 + 8]
//       EDX = EAX[1];                 // [EAX + 4]
//       EAX = [EDX + 8];              // one more pointer dereference
//       ECX = [EAX + ECX*8 - 4];     // 8-byte-stride table indexed by ECX, at offset -4
//       push 4;
//       push arg2;
//       push ECX;
//       ECX = g_obj_132987c;          // [0x0132987c] — this for the second call
//       call FUN_00422f90;            // __thiscall, 3 stack args
//   }
//
// Disassembly (verbatim, RVA 0x00019020..0x00019063, 68 bytes):
//
//   00019020: 56              PUSH ESI
//   00019021: 8b 74 24 0c     MOV ESI, [ESP+0xc]         ; after 1 push: ESI = arg2
//   00019025: 57              PUSH EDI
//   00019026: 8b 7c 24 0c     MOV EDI, [ESP+0xc]         ; after 2 pushes: EDI = arg1
//   0001902a: 8b cf           MOV ECX, EDI
//   0001902c: c1 e1 06        SHL ECX, 6
//   0001902f: 56              PUSH ESI                   ; arg2 for FUN_00419b60
//   00019030: 81 c1 f4 8f 32 01  ADD ECX, 0x1328ff4     ; ECX = &g_array[arg1]
//   00019036: e8 25 0b 00 00  CALL 0x00419b60            ; FUN_00419b60
//   0001903b: a1 b4 8d 32 01  MOV EAX, [0x01328db4]     ; g_registry
//   00019040: 85 c0           TEST EAX, EAX
//   00019042: 74 1d           JZ  +0x1d (→ 0x00419061)
//   00019044: 8b 4c b8 08     MOV ECX, [EAX + EDI*4 + 8]
//   00019048: 8b 50 04        MOV EDX, [EAX + 4]
//   0001904b: 8b 42 08        MOV EAX, [EDX + 8]
//   0001904e: 8b 4c c8 fc     MOV ECX, [EAX + ECX*8 - 4]
//   00019052: 6a 04           PUSH 4
//   00019054: 56              PUSH ESI
//   00019055: 51              PUSH ECX
//   00019056: 8b 0d 7c 98 32 01  MOV ECX, [0x0132987c]  ; g_obj
//   0001905c: e8 2f 9f 00 00  CALL 0x00422f90            ; FUN_00422f90
//   00019061: 5f              POP EDI
//   00019062: 5e              POP ESI
//   00019063: c3              RET
//
// Reconstruction strategy — inline __asm in a naked function:
//
//   Source-level C++ cannot reproduce the interleaved ESI/EDI load pattern
//   (two MOV instructions both at [esp+0xc] but at different stack depths)
//   or guarantee the exact immediate encodings for the global addresses.
//   A __declspec(naked) body with inline __asm reproduces all 68 bytes
//   faithfully. The two CALL rel32 targets are referenced by name so the
//   .obj carries DIR32 relocations; tools/compare.py masks those 4-byte
//   windows. The three absolute-address immediates (0x1328ff4, 0x01328db4,
//   0x0132987c) are bare literals in the instruction stream — no reloc
//   entries — and compare byte-for-byte against the orig binary.

extern "C" void FUN_00419b60();
extern "C" void FUN_00422f90();

extern "C" __declspec(naked) void FUN_00419020() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0xc]    // after 1 push: [esp+0xc] = arg2
        push    edi
        mov     edi, dword ptr [esp + 0xc]    // after 2 pushes: [esp+0xc] = arg1
        mov     ecx, edi
        shl     ecx, 6
        push    esi                            // arg2 as argument to FUN_00419b60
        add     ecx, 0x1328ff4                // ECX = &g_array[arg1] (stride 64)
        call    FUN_00419b60                  // __thiscall; cleans 1 arg via RET 4
        // MOV EAX, [0x01328db4] — must use A1 moffs32 encoding (not B8 imm32)
        // MASM inline asm misencodes [literal] as immediate; use _emit instead.
        _emit 0xa1  // A1 b4 8d 32 01  →  MOV EAX, [0x01328db4]
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        test    eax, eax
        jz      done
        mov     ecx, dword ptr [eax + edi*4 + 8]
        mov     edx, dword ptr [eax + 4]
        mov     eax, dword ptr [edx + 8]
        mov     ecx, dword ptr [eax + ecx*8 - 4]
        push    4
        push    esi
        push    ecx
        // MOV ECX, [0x0132987c] — must use 8B 0D disp32 encoding (not B9 imm32)
        // MASM inline asm misencodes [literal] as immediate; use _emit instead.
        _emit 0x8b  // 8B 0D 7c 98 32 01  →  MOV ECX, [0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        call    FUN_00422f90
    done:
        pop     edi
        pop     esi
        ret
    }
}
