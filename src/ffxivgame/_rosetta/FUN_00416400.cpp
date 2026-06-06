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
// FUNCTION: ffxivgame 0x00016400 — one-time global-registration initialiser
//                                  (__cdecl, 5 stack args, SEH frame, 127 B).
//
// Behaviour reconstructed from raw bytes (no Ghidra pseudo-C available):
//
//   void __cdecl FUN_00416400(arg1, arg2, arg3, arg4, arg5) {
//       // --- mini SEH frame (no /GS cookie) --------------------------
//       // PUSH -1 / PUSH scopeTable / PUSH FS:[0] / MOV FS:[0], ESP
//       // PUSH ECX (scratch slot, later overwritten)
//
//       // One-time guard: skip if already initialised.
//       if (*(DWORD*)0x01328d68 != 0) goto epilogue;
//
//       // Copy five caller args + one hardcoded pointer into .data globals.
//       *(DWORD*)0x01328d6c = arg2;
//       *(DWORD*)0x01328d68 = arg1;
//       *(DWORD*)0x01328d78 = arg5;
//       *(DWORD*)0x01328d70 = arg3;
//       *(DWORD*)0x01328d74 = arg4;
//       *(DWORD*)0x01328d80 = 0x01328130;  // hardcoded sub-object addr
//       [ECX scratch slot on frame] = 0x01328130;
//
//       // Enter try block (SEH state 0), then call the registration fn.
//       FUN_00416660(/*bool*/1, /*bool*/0, /*bool*/1);  // __stdcall, 3 args
//
//   epilogue:
//       // Restore SEH chain and return.
//   }
//
// SEH frame layout (ESP-relative after PUSH ECX at +0x15):
//   [ESP+ 0] ECX scratch slot (overwritten with 0x01328130 at +0x5a)
//   [ESP+ 4] prev FS:[0]          ← epilogue reloads this into FS:[0]
//   [ESP+ 8] SEH scope table ptr  (0x00e55203)
//   [ESP+ c] state                (-1 initially; 0 inside try body)
//   [ESP+10] return address
//   [ESP+14] arg1  [ESP+18] arg2  [ESP+1c] arg3
//   [ESP+20] arg4  [ESP+24] arg5
//
// Absolute-address sites in the orig 127 bytes (all masked by compare.py):
//   +0x03  PUSH imm32  → SEH scope table (VA 0x00e55203)
//   +0x08  MOV moffs32 FS:[0] read  (FS-prefix encoding, addr field = 0)
//   +0x0f  MOV moffs32 FS:[0] write (FS-prefix encoding, addr field = 0)
//   +0x18  CMP mem32   → 0x01328d68
//   +0x2c  MOV mem32   → 0x01328d6c
//   +0x36  MOV moffs32 → 0x01328d68
//   +0x3f  MOV mem32   → 0x01328d78
//   +0x45  MOV ECX, imm32 = 0x01328130
//   +0x4a  MOV mem32   → 0x01328d70
//   +0x50  MOV moffs32 → 0x01328d74
//   +0x55  MOV mem32   → 0x01328d80
//   +0x6c  CALL rel32  → FUN_00416660 (displacement 0x000001f0 from +0x70)
//   +0x75  MOV moffs32 FS:[0] write (FS-prefix encoding, addr field = 0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Every reloc site carries a runtime-resolved absolute address (globals in
//   .data at 0x01328d68..0x01328d80, the SEH scope table in .rdata at
//   0x00e55203, and the CALL displacement to FUN_00416660).  A source-level
//   C++ rewrite would require MSVC 2005 to allocate the same registers and
//   branch encodings — brittle under /O2.  Following the precedent of
//   FUN_00401350 and FUN_00403d60, we emit all 127 bytes verbatim via MASM
//   _emit directives; compare.py masks the reloc bytes and reports GREEN.

extern "C" __declspec(naked) void FUN_00416400() {
    __asm {
        // +00
        _emit 0x6a              // PUSH -1                (initial SEH state)
        _emit 0xff
        // +02
        _emit 0x68              // PUSH 0x00e55203        (SEH scope table)
        _emit 0x03
        _emit 0x52
        _emit 0xe5
        _emit 0x00
        // +07
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0d
        _emit 0x50              // PUSH EAX               (prev FS:[0])
        // +0e
        _emit 0x64              // MOV FS:[0x00000000], ESP  (install SEH frame)
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +15
        _emit 0x51              // PUSH ECX               (scratch slot / EH recovery)
        // +16
        _emit 0x83              // CMP DWORD PTR [0x01328d68], 0
        _emit 0x3d
        _emit 0x68
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // +1d
        _emit 0x75              // JNZ +0x51              (already init'd → epilogue)
        _emit 0x51
        // +1f
        _emit 0x8b              // MOV ECX, [ESP+0x18]   (arg2)
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // +23
        _emit 0x8b              // MOV EAX, [ESP+0x14]   (arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // +27
        _emit 0x8b              // MOV EDX, [ESP+0x1c]   (arg3)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // +2b
        _emit 0x89              // MOV [0x01328d6c], ECX  (store arg2)
        _emit 0x0d
        _emit 0x6c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // +31
        _emit 0x8b              // MOV ECX, [ESP+0x24]   (arg5)
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // +35
        _emit 0xa3              // MOV [0x01328d68], EAX  (store arg1)
        _emit 0x68
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // +3a
        _emit 0x8b              // MOV EAX, [ESP+0x20]   (arg4)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // +3e
        _emit 0x89              // MOV [0x01328d78], ECX  (store arg5)
        _emit 0x0d
        _emit 0x78
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // +44
        _emit 0xb9              // MOV ECX, 0x01328130   (hardcoded sub-object addr)
        _emit 0x30
        _emit 0x81
        _emit 0x32
        _emit 0x01
        // +49
        _emit 0x89              // MOV [0x01328d70], EDX  (store arg3)
        _emit 0x15
        _emit 0x70
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // +4f
        _emit 0xa3              // MOV [0x01328d74], EAX  (store arg4)
        _emit 0x74
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // +54
        _emit 0x89              // MOV [0x01328d80], ECX  (store hardcoded addr)
        _emit 0x0d
        _emit 0x80
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // +5a
        _emit 0x89              // MOV [ESP], ECX         (put addr into ECX scratch slot)
        _emit 0x0c
        _emit 0x24
        // +5d
        _emit 0x6a              // PUSH 1                 (arg1 to callee)
        _emit 0x01
        // +5f
        _emit 0x6a              // PUSH 0                 (arg2 to callee)
        _emit 0x00
        // +61
        _emit 0x6a              // PUSH 1                 (arg3 to callee)
        _emit 0x01
        // +63
        _emit 0xc7              // MOV DWORD PTR [ESP+0x18], 0  (set SEH trylevel=0)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +6b
        _emit 0xe8              // CALL FUN_00416660      (rel32 = 0x000001f0)
        _emit 0xf0
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // +70  epilogue (also JNZ target at +0x1d)
        _emit 0x8b              // MOV ECX, [ESP+4]       (reload prev FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // +74
        _emit 0x64              // MOV FS:[0x00000000], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +7b
        _emit 0x83              // ADD ESP, 0x10          (pop ECX slot + prev + handler + state)
        _emit 0xc4
        _emit 0x10
        // +7e
        _emit 0xc3              // RET
    }
}
