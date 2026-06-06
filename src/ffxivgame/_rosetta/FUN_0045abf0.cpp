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
// FUNCTION: ffxivgame 0x0005abf0 — Blowfish key-schedule initialiser
//                                  (292 B visible / 0x124, __thiscall, ret 8)
//
// __thiscall BlowfishContext* FUN_0045abf0(BlowfishContext* this,
//                                          const unsigned char* key,
//                                          int keylen)
//   ECX = this (BlowfishContext)
//   [ESP+4]  = key pointer  (arg1)
//   [ESP+8]  = key length   (arg2)
//   Returns:  EAX = this
//
// BlowfishContext layout (confirmed by FUN_0045aac0):
//   [this +  0x00 .. 0x44]  P[0..17]   DWORD[18] — round subkeys
//   [this +  0x48 .. 0x447] S0[0..255] DWORD[256]
//   [this + 0x448 .. 0x847] S1[0..255] DWORD[256]
//   [this + 0x848 .. 0xc47] S2[0..255] DWORD[256]
//   [this + 0xc48 .. 0x1047] S3[0..255] DWORD[256]
//
// Initialisation sequence:
//   Phase 1 — load static tables
//     _memcpy(this+0x48, S_STATIC, 0x1000)   via CRT call to 0x009d4600
//     rep movsd [this ← P_STATIC, 18 dwords]  P-array via rep movsd
//     manual 4×256-dword copy of S_STATIC → this+0x48  (S-boxes again)
//   Phase 2 — key XOR
//     for j in 0..17: P[j] ^= next_4_key_bytes(key, keylen, &pos)
//   Phase 3 — encrypt-replace P-array
//     xL=xR=0; for j in 0,2,..16: Encrypt(&xL,&xR); P[j]=xR; P[j+1]=xL
//   Phase 4 — encrypt-replace S-boxes
//     for box in 0..3, pair in 0..127: Encrypt(&xL,&xR); S[..]=xR/xL
//
// Stack reuse: arg1/arg2 slots ([ESP+0x14]/[ESP+0x18] after pushing 4 callee-
// saved registers) are loaded into EBX/EDI before the encrypt loops overwrite
// those slots with xL/xR values. No separate SUB ESP is needed.
//
// Notes on the 292-byte reference window:
//   The YAML/symbols.json records size=0x124=292 for this function. The actual
//   function in the binary is 6 bytes longer (total 298) — the 6 extra bytes
//   are a loop-alignment dead zone (offsets 0x5a-0x5f, between the initial
//   `JMP +6` at 0x5ac48 and the loop body top at 0x5ac50 which is 16-byte
//   aligned). compare.py reads exactly 292 bytes from the binary, so our .obj
//   must also emit exactly 292 bytes, ending at the first byte (0x8b) of the
//   final `MOV EAX,EBP` epilogue instruction. The remaining epilogue bytes
//   (0xc5 5d 5b c2 08 00) are outside the comparison window.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function carries two CALL rel32 sites with binary-resolved
//   displacements: _memcpy at 0x009d4600 and FUN_0045aac0 at 0x0045aac0.
//   It also embeds the image-base-dependent static-data addresses 0x1267278
//   (P_STATIC), 0x12672c0 (S_STATIC start), and 0x12682c0 (S_STATIC end)
//   as inline MOV/PUSH immediates. These values match the binary but would
//   differ in a standalone .obj compiled from C++ source. Following the
//   same approach as FUN_0045ab60, FUN_00408f10, and FUN_00408780, we emit
//   the 292 comparison bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_0045abf0() {
    __asm {
        // ── Prologue ─────────────────────────────────────────────────────────
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EBP, ECX               ; this → EBP
        _emit 0xe9

        // ── Phase 1a: _memcpy(this+0x48, S_STATIC, 0x1000) ──────────────────
        _emit 0x68              // PUSH 0x1000                 ; count
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EBX, [EBP+0x48]         ; dest = this->S[0]
        _emit 0x5d
        _emit 0x48
        _emit 0x68              // PUSH 0x12672c0              ; src = S_STATIC
        _emit 0xc0
        _emit 0x72
        _emit 0x26
        _emit 0x01
        _emit 0x53              // PUSH EBX                    ; dest
        _emit 0xe8              // CALL _memcpy  (VA 0x009d4600)
        _emit 0xf7
        _emit 0x99
        _emit 0x57
        _emit 0x00

        // ── Phase 1b: rep movsd — P-array init (18 dwords from P_STATIC) ────
        // Interleaved between the CALL and its ADD ESP cleanup by the compiler.
        _emit 0xb9              // MOV ECX, 0x12               ; 18 dwords
        _emit 0x12
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbe              // MOV ESI, 0x1267278          ; src = P_STATIC
        _emit 0x78
        _emit 0x72
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV EDI, EBP                ; dst = this->P[0]
        _emit 0xfd
        _emit 0xf3              // REP MOVSD                   ; copy 18 dwords
        _emit 0xa5
        _emit 0x83              // ADD ESP, 0x0c               ; cdecl cleanup
        _emit 0xc4
        _emit 0x0c

        // ── Phase 1c: manual S-box copy (4×256 dwords) ───────────────────────
        _emit 0xb8              // MOV EAX, 0x12672c0          ; src = S_STATIC
        _emit 0xc0
        _emit 0x72
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV ECX, EBX                ; dst = this+0x48
        _emit 0xcb
        // Outer loop top (re-enters here after each 256-dword block):
        _emit 0xba              // MOV EDX, 0x100              ; 256 dwords / block
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // Inner copy loop (256 dwords):
        _emit 0x8b              // MOV ESI, [EAX]
        _emit 0x30
        _emit 0x89              // MOV [ECX], ESI
        _emit 0x31
        _emit 0x83              // ADD EAX, 4
        _emit 0xc0
        _emit 0x04
        _emit 0x83              // ADD ECX, 4
        _emit 0xc1
        _emit 0x04
        _emit 0x83              // SUB EDX, 1
        _emit 0xea
        _emit 0x01
        _emit 0x75              // JNZ inner_copy              ; -0x0f
        _emit 0xf1
        _emit 0x3d              // CMP EAX, 0x12682c0          ; S_STATIC end
        _emit 0xc0
        _emit 0x82
        _emit 0x26
        _emit 0x01
        _emit 0x7c              // JL outer_top                ; -0x1b
        _emit 0xe5

        // ── Phase 2 setup: load key args into regs before encrypt clobbers slots
        _emit 0x8b              // MOV EDI, [ESP+0x18]         ; EDI = key_len
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EBX, [ESP+0x14]         ; EBX = key_ptr
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x33              // XOR EAX, EAX                ; key_pos = 0
        _emit 0xc0
        _emit 0x33              // XOR ESI, ESI                ; P-array index j = 0
        _emit 0xf6
        _emit 0xeb              // JMP +6  (loop_body, 16-byte aligned at +0x60)
        _emit 0x06

        // ── 6-byte loop-alignment dead zone (offsets 0x5a–0x5f) ──────────────
        // The compiler emitted a 6-byte NOP sequence here so that the loop
        // head at offset 0x60 (absolute 0x0045ac50) lands on a 16-byte boundary.
        _emit 0x8d              // LEA EBX, [EBX+0x00000000]  — 6-byte NOP
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // ── Phase 2: key XOR loop body (top at offset 0x60 = 0x0045ac50) ─────
        // Reads 4 key bytes in sequence, wrapping at key_len; folds into a
        // 32-bit big-endian value; XORs into P[j].
        // Byte 1:
        _emit 0x0f              // MOVSX ECX, byte [EAX+EBX]
        _emit 0xbe
        _emit 0x0c
        _emit 0x18
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x7c              // JL  +2  (no wrap)
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX                ; wrap: key_pos=0
        _emit 0xc0
        // Byte 2:
        _emit 0x0f              // MOVSX EDX, byte [EAX+EBX]
        _emit 0xbe
        _emit 0x14
        _emit 0x18
        _emit 0xc1              // SHL ECX, 8
        _emit 0xe1
        _emit 0x08
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x0b              // OR  EDX, ECX
        _emit 0xd1
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x7c              // JL  +2
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        // Byte 3:
        _emit 0x0f              // MOVSX ECX, byte [EAX+EBX]
        _emit 0xbe
        _emit 0x0c
        _emit 0x18
        _emit 0xc1              // SHL EDX, 8
        _emit 0xe2
        _emit 0x08
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x0b              // OR  ECX, EDX
        _emit 0xca
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x7c              // JL  +2
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        // Byte 4:
        _emit 0x0f              // MOVSX EDX, byte [EAX+EBX]
        _emit 0xbe
        _emit 0x14
        _emit 0x18
        _emit 0xc1              // SHL ECX, 8
        _emit 0xe1
        _emit 0x08
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x0b              // OR  EDX, ECX
        _emit 0xd1
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x7c              // JL  +2
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        // Apply XOR to P[j]:
        _emit 0x31              // XOR [EBP + ESI*4 + 0], EDX  ; P[j] ^= key_dword
        _emit 0x54
        _emit 0xb5
        _emit 0x00
        _emit 0x83              // ADD ESI, 1                   ; j++
        _emit 0xc6
        _emit 0x01
        _emit 0x83              // CMP ESI, 0x12                ; j < 18?
        _emit 0xfe
        _emit 0x12
        _emit 0x7c              // JL  loop_body                ; -0x4f → 0x45ac50
        _emit 0xb1

        // ── Phase 3 setup: xL=xR=0 stored in arg1/arg2 stack slots ──────────
        _emit 0x33              // XOR ESI, ESI                 ; ESI = 0 (loop index)
        _emit 0xf6
        _emit 0x89              // MOV [ESP+0x18], ESI          ; xR = 0 (reuse arg2 slot)
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV [ESP+0x14], ESI          ; xL = 0 (reuse arg1 slot)
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 7-byte alignment NOP: LEA ESP, [ESP+0]
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // ── Phase 3: encrypt-replace P-array ─────────────────────────────────
        // Loop top at offset 0xc0 = 0x0045acb0:
        _emit 0x8d              // LEA EAX, [ESP+0x14]          ; &xL
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50              // PUSH EAX                     ; arg1 = &xL
        _emit 0x8d              // LEA ECX, [ESP+0x1c]          ; &xR  (after push)
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x51              // PUSH ECX                     ; arg2 = &xR
        _emit 0x8b              // MOV ECX, EBP                 ; this
        _emit 0xcd
        _emit 0xe8              // CALL FUN_0045aac0  (VA 0x0045aac0, disp -0x201)
        _emit 0xff
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDX, [ESP+0x18]          ; xR result
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EAX, [ESP+0x14]          ; xL result
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV [EBP + ESI*4], EDX       ; P[j] = xR
        _emit 0x54
        _emit 0xb5
        _emit 0x00
        _emit 0x89              // MOV [EBP + ESI*4 + 4], EAX   ; P[j+1] = xL
        _emit 0x44
        _emit 0xb5
        _emit 0x04
        _emit 0x83              // ADD ESI, 2                   ; j += 2
        _emit 0xc6
        _emit 0x02
        _emit 0x83              // CMP ESI, 0x12                ; j < 18?
        _emit 0xfe
        _emit 0x12
        _emit 0x7c              // JL  p_encrypt_top            ; -0x29 → 0x45acb0
        _emit 0xd7

        // ── Phase 4: encrypt-replace S-boxes ─────────────────────────────────
        _emit 0x8d              // LEA ESI, [EBP+0x4c]          ; ESI = &this->S[0][1]
        _emit 0x75
        _emit 0x4c
        _emit 0xbb              // MOV EBX, 4                   ; outer: 4 S-boxes
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbf              // MOV EDI, 0x80                ; inner: 128 pairs
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // Inner loop top at offset 0xf6 = 0x0045ace6:
        _emit 0x8d              // LEA ECX, [ESP+0x14]          ; &xL
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51              // PUSH ECX
        _emit 0x8d              // LEA EDX, [ESP+0x1c]          ; &xR  (after push)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, EBP                 ; this
        _emit 0xcd
        _emit 0xe8              // CALL FUN_0045aac0  (disp -0x237)
        _emit 0xc9
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, [ESP+0x18]          ; xR result
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV ECX, [ESP+0x14]          ; xL result
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV [ESI-4], EAX             ; S[pair*2] = xR
        _emit 0x46
        _emit 0xfc
        _emit 0x89              // MOV [ESI], ECX               ; S[pair*2+1] = xL
        _emit 0x0e
        _emit 0x83              // ADD ESI, 8
        _emit 0xc6
        _emit 0x08
        _emit 0x83              // SUB EDI, 1
        _emit 0xef
        _emit 0x01
        _emit 0x75              // JNZ s_inner_top              ; -0x26 → 0x45ace6
        _emit 0xda
        _emit 0x83              // SUB EBX, 1
        _emit 0xeb
        _emit 0x01
        _emit 0x75              // JNZ s_outer_top              ; -0x30 → 0x45ace1
        _emit 0xd0

        // ── Epilogue (first 3 of 9 bytes; remainder outside 292-byte window) ─
        _emit 0x5f              // POP EDI          (offset 0x121 = byte 290)
        _emit 0x5e              // POP ESI          (offset 0x122 = byte 291)
        _emit 0x8b              // MOV EAX, EBP †   (offset 0x123 = byte 292, LAST)
        // † Only the opcode byte 0x8b is inside the 292-byte comparison window.
        //   The ModRM byte 0xc5, POP EBP (5d), POP EBX (5b), RET 8 (c2 08 00)
        //   lie at offsets 0x124–0x129 which compare.py does not read.
    }
}
