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
// FUNCTION: ffxivgame 0x00417a70 — `__thiscall` chunked-stream write helper
//                                   (63 B / 0x3f)
//
// void* __thiscall FUN_00417a70(this /*ECX*/,
//                               const void* src  /*[ESP+0x4 on entry]*/,
//                               unsigned int size /*[ESP+0x8 on entry]*/)
//   -> RET 8  (two caller-cleaned stack arguments)
//   Returns: pointer to the start of the written region.
//
// Object layout (inferred):
//   this+0x14   Chunk*  current;       pointer to current chunk descriptor
//   this+0x18   int     offset;        byte offset within current chunk's buffer
//
//   Chunk layout (each element is 8 bytes in the enclosing vector):
//     Chunk+0x0  char*         data;      raw buffer
//     Chunk+0x4  unsigned int  capacity;  maximum bytes
//
// Behaviour:
//   1. Load size into EBX (before saving EDI to keep arg2's stack slot live).
//   2. Check: if (this->offset + size) >= this->current->capacity -> grow.
//   3. Grow by calling FUN_00417970 (__thiscall, no stack args), which advances
//      the cursor to the next chunk and resets this->offset to 0.
//   4. Compute write destination: dst = this->current->data + this->offset.
//   5. Copy: FUN_009d4600(dst, src, size)  [memcpy-like, __cdecl 3-arg].
//   6. Advance: this->offset += size.
//   7. Return dst (EDI, the pointer to the written region).
//
// The PUSH EDI is interleaved between CMP and JC (does not affect flags).
// MSVC 2005 /O2 emits this ordering to fill a decode slot between the
// compare and the branch without penalising the critical path.
//
// Reloc-bearing sites in the orig 63 bytes (masked by compare.py):
//   +0x18  REL32  code FUN_00417970  (grow / advance-chunk)
//   +0x2c  REL32  code FUN_009d4600  (memcpy-like copy helper)

extern "C" void FUN_00417970();  // __thiscall grow / advance-chunk
extern "C" void FUN_009d4600();  // __cdecl  memcpy-like (dst, src, size)

extern "C" __declspec(naked) void FUN_00417a70() {
    __asm {
        // 00017a70: 53
        push    ebx
        // 00017a71: 8b 5c 24 0c
        mov     ebx, dword ptr [esp + 0xc]   // EBX = size (arg2)
        // 00017a75: 56
        push    esi
        // 00017a76: 8b f1
        mov     esi, ecx                     // ESI = this
        // 00017a78: 8b 46 18
        mov     eax, dword ptr [esi + 0x18]  // EAX = this->offset
        // 00017a7b: 8b 4e 14
        mov     ecx, dword ptr [esi + 0x14]  // ECX = this->current (Chunk*)
        // 00017a7e: 03 c3
        add     eax, ebx                     // EAX = offset + size
        // 00017a80: 3b 41 04
        cmp     eax, dword ptr [ecx + 0x4]   // vs. current->capacity
        // 00017a83: 57
        push    edi                          // save EDI (interleaved, flags unchanged)
        // 00017a84: 72 07
        jc      skip_grow                    // if (offset+size) < capacity -> skip
        // 00017a86: 8b ce
        mov     ecx, esi                     // ECX = this
        // 00017a88: e8 e3 fe ff ff
        call    FUN_00417970                 // this->grow()
    skip_grow:
        // 00017a8d: 8b 56 14
        mov     edx, dword ptr [esi + 0x14]  // EDX = this->current (possibly new)
        // 00017a90: 8b 3a
        mov     edi, dword ptr [edx]         // EDI = current->data
        // 00017a92: 8b 44 24 10
        mov     eax, dword ptr [esp + 0x10]  // EAX = src (arg1; 3 regs saved above)
        // 00017a96: 03 7e 18
        add     edi, dword ptr [esi + 0x18]  // EDI = current->data + offset
        // 00017a99: 53
        push    ebx                          // arg3: size
        // 00017a9a: 50
        push    eax                          // arg2: src
        // 00017a9b: 57
        push    edi                          // arg1: dst
        // 00017a9c: e8 5f cb 5b 00
        call    FUN_009d4600                 // memcpy(dst, src, size)
        // 00017aa1: 01 5e 18
        add     dword ptr [esi + 0x18], ebx  // this->offset += size
        // 00017aa4: 83 c4 0c
        add     esp, 0xc                     // caller-cleans 3 args
        // 00017aa7: 8b c7
        mov     eax, edi                     // return dst
        // 00017aa9: 5f
        pop     edi
        // 00017aaa: 5e
        pop     esi
        // 00017aab: 5b
        pop     ebx
        // 00017aac: c2 08 00
        ret     0x8
    }
}
