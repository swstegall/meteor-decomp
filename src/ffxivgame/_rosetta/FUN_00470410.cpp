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
// FUNCTION: ffxivgame 0x00070410 (VA 0x00470410) — name_cmp
//           __cdecl int name_cmp(const char *filename, const char *name)
//           69 bytes / 0x45
//
// Compares `filename` against `name` using a prefix-plus-extension rule:
//
//   1. Compute n = strlen(name) via an inline pointer-increment loop.
//      The loop top is 16-byte-aligned in the orig binary; MSVC inserts
//      a 3-byte NOP `LEA ECX, [ECX+0]` (8d 49 00) at offset +0x0d as
//      alignment padding between the forward JMP and the loop body.
//
//   2. Call FUN_009d5475(filename, name, n) — the strncmp-equivalent at
//      VA 0x009d5475.  Arguments are pushed right-to-left (__cdecl).
//
//   3. If strncmp ≠ 0 → fall through to the shared epilogue at +0x45
//      (POP EDI / POP ESI / RET, shared with the following function) and
//      return strncmp's non-zero result.
//
//   4. If strncmp = 0 (first n chars match):
//        filename[n] == '\0' or filename[n] == '.'  → return 0
//        otherwise                                   → return 1
//
// Register layout after the prologue:
//   EDX = name       (loaded at entry, before PUSH ESI / PUSH EDI)
//   ESI = name + 1   (strlen sentinel; overwritten with strlen after loop)
//   EAX = walking ptr
//   EDI = filename   (loaded after the strlen loop)
//
// Calling convention: __cdecl — two stack args, caller cleanup, plain RET.
//
// Shared-epilogue note:
//   The `POP EDI / POP ESI / RET` at offsets +0x45–+0x47 belong to the
//   following function (Ghidra sizes this function at 0x45, not 0x48).
//   The `JNZ epilogue_shared` at +0x2e branches there when strncmp
//   returns non-zero; in the .obj that JNZ encodes as `75 15` (short,
//   +21 from the next instruction at +0x30 to the label at +0x45).
//   The epilogue bytes are NOT emitted by this translation unit.
//
// Reloc-bearing site masked by tools/compare.py:
//   REL32 +0x25  →  FUN_009d5475  (strncmp-equivalent, VA 0x009d5475)
//
// Reconstruction strategy — naked __asm passthrough:
//   Three instructions require _emit to pin exact encodings:
//     • Forward JMP +3 (eb 03) — MASM may choose near form for forward jumps.
//     • 3-byte alignment NOP 8d 49 00 — `lea ecx,[ecx]` assembles to
//       the 2-byte form (8d 09); the 3-byte form with disp8=0 must be
//       forced via _emit.
//     • MOV AL,[ESI+EDI] (8a 04 3e) — SIB base/index order is ambiguous
//       in MASM; _emit guarantees base=ESI, index=EDI.

extern "C" int FUN_009d5475();  // strncmp-equivalent (s1, s2, n), __cdecl

extern "C" __declspec(naked) void FUN_00470410() {
    __asm {
        ; --- prologue ---
        mov     edx, dword ptr [esp+0x8]    ; 8b 54 24 08  load name (arg2) before saves
        push    esi                          ; 56
        mov     eax, edx                     ; 8b c2
        push    edi                          ; 57
        lea     esi, [eax+1]                 ; 8d 70 01  ESI = name+1 (strlen sentinel)
        ; Forward JMP to loop body (+3 bytes, short form eb 03).
        ; MASM may emit the near form for forward unconditional jumps,
        ; so we pin it with _emit.
        _emit   0xeb                         ; eb  JMP SHORT opcode
        _emit   0x03                         ; 03  +3 offset → loop_body at +0x10
        ; 3-byte NOP (LEA ECX,[ECX+0]) aligns loop_body to 16-byte boundary.
        _emit   0x8d                         ; 8d
        _emit   0x49                         ; 49  ModRM: mod=01, reg=1(ECX), rm=1(ECX)
        _emit   0x00                         ; 00  disp8 = 0

        ; --- inline strlen(name) loop (top at +0x10, 16-byte aligned) ---
    loop_body:
        mov     cl, byte ptr [eax]           ; 8a 08
        add     eax, 1                       ; 83 c0 01  (sign-extended imm8 form)
        test    cl, cl                       ; 84 c9
        jnz     loop_body                    ; 75 f7  (backward short branch)
        ; EAX = name + strlen(name) + 1

        ; --- compute length and load filename ---
        mov     edi, dword ptr [esp+0xc]     ; 8b 7c 24 0c  EDI = filename (arg1)
        sub     eax, esi                     ; 2b c6  EAX = (name+len+1) - (name+1) = len
        mov     esi, eax                     ; 8b f0  ESI = n

        ; --- call strncmp(filename, name, n) ---
        push    esi                          ; 56  push n (last arg)
        push    edx                          ; 52  push name
        push    edi                          ; 57  push filename (first arg)
        call    FUN_009d5475                 ; e8 rel32  REL32 reloc → FUN_009d5475
        add     esp, 0xc                     ; 83 c4 0c  cdecl caller cleanup

        ; --- check strncmp result ---
        test    eax, eax                     ; 85 c0
        jnz     epilogue_shared              ; 75 15  strncmp ≠ 0: fall to shared epilogue

        ; --- check filename[n] ---
        ; MOV AL,[ESI+EDI*1] where ESI=n, EDI=filename.
        ; 8a 04 3e: SIB base=ESI(6), index=EDI(7), scale=1.
        ; Pinned with _emit — MASM may swap base/index.
        _emit   0x8a                         ; 8a  MOV r8, r/m8
        _emit   0x04                         ; 04  ModRM: mod=00, reg=0(AL), rm=4(SIB)
        _emit   0x3e                         ; 3e  SIB: ss=0, index=7(EDI), base=6(ESI)
        test    al, al                       ; 84 c0
        jz      return_zero                  ; 74 0c  '\0' → return 0
        cmp     al, 0x2e                     ; 3c 2e  '.'?
        jz      return_zero                  ; 74 08  '.' → return 0

        ; --- filename has more non-extension chars: return 1 ---
        pop     edi                          ; 5f
        mov     eax, 1                       ; b8 01 00 00 00
        pop     esi                          ; 5e
        ret                                  ; c3

        ; --- exact match or extension match: return 0 ---
    return_zero:
        xor     eax, eax                     ; 33 c0

        ; --- shared epilogue (offsets +0x45–+0x47) ---
        ; Ghidra originally attributed these 3 bytes to the following
        ; function, but size_overrides.json corrects the size to 72 B
        ; (0x48) so compare.py expects them here.  The JNZ above
        ; (`jnz epilogue_shared`) branches here when strncmp ≠ 0,
        ; returning strncmp's result via EAX.
    epilogue_shared:
        pop     edi                          ; 5f
        pop     esi                          ; 5e
        ret                                  ; c3
    }
}
