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
// FUNCTION: ffxivgame 0x00033f00 — `__thiscall` allocate-and-enqueue a small
//                                  8-byte command/event object (79 B / 0x4f).
//
// Behaviour read from the disassembly at orig RVA 0x00033f00:
//
//   void __thiscall FUN_00433f00(C *this, void *payload);
//
//     // A global registry pointer (.data 0x01328d90) holds a small struct:
//     //   struct Registry { unsigned char index; /*pad*/ Slot *slots; };
//     // index (byte at +0) selects a 28-byte (0x1c) slot record; slots
//     // array base lives at +4. The slot pointer becomes the `this` for
//     // the allocator call.
//     Registry *reg  = *(Registry **)0x01328d90;
//     unsigned   idx = reg->index;               // movzx eax, byte [ecx]
//     Slot      *slot = reg->slots + idx;         // base + idx*0x1c
//
//     // slot->Allocate(8) — __thiscall pool allocator (FUN_00417ab0),
//     // returns a fresh 8-byte block or null.
//     Obj *o = slot->Allocate(8);
//     if (o) {
//         o->vtbl    = &vtable_00f64960;          // mov [eax], 0xf64960
//         o->payload = payload;                   // mov [eax+4], arg
//         this->sink->Enqueue(o);                 // (this+0xc)->Enqueue(o)
//     } else {
//         this->sink->Enqueue(0);                 // null on alloc failure
//     }
//
//   The `idx * 0x1c` (28-byte stride) is built with the canonical MSVC
//   strength-reduction `lea edx,[eax*8]; sub edx,eax` (→ eax*7) followed by
//   `lea ecx,[eax+edx*4]` (base + eax*7*4 = base + idx*28).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function mixes a strength-reduced index computation, two .data/.rdata
//   absolute references (the registry global and the vtable immediate) and
//   three rel32 calls into a two-arm shape that MSVC 2005 /O2 only emits with
//   this exact register schedule. Re-deriving it from C++ source would be
//   brittle, so we pin the 79 bytes with naked MASM carrying real symbolic
//   operands; compare.py reloc-masks the five 4-byte windows: 2 DIR32 (the
//   registry global load and the vtable immediate) and 3 REL32 (the alloc
//   call + the two Enqueue calls).
//
// Reloc-bearing sites (4-byte windows, wildcarded by compare.py):
//   +0x05  MOV ECX,[data_01328d90]   (dir32, .data registry pointer)
//   +0x1e  CALL FUN_00417ab0         (rel32, Slot::Allocate)
//   +0x2c  MOV [EAX],offset vtable   (dir32, .rdata vtable 0x00f64960)
//   +0x38  CALL FUN_0043c2d0         (rel32, sink->Enqueue, found arm)
//   +0x47  CALL FUN_0043c2d0         (rel32, sink->Enqueue, null arm)

extern "C" {
    // .data — RVA 0x01328d90. Pointer to the command/event registry struct.
    extern int data_01328d90;

    // .rdata — RVA 0x00f64960. Vtable installed into the allocated object.
    extern int data_00f64960;

    // .text — RVA 0x00417ab0. Slot::Allocate(size) — __thiscall, returns
    // void* in EAX (or null), cleans its single stack arg.
    int FUN_00417ab0();

    // .text — RVA 0x0043c2d0. sink->Enqueue(obj) — __thiscall, cleans its
    // single stack arg.
    int FUN_0043c2d0();
}

extern "C" __declspec(naked) void FUN_00433f00() {
    __asm {
        push    esi                                   // 56
        mov     esi, ecx                              // 8b f1         this
        mov     ecx, dword ptr [data_01328d90]        // 8b 0d ?? ?? ?? ??
        movzx   eax, byte ptr [ecx]                   // 0f b6 01      reg->index
        lea     edx, [eax*8]                          // 8d 14 c5 00 00 00 00
        sub     edx, eax                              // 2b d0         eax*7
        mov     eax, dword ptr [ecx + 4]              // 8b 41 04      reg->slots
        lea     ecx, [eax + edx*4]                    // 8d 0c 90      slot
        push    8                                     // 6a 08
        call    FUN_00417ab0                          // e8 ?? ?? ?? ??
        test    eax, eax                              // 85 c0
        jz      null_path                             // 74 1a

        mov     ecx, dword ptr [esp + 8]              // 8b 4c 24 08   payload
        mov     dword ptr [eax], offset data_00f64960 // c7 00 ?? ?? ?? ??
        mov     dword ptr [eax + 4], ecx              // 89 48 04
        mov     ecx, dword ptr [esi + 0xc]            // 8b 4e 0c      this->sink
        push    eax                                   // 50
        call    FUN_0043c2d0                          // e8 ?? ?? ?? ??
        pop     esi                                   // 5e
        ret     4                                     // c2 04 00

    null_path:
        mov     ecx, dword ptr [esi + 0xc]            // 8b 4e 0c      this->sink
        xor     eax, eax                              // 33 c0
        push    eax                                   // 50
        call    FUN_0043c2d0                          // e8 ?? ?? ?? ??
        pop     esi                                   // 5e
        ret     4                                     // c2 04 00
    }
}
