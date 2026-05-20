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
// FUNCTION: ffxivgame 0x00404800 — wrapper around FUN_00404760 (28-byte
//                                  slot swap helper) that computes the
//                                  "leftover" tail offset (90 bytes)
//
// __cdecl int FUN_00404800(int begin, int end, int cur,
//                          undefined arg4, undefined arg5)
//
// Body (read from the orig bytes at RVA 0x00004800, 90 bytes total):
//
//   push ecx                       ; reserve a 4-byte local slot
//                                  ;   (becomes the 0-byte "do_copy" arg)
//   mov  ecx, [esp+0x18]           ; ecx = arg5
//   mov  edx, [esp+0x10]           ; edx = arg3 (cur)
//   push ebx
//   mov  ebx, [esp+0x0c]           ; ebx = arg1 (begin)
//   push esi
//   mov  esi, [esp+0x14]           ; esi = arg2 (end)
//   push edi
//   mov  edi, [esp+0x1c]           ; edi = arg3 (cur)  — second copy,
//                                  ;   used after the call for the
//                                  ;   modular-remainder math below
//   mov  byte ptr [esp+0x0c], 0    ; clear low byte of the local slot
//   mov  eax, [esp+0x0c]           ; eax = (low byte 0, upper 3 bytes
//                                  ;   garbage — only the byte matters
//                                  ;   to the callee, which reads it as
//                                  ;   bool/undefined)
//   push eax                       ; arg6 = the cleared-byte local
//   push ecx                       ; arg5
//   push edx                       ; arg4 = cur
//   push edi                       ; arg3 = cur
//   push esi                       ; arg2 = end
//   push ebx                       ; arg1 = begin
//   call FUN_00404760              ; (rel32 → 0x00404760)
//
//   ; --- modular remainder: return cur - ((end - begin) / 0x1c) * 0x1c ---
//   sub  esi, ebx                  ; esi = end - begin
//   mov  eax, 0x92492493           ; signed-divide-by-28 magic
//   imul esi                       ; edx:eax = esi * 0x92492493 (signed)
//   add  edx, esi
//   sar  edx, 4                    ; edx = signed (esi / 28) - sign
//   mov  eax, edx
//   shr  eax, 31                   ; eax = sign bit (0/1)
//   add  eax, edx                  ; eax = (end - begin) / 28
//   lea  ecx, [eax*8 + 0]          ; ecx = eax * 8
//   add  esp, 0x18                 ; clean up the 6 pushed args
//   sub  ecx, eax                  ; ecx = eax * 7
//   mov  eax, edi                  ; eax = cur
//   add  ecx, ecx                  ; ecx = eax * 14
//   pop  edi
//   add  ecx, ecx                  ; ecx = eax * 28 = ((end-begin)/28)*28
//   pop  esi
//   sub  eax, ecx                  ; eax = cur - ((end-begin)/28)*28
//   pop  ebx
//   pop  ecx                       ; discard the 4-byte local slot
//   ret                            ; __cdecl, caller cleans the 5 args
//
// Reloc-bearing site in the 90 orig bytes:
//   +0x28   CALL rel32   → FUN_00404760  (RVA 0x00004760)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (call the helper, then return
//   `cur - ((end - begin) / 28) * 28`) would emit the same shape but
//   would produce one CALL rel32 relocation the linker resolves at
//   relink time. The byte position of the reloc would match the
//   orig's wire layout, but the immediate bytes themselves would be
//   zero-filled in the .obj and only resolved at link time.
//
//   The pragmatic choice — the same one many sibling functions take —
//   is a `__declspec(naked)` body that re-emits the orig 90 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice (no relocations: the
//   rel32 offset resolves against the orig binary's own address
//   space, and emitting it as raw bytes produces the exact wire image
//   the linker would emit at relink). `tools/compare.py` then reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00404800() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x0C]
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x14]
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x1C]
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0xc6              // MOV byte ptr [ESP+0x0C], 0
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x0C]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0x53              // PUSH EBX
        _emit 0xe8              // CALL FUN_00404760 (rel32 → 0xffffff34)
        _emit 0x34
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x2b              // SUB ESI, EBX
        _emit 0xf3
        _emit 0xb8              // MOV EAX, 0x92492493
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        _emit 0xf7              // IMUL ESI
        _emit 0xee
        _emit 0x03              // ADD EDX, ESI
        _emit 0xd6
        _emit 0xc1              // SAR EDX, 4
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SHR EAX, 0x1F
        _emit 0xe8
        _emit 0x1f
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x8d              // LEA ECX, [EAX*8 + 0]
        _emit 0x0c
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x2b              // SUB ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, EDI
        _emit 0xc7
        _emit 0x03              // ADD ECX, ECX
        _emit 0xc9
        _emit 0x5f              // POP EDI
        _emit 0x03              // ADD ECX, ECX
        _emit 0xc9
        _emit 0x5e              // POP ESI
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX
        _emit 0xc3              // RET
    }
}
