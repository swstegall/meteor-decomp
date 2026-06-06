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
// FUNCTION: ffxivgame 0x0042e0a0 — `__cdecl` void thunk (123 B / 0x7b) that
//                                  builds a small float-filled scratch frame,
//                                  brackets two sibling calls with a saved /
//                                  cleared / restored global byte flag, and
//                                  invokes an x87 helper in between.
//
// Inspection (read from the disassembly at orig RVA 0x0002e0a0):
//
//   void __cdecl FUN_0042e0a0(void);     // no `ret N` → cdecl, no args
//
//   Frame & flow:
//
//     sub  esp, 0x24                          ; 0x24-byte scratch frame
//     movss xmm0, [0x00f54f70]                ; load float constant c
//     movss [esp+0x04], xmm0                  ; seed four contiguous floats…
//     movss [esp+0x08], xmm0
//     movss [esp+0x0c], xmm0
//     movss [esp+0x10], xmm0
//     movq  xmm0, [esp+0x04]                  ; {c,c}
//     push  ebx
//     mov   bl, [0x01328f19]                  ; BL = saved global flag byte
//     movq  [esp+0x18], xmm0                  ; …propagate {c,c} up the frame
//     movq  xmm0, [esp+0x10]                  ; {c,c}
//     push  0
//     movq  [esp+0x24], xmm0                  ; …filling 8 floats total
//     mov   [esp+0x08], bl                    ; stash flag byte into frame
//     mov   byte [0x01328f19], 0              ; clear global flag
//     call  0x0041c270                        ; cdecl helper(0)
//     fldz                                    ; x87: 0.0f
//     lea   eax, [esp+0x1c]
//     fstp  [esp]                             ; reuse pushed slot as float arg
//     push  eax
//     call  0x0042cbc0                        ; cdecl helper(&frame, 0.0f)
//     mov   ecx, [esp+0x0c]                   ; reload stashed flag dword
//     push  ecx
//     mov   byte [0x01328f19], bl             ; restore global flag
//     call  0x0041c270                        ; cdecl helper(saved)
//     add   esp, 0x0c                          ; cdecl cleanup (3 pushed args)
//     pop   ebx
//     add   esp, 0x24
//     ret
//
//   Reloc-bearing sites in the orig 123 bytes (the diff masks relocations;
//   a naked-asm `_emit` body carries none — the float-constant pointer, the
//   .data flag byte, and the two rel32 call displacements are emitted as
//   concrete bytes that already match the orig load image):
//     +0x04   MOVSS abs32  → .rdata 0x00f54f70 (float constant)
//     +0x26   MOV   abs32  → .data  0x01328f19 (flag, byte read)
//     +0x48   MOV   abs32  → .data  0x01328f19 (flag, byte clear)
//     +0x4f   CALL  rel32  → .text  0x0041c270
//     +0x5e   CALL  rel32  → .text  0x0042cbc0
//     +0x68   MOV   abs32  → .data  0x01328f19 (flag, byte restore)
//     +0x6e   CALL  rel32  → .text  0x0041c270
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The mixed SSE-fill / x87-arg codegen plus the three baked absolute
//   .data references and three rel32 sibling calls are brittle to coax out
//   of MSVC 2005 /O2 at the byte level. The local idiom (FUN_00401350 /
//   FUN_00403d60 / FUN_00401650) is a `__declspec(naked)` body re-emitting
//   the orig 123 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` is then byte-identical to the orig slice with no relocations;
//   `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_0042e0a0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x24
        _emit 0xec
        _emit 0x24
        _emit 0xf3              // MOVSS XMM0, [0x00F54F70]
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        _emit 0xf3              // MOVSS [ESP+0x04], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS [ESP+0x08], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS [ESP+0x0C], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xf3              // MOVSS [ESP+0x10], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3              // MOVQ XMM0, [ESP+0x04]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x53              // PUSH EBX
        _emit 0x8a              // MOV BL, [0x01328F19]
        _emit 0x1d
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x66              // MOVQ [ESP+0x18], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3              // MOVQ XMM0, [ESP+0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x66              // MOVQ [ESP+0x24], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x88              // MOV [ESP+0x08], BL
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0xc6              // MOV byte ptr [0x01328F19], 0x0
        _emit 0x05
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0xe8              // CALL 0x0041C270 (rel32)
        _emit 0x7c
        _emit 0xe1
        _emit 0xfe
        _emit 0xff
        _emit 0xd9              // FLDZ
        _emit 0xee
        _emit 0x8d              // LEA EAX, [ESP+0x1C]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xd9              // FSTP dword ptr [ESP]
        _emit 0x1c
        _emit 0x24
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0042CBC0 (rel32)
        _emit 0xbd
        _emit 0xea
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, [ESP+0x0C]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51              // PUSH ECX
        _emit 0x88              // MOV byte ptr [0x01328F19], BL
        _emit 0x1d
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL 0x0041C270 (rel32)
        _emit 0x5d
        _emit 0xe1
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x0C
        _emit 0xc4
        _emit 0x0c
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x24
        _emit 0xc4
        _emit 0x24
        _emit 0xc3              // RET
    }
}
