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
// FUNCTION: ffxivgame 0x00445600 — UTF-8 encode one codepoint (__cdecl, 110 B)
//
// int FUN_00445600(unsigned short codepoint, unsigned char *buf)
//
//   Encodes a single Unicode codepoint into up to 3 UTF-8 bytes.
//   If buf == NULL, returns the byte count needed without writing.
//   If buf != NULL, writes the encoded bytes and returns the byte count.
//
//   Return value: 1, 2, or 3.
//
// Calling convention: __cdecl (caller cleans; no callee-saved regs used)
//   [ESP+4]  = codepoint   (unsigned short, passed as dword)
//   [ESP+8]  = buf         (unsigned char *, pointer to output buffer)
//
// Control-flow layout (relative to function start):
//
//   +0x00  MOV ECX,[ESP+8]         ; ECX = buf
//   +0x04  TEST ECX,ECX            ; set flags for null check
//   +0x06  MOV EAX,[ESP+4]         ; EAX = codepoint (AX = low 16 bits)
//   +0x0a  JNZ +0x10 → +0x1c      ; if buf != NULL, go to write path
//
//   Size-only path (buf == NULL):
//   +0x0c  CMP AX,0x0080
//   +0x10  JC  +0x12 → +0x24      ; if codepoint < 0x80: jump to return 1
//   +0x12  CMP AX,0x0800
//   +0x16  SBB EAX,EAX             ; CF=1(< 0x800) → EAX=-1; CF=0 → EAX=0
//   +0x18  ADD EAX,3               ; -1+3=2 or 0+3=3
//   +0x1b  RET
//
//   Write path, 1-byte case (buf != NULL, codepoint < 0x80):
//   +0x1c  CMP AX,0x0080
//   +0x20  JNC +0x08 → +0x2a      ; if codepoint >= 0x80, go to 2-byte check
//   +0x22  MOV [ECX],AL            ; write single byte
//   (fall-through to shared return-1 block)
//
//   Shared return-1 (reached from both size-only < 0x80 and write 1-byte):
//   +0x24  MOV EAX,1
//   +0x29  RET
//
//   Write path, 2-byte case (buf != NULL, 0x80 <= codepoint < 0x800):
//   +0x2a  CMP AX,0x0800
//   +0x2e  JNC +0x17 → +0x47      ; if codepoint >= 0x800, go to 3-byte
//   +0x30  MOV EDX,EAX             ; copy codepoint
//   +0x32  SHR EDX,6               ; bits [11:6]
//   +0x35  AND AL,0x3f             ; bits [5:0]
//   +0x37  OR  DL,0xc0             ; 110xxxxx prefix
//   +0x3a  OR  AL,0x80             ; 10xxxxxx continuation
//   +0x3c  MOV [ECX],DL            ; write byte 0
//   +0x3e  MOV [ECX+1],AL          ; write byte 1
//   +0x41  MOV EAX,2
//   +0x46  RET
//
//   Write path, 3-byte case (buf != NULL, codepoint >= 0x800):
//   +0x47  MOV DX,AX               ; 16-bit copy (66 8b d0)
//   +0x4a  SHR DX,0xc              ; bits [15:12]
//   +0x4e  OR  DL,0xe0             ; 1110xxxx prefix
//   +0x51  MOV [ECX],DL            ; write byte 0
//   +0x53  MOV EDX,EAX             ; copy codepoint (32-bit for 6-bit shift)
//   +0x55  SHR EDX,6               ; bits [11:6]
//   +0x58  AND DL,0x3f             ; mask bits [5:0] of shifted value
//   +0x5b  AND AL,0x3f             ; bits [5:0] of codepoint
//   +0x5d  OR  DL,0x80             ; 10xxxxxx continuation
//   +0x60  OR  AL,0x80             ; 10xxxxxx continuation
//   +0x62  MOV [ECX+1],DL          ; write byte 1
//   +0x65  MOV [ECX+2],AL          ; write byte 2
//   +0x68  MOV EAX,3
//   +0x6d  RET
//
// No relocations (no CALLs, no absolute addresses) — naked-asm byte
// passthrough produces a .text section byte-identical to the original.

extern "C" __declspec(naked) void FUN_00445600() {
    __asm {
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x75              // JNZ +0x10 (→ +0x1c, write path)
        _emit 0x10
        _emit 0x66              // CMP AX, 0x0080
        _emit 0x3d
        _emit 0x80
        _emit 0x00
        _emit 0x72              // JC +0x12 (→ +0x24, return 1)
        _emit 0x12
        _emit 0x66              // CMP AX, 0x0800
        _emit 0x3d
        _emit 0x00
        _emit 0x08
        _emit 0x1b              // SBB EAX, EAX
        _emit 0xc0
        _emit 0x83              // ADD EAX, 0x3
        _emit 0xc0
        _emit 0x03
        _emit 0xc3              // RET
        _emit 0x66              // CMP AX, 0x0080   (write path: +0x1c)
        _emit 0x3d
        _emit 0x80
        _emit 0x00
        _emit 0x73              // JNC +0x08 (→ +0x2a, 2-byte check)
        _emit 0x08
        _emit 0x88              // MOV byte ptr [ECX], AL
        _emit 0x01
        _emit 0xb8              // MOV EAX, 0x1     (shared return-1: +0x24)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x66              // CMP AX, 0x0800   (2-byte check: +0x2a)
        _emit 0x3d
        _emit 0x00
        _emit 0x08
        _emit 0x73              // JNC +0x17 (→ +0x47, 3-byte path)
        _emit 0x17
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0xc1              // SHR EDX, 0x6
        _emit 0xea
        _emit 0x06
        _emit 0x24              // AND AL, 0x3f
        _emit 0x3f
        _emit 0x80              // OR DL, 0xc0
        _emit 0xca
        _emit 0xc0
        _emit 0x0c              // OR AL, 0x80
        _emit 0x80
        _emit 0x88              // MOV byte ptr [ECX], DL
        _emit 0x11
        _emit 0x88              // MOV byte ptr [ECX+0x1], AL
        _emit 0x41
        _emit 0x01
        _emit 0xb8              // MOV EAX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0x66              // MOV DX, AX       (3-byte path: +0x47)
        _emit 0x8b
        _emit 0xd0
        _emit 0x66              // SHR DX, 0xc
        _emit 0xc1
        _emit 0xea
        _emit 0x0c
        _emit 0x80              // OR DL, 0xe0
        _emit 0xca
        _emit 0xe0
        _emit 0x88              // MOV byte ptr [ECX], DL
        _emit 0x11
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0xc1              // SHR EDX, 0x6
        _emit 0xea
        _emit 0x06
        _emit 0x80              // AND DL, 0x3f
        _emit 0xe2
        _emit 0x3f
        _emit 0x24              // AND AL, 0x3f
        _emit 0x3f
        _emit 0x80              // OR DL, 0x80
        _emit 0xca
        _emit 0x80
        _emit 0x0c              // OR AL, 0x80
        _emit 0x80
        _emit 0x88              // MOV byte ptr [ECX+0x1], DL
        _emit 0x51
        _emit 0x01
        _emit 0x88              // MOV byte ptr [ECX+0x2], AL
        _emit 0x41
        _emit 0x02
        _emit 0xb8              // MOV EAX, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
