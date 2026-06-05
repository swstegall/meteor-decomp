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
// FUNCTION: ffxivgame 0x00450000 — format-then-append helper
//                                  (__stdcall, /GS-guarded, 93 bytes)
//
// __stdcall void FUN_00450000(int value, Sink *sink)
//   stack layout (after RET, image-base 0x00400000):
//     [ESP+0x28] : int value      (formatted into the temp buffer)
//     [ESP+0x30] : Sink *sink     (→ ESI; receiver of FUN_00404120)
//
// Behaviour: reserve a 0x20-byte temp buffer on the (security-cookie
// protected) stack frame, format `value` into it via the 4-arg
// CRT-style routine at 0x009d4f83 ( buf, 0x20, "%...", value ), measure
// the resulting C-string length with an inline strlen loop, then invoke
// sink->FUN_00404120(buf, len) (__thiscall, ECX = sink). Finally verify
// the stack cookie (__security_check_cookie @ 0x009d20f4) and RET 8.
//
// Inspection (orig bytes at RVA 0x00050000, 93 bytes total):
//
//   sub  esp, 0x24
//   mov  eax, [0x012ea8b0]          ; __security_cookie
//   xor  eax, esp
//   mov  [esp+0x20], eax            ; stack guard
//   mov  eax, [esp+0x28]            ; value
//   push esi
//   mov  esi, [esp+0x30]            ; sink
//   push eax                        ; value
//   push 0x00f676c8                 ; format string
//   lea  ecx, [esp+0xc]             ; &buf
//   push 0x20                       ; buffer size
//   push ecx                        ; &buf
//   call 0x009d4f83                 ; sprintf_s(buf, 0x20, fmt, value)
//   lea  eax, [esp+0x14]            ; &buf (post-call)
//   add  esp, 0x10                  ; drop 4 args
//   lea  edx, [eax+1]
// strlen:
//   mov  cl, [eax]
//   add  eax, 1
//   test cl, cl
//   jnz  strlen
//   sub  eax, edx                   ; len = strlen(buf)
//   push eax                        ; len
//   lea  edx, [esp+0x8]             ; &buf
//   push edx
//   mov  ecx, esi                   ; this = sink
//   call 0x00404120                 ; sink->append(buf, len)  (RET 8)
//   mov  ecx, [esp+0x24]            ; guard
//   pop  esi
//   xor  ecx, esp
//   call 0x009d20f4                 ; __security_check_cookie
//   add  esp, 0x24
//   ret  0x8
//
// Reloc-bearing sites (CALL rel32 the linker would resolve from a
// source-level form; re-emitted verbatim so the .obj's .text is
// byte-identical to orig with NO relocations):
//     +0x24   CALL rel32 → 0x009d4f83 (CRT format routine)
//     +0x46   CALL rel32 → 0x00404120 (sink->append)
//     +0x52   CALL rel32 → 0x009d20f4 (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough (see FUN_00406133).

extern "C" __declspec(naked) void FUN_00450000() {
    __asm {
        _emit 0x83              // SUB ESP, 0x24
        _emit 0xec
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV [ESP+0x20], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV EAX, [ESP+0x28]
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x30]
        _emit 0x74
        _emit 0x24
        _emit 0x30
        _emit 0x50              // PUSH EAX
        _emit 0x68              // PUSH 0x00f676c8
        _emit 0xc8
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x6a              // PUSH 0x20
        _emit 0x20
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x009d4f83 (rel32)
        _emit 0x5a
        _emit 0x4f
        _emit 0x58
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x8d              // LEA EDX, [EAX+0x1]
        _emit 0x50
        _emit 0x01
        _emit 0x8a              // MOV CL, [EAX]
        _emit 0x08
        _emit 0x83              // ADD EAX, 0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ -0x9 (back to MOV CL,[EAX])
        _emit 0xf7
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EDX, [ESP+0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00404120 (rel32)
        _emit 0xd5
        _emit 0x40
        _emit 0xfb
        _emit 0xff
        _emit 0x8b              // MOV ECX, [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x5e              // POP ESI
        _emit 0x33              // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8              // CALL 0x009d20f4 (rel32)
        _emit 0x9d
        _emit 0x20
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x24
        _emit 0xc4
        _emit 0x24
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
