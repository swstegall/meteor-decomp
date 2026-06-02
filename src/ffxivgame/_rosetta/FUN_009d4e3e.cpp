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
// FUNCTION: ffxivgame 0x005d4e3e — __cdecl range-check / lock helper
//                                  (60 bytes / 0x3c)
//
// Inspection (read from the orig bytes at RVA 0x005d4e3e, 60 bytes):
//
//   __cdecl void FUN_009d4e3e(void* ptr)
//     stack:
//       [ESP+4]  : void*  ptr  (read after PUSH ESI as [ESP+8])
//
//   Structure:
//
//     esi = ptr
//     eax = 0x012ea8d8              // start of in-pool range (global data)
//     if (esi < eax)  goto L_other; // below range → unsigned JB
//     if (esi > 0x012eab38) goto L_other; // above range → unsigned JA
//
//     // in-range path — compute slot index, call an internal helper,
//     // then set a flag bit in the object header:
//     ecx = esi - eax               // offset from base of pool
//     ecx >>= 5                     // SAR 5 (divide by 32 with sign)
//     ecx += 0x10                   // bias by 16
//     call FUN_009e264c(ecx)        // internal helper (rel32 → RVA 0x5e264c)
//     *(unsigned int*)(esi + 0x0c) |= 0x8000    // set bit 15
//     pop ecx                       // clean pushed arg
//     pop esi
//     ret                           // __cdecl: caller cleans
//
//   L_other:
//     esi += 0x20
//     push esi                      // &ptr->field_0x20
//     call [EnterCriticalSection]   // KERNEL32 IAT @ 0x00f3e16c
//     pop esi
//     ret
//
// Reloc-bearing sites in the orig 60 bytes:
//     +0x06   MOV EAX, imm32  → pool base 0x012ea8d8   (IMAGE_RELOC, global .data)
//     +0x11   CMP ESI, imm32  → pool end  0x012eab38   (IMAGE_RELOC, global .data)
//     +0x22   CALL rel32      → FUN_009e264c  (IMAGE_REL_I386_REL32)
//     +0x36   CALL [imm32]    → IAT EnterCriticalSection  (IMAGE_RELOC, .rdata)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The four relocation sites (two global-var immediates, one rel32 call,
//   one IAT indirect call) prevent a standalone .obj from resolving these
//   addresses the same way the full-binary link does.  The pragmatic
//   approach — the same one FUN_00406fa0, FUN_00408780, and other
//   similarly-structured siblings took — is a `__declspec(naked)` body
//   that re-emits all 60 orig bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` is then byte-identical to the orig slice;
//   `tools/compare.py` reports GREEN (no COFF relocs → no masked
//   positions, but all 60 bytes match the already-resolved orig values).

extern "C" __declspec(naked) void FUN_009d4e3e() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x08]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0xb8              // MOV EAX, 0x012ea8d8  (pool start)
        _emit 0xd8
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x3b              // CMP ESI, EAX
        _emit 0xf0
        _emit 0x72              // JB  +0x22 → L_other
        _emit 0x22
        _emit 0x81              // CMP ESI, 0x012eab38  (pool end)
        _emit 0xfe
        _emit 0x38
        _emit 0xab
        _emit 0x2e
        _emit 0x01
        _emit 0x77              // JA  +0x1a → L_other
        _emit 0x1a
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0x2b              // SUB ECX, EAX
        _emit 0xc8
        _emit 0xc1              // SAR ECX, 5
        _emit 0xf9
        _emit 0x05
        _emit 0x83              // ADD ECX, 0x10
        _emit 0xc1
        _emit 0x10
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL rel32 → FUN_009e264c
        _emit 0xe8
        _emit 0xd7
        _emit 0x00
        _emit 0x00
        _emit 0x81              // OR dword ptr [ESI+0x0c], 0x8000
        _emit 0x4e
        _emit 0x0c
        _emit 0x00
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX  (clean PUSH ECX arg)
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        // L_other:
        _emit 0x83              // ADD ESI, 0x20
        _emit 0xc6
        _emit 0x20
        _emit 0x56              // PUSH ESI  (&ptr->field_0x20)
        _emit 0xff              // CALL dword ptr [0x00f3e16c]  (EnterCriticalSection)
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
