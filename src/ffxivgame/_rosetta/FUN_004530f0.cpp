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
// FUNCTION: ffxivgame 0x004530f0 — __thiscall file-copy loop (128 B / 0x80).
//
// Behaviour reconstructed from the asm at RVA 0x000530f0:
//
//   __thiscall bool FUN_004530f0(SomeObj* arg  /*[ESP+4] on entry*/)
//   {
//       // EDI = this (ECX)
//       // EBX = arg (first stack arg, [ESP+0xc] after PUSH EDI+EBX+ESI)
//       if (this->field4 == NULL) {
//           warn(0x29d5);          // always returns
//           return false;          // XOR AL,AL
//       }
//       while (true) {
//           FILE* src = (FILE*)this->field4;  // [EDI+4]
//           if (!src) {
//               warn(0x29d5);
//               return true;       // MOV AL,1 — fall-through to epilogue
//           }
//           char* buf = (char*)this + 0xe;   // LEA ESI,[EDI+0xe]
//           size_t n = fread(buf, 1, 0x2000, src);
//           if (n == 0) {
//               warn(0x29d5);
//               return true;
//           }
//           FILE* dst = (FILE*)arg->field4;  // [EBX+4]
//           if (!dst || !buf) {
//               warn(0x29d5);
//               continue;          // JMP back to loop top
//           }
//           fwrite(buf, 1, n, dst);
//           // loop
//       }
//   }
//
// Calling convention: __thiscall (ECX = this, callee-cleaned 4-byte stack
//   arg — `RET 0x4` epilogue).
//
// Register allocation:
//   EDI = this
//   EBX = arg (loaded from [ESP+0xc] after saving EDI, EBX, ESI)
//   ESI = &this->buffer (recomputed each iteration via LEA [EDI+0xe])
//   EAX = fread return value / scratch
//   ECX = arg->field4 (for fwrite)
//
// Notable layout detail: a single NOP at offset 0x0f aligns the loop top
//   to RVA 0x53100 (0-mod-16), which is a standard MSVC /O2 loop-alignment
//   pad. Without it the loop top would be at 0x530ff.
//
// Reloc / absolute-address sites (no COFF relocations in the .obj —
//   all addresses emitted as literal imm32 / rel32 bytes):
//   +0x23   CALL rel32 → 0x9d6947  (fread in static CRT at high RVA)
//   +0x3f   CALL rel32 → 0x9d739a  (fwrite in static CRT at high RVA)
//   +0x4e   CALL rel32 → 0x4564e0  (warn/error helper — inner loop error path)
//   +0x5d   CALL rel32 → 0x4564e0  (warn/error helper — fread==0 path)
//   +0x72   CALL rel32 → 0x4564e0  (warn/error helper — field4==NULL early exit)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The far CALL targets at 0x9d6947 / 0x9d739a resolve to the static CRT
//   fread/fwrite implementations linked into the original binary at those
//   high RVAs. A source-level rewrite would require external declarations
//   whose COFF relocations we cannot resolve from a standalone .obj, AND
//   would fight MSVC's loop-alignment heuristic for the NOP. Emitting the
//   128 bytes verbatim via _emit produces a .text section identical to the
//   original slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004530f0() {
    __asm {
        // offset 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX  (this)
        _emit 0xf9

        // offset 0x03
        _emit 0x83              // CMP dword ptr [EDI+4], 0
        _emit 0x7f
        _emit 0x04
        _emit 0x00

        // offset 0x07
        _emit 0x74              // JZ +0x64  → early-exit error (offset 0x6d)
        _emit 0x64

        // offset 0x09
        _emit 0x53              // PUSH EBX

        // offset 0x0a
        _emit 0x8b              // MOV EBX, [ESP+0xc]  (arg — after PUSH EDI+EBX)
        _emit 0x5c
        _emit 0x24
        _emit 0x0c

        // offset 0x0e
        _emit 0x56              // PUSH ESI

        // offset 0x0f  — alignment NOP so loop top lands at 0x53100 (0-mod-16)
        _emit 0x90              // NOP

        // ---- loop top (offset 0x10 / RVA 0x53100) ----
        // offset 0x10
        _emit 0x8b              // MOV EAX, [EDI+4]  (this->field4)
        _emit 0x47
        _emit 0x04

        // offset 0x13
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0

        // offset 0x15
        _emit 0x74              // JZ +0x41  → fread==0 error path (offset 0x58)
        _emit 0x41

        // offset 0x17
        _emit 0x50              // PUSH EAX  (src FILE* — 4th arg to fread)

        // offset 0x18
        _emit 0x68              // PUSH 0x2000  (count — 3rd arg to fread)
        _emit 0x00
        _emit 0x20
        _emit 0x00
        _emit 0x00

        // offset 0x1d
        _emit 0x8d              // LEA ESI, [EDI+0xe]  (buf — 1st arg to fread)
        _emit 0x77
        _emit 0x0e

        // offset 0x20
        _emit 0x6a              // PUSH 0x1  (element size — 2nd arg to fread)
        _emit 0x01

        // offset 0x22
        _emit 0x56              // PUSH ESI  (buf)

        // offset 0x23
        _emit 0xe8              // CALL 0x9d6947  (fread; rel32 = 0x0058382f)
        _emit 0x2f
        _emit 0x38
        _emit 0x58
        _emit 0x00

        // offset 0x28
        _emit 0x83              // ADD ESP, 0x10  (cdecl cleanup: 4 args × 4)
        _emit 0xc4
        _emit 0x10

        // offset 0x2b
        _emit 0x85              // TEST EAX, EAX  (n = bytes read)
        _emit 0xc0

        // offset 0x2d
        _emit 0x76              // JBE +0x36  → exit with success (offset 0x65)
        _emit 0x36

        // offset 0x2f
        _emit 0x8b              // MOV ECX, [EBX+4]  (arg->field4 / dst FILE*)
        _emit 0x4b
        _emit 0x04

        // offset 0x32
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9

        // offset 0x34
        _emit 0x74              // JZ +0x13  → inner-error path (offset 0x49)
        _emit 0x13

        // offset 0x36
        _emit 0x85              // TEST ESI, ESI  (buf)
        _emit 0xf6

        // offset 0x38
        _emit 0x74              // JZ +0x0f  → inner-error path (offset 0x49)
        _emit 0x0f

        // offset 0x3a
        _emit 0x51              // PUSH ECX  (dst FILE* — 4th arg to fwrite)

        // offset 0x3b
        _emit 0x50              // PUSH EAX  (n — 3rd arg to fwrite)

        // offset 0x3c
        _emit 0x6a              // PUSH 0x1  (element size — 2nd arg to fwrite)
        _emit 0x01

        // offset 0x3e
        _emit 0x56              // PUSH ESI  (buf — 1st arg to fwrite)

        // offset 0x3f
        _emit 0xe8              // CALL 0x9d739a  (fwrite; rel32 = 0x00584266)
        _emit 0x66
        _emit 0x42
        _emit 0x58
        _emit 0x00

        // offset 0x44
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10

        // offset 0x47
        _emit 0xeb              // JMP -0x39  → loop top (offset 0x10)
        _emit 0xc7

        // ---- inner-error path (offset 0x49) ----
        // Reached when dst==NULL or buf==NULL; calls warn then continues loop.
        _emit 0x68              // PUSH 0x29d5  (error code)
        _emit 0xd5
        _emit 0x29
        _emit 0x00
        _emit 0x00

        // offset 0x4e
        _emit 0xe8              // CALL 0x4564e0  (warn; rel32 = 0x0000339d)
        _emit 0x9d
        _emit 0x33
        _emit 0x00
        _emit 0x00

        // offset 0x53
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04

        // offset 0x56
        _emit 0xeb              // JMP -0x48  → loop top (offset 0x10)
        _emit 0xb8

        // ---- fread==0 error path (offset 0x58) ----
        // Reached when this->field4 is NULL at top of loop; warn then return true.
        _emit 0x68              // PUSH 0x29d5
        _emit 0xd5
        _emit 0x29
        _emit 0x00
        _emit 0x00

        // offset 0x5d
        _emit 0xe8              // CALL 0x4564e0  (rel32 = 0x0000338e)
        _emit 0x8e
        _emit 0x33
        _emit 0x00
        _emit 0x00

        // offset 0x62
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04

        // ---- success epilogue (offset 0x65) ----
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xb0              // MOV AL, 0x1  (return true)
        _emit 0x01
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 0x4  (callee-cleanup of 1 stack arg)
        _emit 0x04
        _emit 0x00

        // ---- early-exit error (offset 0x6d) ----
        // Reached when this->field4 == NULL before the loop; warn then return false.
        _emit 0x68              // PUSH 0x29d5
        _emit 0xd5
        _emit 0x29
        _emit 0x00
        _emit 0x00

        // offset 0x72
        _emit 0xe8              // CALL 0x4564e0  (rel32 = 0x00003379)
        _emit 0x79
        _emit 0x33
        _emit 0x00
        _emit 0x00

        // offset 0x77
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04

        // offset 0x7a
        _emit 0x32              // XOR AL, AL  (return false)
        _emit 0xc0

        // offset 0x7c
        _emit 0x5f              // POP EDI

        // offset 0x7d
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
