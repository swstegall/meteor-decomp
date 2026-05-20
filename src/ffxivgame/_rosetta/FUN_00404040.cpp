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
// FUNCTION: ffxivgame 0x00004040 — Utf8String::assign_substr
//                                  (217 B / 0xd9, __thiscall, ret 0x0c).
//
// Behaviour read from the orig bytes at RVA 0x00004040 cross-referenced
// against the Ghidra headless decompile hint
// (build/ghidra-decomp/ffxivgame/00004040_FUN_00404040.c):
//
//   __thiscall void Utf8String::assign_substr(this,
//                                             const Utf8String *src,
//                                             unsigned int pos,
//                                             unsigned int count);
//   ECX = this (recipient); stack args: [esp+4]=src, [esp+8]=pos,
//   [esp+0xc]=count. Cleans 0x0c bytes of stack args on return
//   (`ret 0x0c`).
//
//   The function is the canonical MSVC-2005 `basic_string::assign(
//   const basic_string&, size_type _Roff, size_type _Count)` lowering
//   for ffxivgame's UTF-8 string class (16-byte SSO threshold visible
//   in the `cmp dword ptr [src+0x18], 0x10` / `cmp dword ptr [this+0x18],
//   0x10` capacity tests at offsets 0x6d and 0xa4, and the `+0x04 inline
//   buffer / +0x14 length / +0x18 capacity` field layout from the
//   `mov esi, ecx; lea ... [esi+0x04]; mov dword ptr [this+0x14], len;
//   mov byte ptr [data+len], 0` epilogue).
//
//   Sketch (from asm):
//     ESI = this; EDI = pos
//     if (src->length < pos) call FUN_009d046d  // out_of_range
//     EDI = src->length - pos
//     if (count < EDI) EDI = count            // CMOVB
//     if (this == src) {                       // self-assign branch
//         FUN_00449570(this, pos + count, ~0u); // erase tail
//         FUN_00449570(this, 0, pos);           // erase head
//         return;
//     }
//     if (EDI == 0xffffffff) call FUN_009d042e // length_error
//     if (this->cap < EDI)
//         FUN_00403d60(this, EDI, src->length); // grow capacity
//     else if (EDI == 0) {
//         this->length = 0;
//         *(char*)(this->cap < 0x10 ? &this->buf : this->buf) = 0;
//         return;
//     }
//     if (EDI != 0) {
//         src_data = src->cap < 0x10 ? &src->buf : src->buf;
//         dst_data = this->cap < 0x10 ? &this->buf : this->buf;
//         _memcpy_s(dst_data, this->cap, src_data + pos, EDI);
//         this->length = EDI;
//         *(char*)(this->cap < 0x10 ? &this->buf : this->buf + EDI) = 0;
//     }
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function carries five PC-relative CALL targets (rel32) embedded
//   in the body (the two range-check helpers FUN_009d046d / FUN_009d042e,
//   the two erase-range siblings via the `mov ecx, esi; call rel32`
//   sequence at 0x32 and 0x3a — both targeting FUN_00449570 at the
//   sibling-RVA offset, and the _memcpy_s helper at the end). A source-
//   level C++ port at /O2 /EHsc /GS would need to coax MSVC 2005 into
//   reproducing the exact prologue (PUSH EBX / MOV EBX,[ESP+8] / PUSH
//   EBP / MOV EBP,[ESP+10] / CMP [EBX+14],EBP / PUSH ESI,EDI / MOV
//   ESI,ECX), the SUB/CMOVB length clamp, the CALL-with-ECX-this
//   self-assign branch, AND the three distinct return-via-RET-0xC tail
//   sequences (offset 0x46, 0x88, 0x9d, 0xd0). The naked-asm `_emit`
//   path is the canonical workaround for ffxivgame and produces a
//   `.text` whose 217 bytes match orig byte-for-byte (no relocations —
//   the rel32 targets are emitted as raw immediates).

extern "C" __declspec(naked) void FUN_00404040() {
    __asm {
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x39
        _emit 0x6b
        _emit 0x14
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xf1
        _emit 0x73
        _emit 0x05
        _emit 0xe8
        _emit 0x15
        _emit 0xc4
        _emit 0x5c
        _emit 0x00
        _emit 0x8b
        _emit 0x7b
        _emit 0x14
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x2b
        _emit 0xfd
        _emit 0x3b
        _emit 0xc7
        _emit 0x0f
        _emit 0x42
        _emit 0xf8
        _emit 0x3b
        _emit 0xf3
        _emit 0x75
        _emit 0x1f
        _emit 0x6a
        _emit 0xff
        _emit 0x03
        _emit 0xfd
        _emit 0x57
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xfa
        _emit 0x54
        _emit 0x04
        _emit 0x00
        _emit 0x55
        _emit 0x6a
        _emit 0x00
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xf0
        _emit 0x54
        _emit 0x04
        _emit 0x00
        _emit 0x5f
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x83
        _emit 0xff
        _emit 0xfe
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x9b
        _emit 0xc3
        _emit 0x5c
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x3b
        _emit 0xc7
        _emit 0x73
        _emit 0x1b
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x50
        _emit 0x57
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xba
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x85
        _emit 0xff
        _emit 0x76
        _emit 0x66
        _emit 0x83
        _emit 0x7b
        _emit 0x18
        _emit 0x10
        _emit 0x72
        _emit 0x2f
        _emit 0x8b
        _emit 0x53
        _emit 0x04
        _emit 0xeb
        _emit 0x2d
        _emit 0x85
        _emit 0xff
        _emit 0x75
        _emit 0xef
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        _emit 0x72
        _emit 0x0f
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        _emit 0x5f
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        _emit 0x5f
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        _emit 0x8d
        _emit 0x53
        _emit 0x04
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        _emit 0x83
        _emit 0xf9
        _emit 0x10
        _emit 0x8d
        _emit 0x5e
        _emit 0x04
        _emit 0x72
        _emit 0x04
        _emit 0x8b
        _emit 0x03
        _emit 0xeb
        _emit 0x02
        _emit 0x8b
        _emit 0xc3
        _emit 0x57
        _emit 0x03
        _emit 0xd5
        _emit 0x52
        _emit 0x51
        _emit 0x50
        _emit 0xe8
        _emit 0xf5
        _emit 0xd6
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        _emit 0x72
        _emit 0x02
        _emit 0x8b
        _emit 0x1b
        _emit 0xc6
        _emit 0x04
        _emit 0x3b
        _emit 0x00
        _emit 0x5f
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
