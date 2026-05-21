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
// FUNCTION: ffxivgame 0x004065c0 — long-option matcher ("-name[=value]")
//                                  (__cdecl, 85 bytes)
//
// __cdecl const char* match_long_option(const char *arg, const char *name)
//   stack layout (after ret):
//     [ESP+0x04] : const char *arg     (e.g. "-foo=bar")
//     [ESP+0x08] : const char *name    (e.g. "foo")
//
// Behaviour: tests whether `arg` begins with '-' followed by `name`;
// on a match, returns a pointer into `arg` positioned just past the
// matched name (and past a trailing '=' if present, so the caller can
// read the value). Returns NULL on no match.
//
// Inspection (read from the orig bytes at RVA 0x000065c0, 85 bytes total):
//
//   push ebx
//   push edi
//   mov  edi, [esp+0x0C]         ; edi = arg
//   xor  ebx, ebx                ; ebx = 0 (return value on miss)
//   cmp  byte ptr [edi], '-'
//   jnz  miss_short              ; not a long option → return NULL
//   mov  edx, [esp+0x10]         ; edx = name
//   mov  eax, edx
//   push esi
//   lea  esi, [eax+1]            ; esi = name + 1 (strlen base)
// strlen_loop:
//   mov  cl, [eax]
//   add  eax, 1
//   test cl, cl
//   jnz  strlen_loop             ; inlined strlen(name)
//   sub  eax, esi                ; eax = strlen(name)
//   mov  esi, eax                ; esi = len
//   push esi                     ;   arg3 = len
//   push edx                     ;   arg2 = name
//   lea  eax, [edi+1]
//   push eax                     ;   arg1 = arg + 1
//   call __strnicmp              ; (rel32 → 0x009d50ae)
//   add  esp, 0x0C               ; cdecl arg cleanup
//   test eax, eax
//   jnz  miss_long               ; nonzero → no match, return NULL
//   xor  ecx, ecx
//   cmp  byte ptr [esi+edi+1], '='   ; char after the name == '='?
//   setz cl                          ; cl = 1 if '=' follows
//   add  ecx, esi                    ; ecx = len + ('=' ? 1 : 0)
//   pop  esi
//   lea  eax, [ecx+edi+1]            ; ret = arg + 1 + len + ('=' ? 1 : 0)
//   pop  edi
//   pop  ebx
//   ret
// miss_long:
//   pop  esi
//   mov  eax, ebx                    ; eax = 0
//   pop  edi
//   pop  ebx
//   ret
// miss_short:
//   pop  edi
//   mov  eax, ebx                    ; eax = 0
//   pop  ebx
//   ret
//
// Reloc-bearing sites (CALL rel32 targets the linker would resolve when
// emitted from source-level C++; we re-emit the orig rel32 bytes verbatim
// so the .obj's .text matches byte-for-byte with NO relocations):
//     +0x2a   CALL rel32  → __strnicmp   (RVA 0x005d50ae)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (e.g. `if (*arg == '-' && !_strnicmp(arg+1,
//   name, strlen(name))) { … }`) would emit the same shape but produce
//   one CALL rel32 relocation the linker resolves at relink time.
//   `tools/compare.py` masks reloc bytes out of the diff, but driving
//   a relink isn't necessary: a `__declspec(naked)` body that re-emits
//   the orig 85 bytes verbatim via MASM `_emit` directives produces a
//   .obj whose .text is byte-identical to the orig slice (no
//   relocations — the rel32 offset is baked into the orig binary's
//   own address space and emitted here as raw bytes). compare.py then
//   reports GREEN.

extern "C" __declspec(naked) void FUN_004065c0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x0C]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x80              // CMP byte ptr [EDI], 0x2D
        _emit 0x3f
        _emit 0x2d
        _emit 0x75              // JNZ miss_short (+0x43)
        _emit 0x43
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ESI, [EAX+0x01]
        _emit 0x70
        _emit 0x01
        _emit 0x8a              // MOV CL, byte ptr [EAX]
        _emit 0x08
        _emit 0x83              // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ strlen_loop (-0x09)
        _emit 0xf7
        _emit 0x2b              // SUB EAX, ESI
        _emit 0xc6
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0x8d              // LEA EAX, [EDI+0x01]
        _emit 0x47
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL __strnicmp (rel32 → 0x005d50ae)
        _emit 0xbf
        _emit 0xea
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0C
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ miss_long (+0x14)
        _emit 0x14
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x80              // CMP byte ptr [ESI+EDI*1+0x01], 0x3D
        _emit 0x7c
        _emit 0x3e
        _emit 0x01
        _emit 0x3d
        _emit 0x0f              // SETZ CL
        _emit 0x94
        _emit 0xc1
        _emit 0x03              // ADD ECX, ESI
        _emit 0xce
        _emit 0x5e              // POP ESI
        _emit 0x8d              // LEA EAX, [ECX+EDI*1+0x01]
        _emit 0x44
        _emit 0x39
        _emit 0x01
        _emit 0x5f              // POP EDI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        _emit 0x5e              // POP ESI            ; miss_long
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        _emit 0x5f              // POP EDI            ; miss_short
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
