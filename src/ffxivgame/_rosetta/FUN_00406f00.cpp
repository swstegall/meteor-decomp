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
// FUNCTION: ffxivgame 0x00006f00 — __thiscall thread-safe stats-accumulator
//                                  Record(int value, const char *name).
//                                  154 B / 0x9a, ret 8.
//
// __thiscall void Record(this, int value, const char *name):
//   ECX = this (a 0x4018-byte "stats table" with these fields):
//     +0x0000  pad/unused word (entry 0 is unused — FUN_00406ea0 scans
//              from index 1)
//     +0x0004  Slot slots[128];           // 128 entries, 64 B each
//                  // Per-slot layout (inferred from the init writes):
//                  //   +0x00  char  name[0x33];           // strncpy'd
//                  //   +0x33  char  nul;                  // forced to 0
//                  //   +0x34  int   _zero;                // init to 0
//                  //   +0x38  int   accumulator;          // += value
//                  //   +0x3c  int   counter_a;            // += 1
//                  //   +0x40  int   counter_b;            // += 1
//                  //   (the LEA at +0x6f7c writes through "base+4" so
//                  //   the three live stats fields are at slot+0x34,
//                  //   slot+0x38, slot+0x3c — see the asm comments
//                  //   below for the exact bytes.)
//     +0x2004  int   count;                // number of slots in use, ≤128
//     +0x2008  ... (gap, ~0x2004 bytes — unused by this function)
//     +0x400c  CRITICAL_SECTION cs;        // guards the whole structure
//
// Behaviour (recovered from asm @ 0x00006f00):
//
//   void Stats::Record(int value, const char *name) {
//       EnterCriticalSection(&this->cs);
//       int idx = this->FindSlot(name);         // FUN_00406ea0
//       if (idx < 0) {
//           if (this->count < 0x80) {
//               idx = this->count;
//               char *slot = (char *)this + idx * 64 + 4;
//               strncpy(slot, name, 0x33);          // FUN_009d5540
//               slot[0x33] = 0;
//               *(int *)(slot + 0x34) = 0;
//               *(int *)(slot + 0x38) = 0;
//               *(int *)(slot + 0x3c) = 0;
//               this->count = idx + 1;
//           }
//       }
//       if (idx >= 0) {
//           char *p = (char *)this + idx * 64;
//           *(int *)(p + 0x38) += value;
//           *(int *)(p + 0x3c) += 1;
//           *(int *)(p + 0x40) += 1;
//       }
//       LeaveCriticalSection(&this->cs);
//   }
//
// CALL targets (all REL32/IAT, wildcarded by tools/compare.py):
//   +0x12   CALL [ext_f3e16c]   — kernel32!EnterCriticalSection
//   +0x1f   CALL FUN_00406ea0   — find slot by name (returns idx ≥ 0,
//                                 0 for NULL name, or -1 if not found)
//   +0x55   CALL FUN_009d5540   — strncpy (slot, name, 0x33)
//   +0x8c   CALL [ext_f3e168]   — kernel32!LeaveCriticalSection
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The orig has two unusual codegen idioms that source-level C++ at
//   /O2 will not reliably reproduce:
//
//   (a) The CS pointer `&this->cs` is materialised once into ESI in
//       the prologue, pushed for EnterCriticalSection, and then saved
//       into a stack-resident local slot (the `PUSH ECX` reserve-local
//       at offset 0). The init path overwrites ESI with a fresh slot
//       pointer, then reloads ESI from that local immediately before
//       the `ADD ESP, 0xC` cleanup so the post-update path can PUSH it
//       again for LeaveCriticalSection. MSVC's register-allocator will
//       not spontaneously spill a non-volatile register through a
//       stack home unless the source carefully forces it.
//
//   (b) The two +1 increments at offsets +0x3c and +0x40 are emitted
//       as two `ADD r/m32, ECX` (with ECX held at 1), not as two
//       `INC` instructions and not as a fused 64-bit increment. The
//       LEA pivots through `[EDI + EBP + 4]` so both ADDs share the
//       same base register — a pattern MSVC's CSE/strength-reduction
//       won't pick unless the source spells it out very specifically.
//
//   Both observations land in the same place as the sibling rosetta
//   shells in this directory (FUN_00401b70, FUN_004061e0) — at /O2
//   the source-level lowering converges on a PARTIAL by reordering
//   one or two callee-saved slots vs. orig. Per the project
//   convention for ffxivgame, fall back to naked asm whose .text
//   bytes match orig modulo the four reloc windows.

extern "C" {
    // .idata — IAT slots (DIR32 relocs). Declared as plain int so MASM
    // emits `call dword ptr [ext_f3eXXX]` as `ff 15 ?? ?? ?? ??`.
    extern int ext_f3e16c;   // kernel32!EnterCriticalSection
    extern int ext_f3e168;   // kernel32!LeaveCriticalSection

    // .text — internal direct-call targets within the binary (REL32).
    int FUN_00406ea0();      // __thiscall int FindSlot(const char *name)
    int FUN_009d5540();      // __cdecl char *strncpy(char *dst, const char *src, size_t n)
}

extern "C" __declspec(naked) void FUN_00406f00() {
    __asm {
        // --- prologue ---------------------------------------------------
        push    ecx                                 // 51         reserve local slot for &cs
        push    ebx                                 // 53
        push    ebp                                 // 55
        push    esi                                 // 56
        mov     ebp, ecx                            // 8b e9      EBP = this
        push    edi                                 // 57
        lea     esi, [ebp + 0x400c]                 // 8d b5 0c 40 00 00   ESI = &this->cs
        push    esi                                 // 56         arg for EnterCS
        mov     dword ptr [esp + 0x14], esi         // 89 74 24 14         save &cs into reserved local
        call    dword ptr [ext_f3e16c]              // ff 15 ?? ?? ?? ??   EnterCriticalSection(&cs)

        // idx = FindSlot(name)
        mov     eax, dword ptr [esp + 0x1c]         // 8b 44 24 1c         EAX = name (arg1)
        push    eax                                 // 50
        mov     ecx, ebp                            // 8b cd               ECX = this
        call    FUN_00406ea0                        // e8 ?? ?? ?? ??
        mov     edi, eax                            // 8b f8               EDI = idx
        xor     ebx, ebx                            // 33 db               EBX = 0
        cmp     edi, ebx                            // 3b fb
        jge     short have_idx                      // 7d 45   -> 0x6f71

        // idx < 0 (not found) — try to add a new slot
        mov     eax, dword ptr [ebp + 0x2004]       // 8b 85 04 20 00 00   EAX = this->count
        cmp     eax, 0x80                           // 3d 80 00 00 00
        jge     short skip_init                     // 7d 34   -> 0x6f6d

        // count < 128 — add new slot
        mov     edx, dword ptr [esp + 0x1c]         // 8b 54 24 1c         EDX = name
        mov     edi, eax                            // 8b f8               EDI = old count = new idx
        mov     ecx, edi                            // 8b cf
        shl     ecx, 6                              // c1 e1 06            ECX = idx * 64
        push    0x33                                // 6a 33               strncpy.n
        lea     esi, [ecx + ebp + 4]                // 8d 74 29 04         ESI = slot ptr
        push    edx                                 // 52                  strncpy.src
        add     eax, 1                              // 83 c0 01
        push    esi                                 // 56                  strncpy.dst
        mov     dword ptr [ebp + 0x2004], eax       // 89 85 04 20 00 00   this->count = idx + 1
        call    FUN_009d5540                        // e8 ?? ?? ?? ??      strncpy(slot, name, 0x33)
        mov     byte ptr [esi + 0x33], bl           // 88 5e 33            slot[0x33] = 0
        mov     dword ptr [esi + 0x34], ebx         // 89 5e 34            slot[0x34..0x38) = 0
        mov     dword ptr [esi + 0x38], ebx         // 89 5e 38            slot[0x38..0x3c) = 0
        mov     dword ptr [esi + 0x3c], ebx         // 89 5e 3c            slot[0x3c..0x40) = 0
        mov     esi, dword ptr [esp + 0x1c]         // 8b 74 24 1c         reload &cs from local
        add     esp, 0xc                            // 83 c4 0c            unwind strncpy args

    skip_init:
        cmp     edi, ebx                            // 3b fb
        jl      short skip_update                   // 7c 1a   -> 0x6f8b

    have_idx:
        // Update slot stats: slot[0x34] += value, slot[0x38] += 1, slot[0x3c] += 1
        mov     ecx, dword ptr [esp + 0x18]         // 8b 4c 24 18         ECX = value (arg0)
        shl     edi, 6                              // c1 e7 06            EDI = idx * 64
        add     dword ptr [edi + ebp + 0x38], ecx   // 01 4c 2f 38         *(int*)(p + 0x38) += value
        lea     eax, [edi + ebp + 4]                // 8d 44 2f 04
        mov     ecx, 1                              // b9 01 00 00 00
        add     dword ptr [eax + 0x38], ecx         // 01 48 38            *(int*)(p + 0x3c) += 1
        add     dword ptr [eax + 0x3c], ecx         // 01 48 3c            *(int*)(p + 0x40) += 1

    skip_update:
        push    esi                                 // 56                  arg for LeaveCS
        call    dword ptr [ext_f3e168]              // ff 15 ?? ?? ?? ??   LeaveCriticalSection(&cs)

        // --- epilogue ---------------------------------------------------
        pop     edi                                 // 5f
        pop     esi                                 // 5e
        pop     ebp                                 // 5d
        pop     ebx                                 // 5b
        pop     ecx                                 // 59
        ret     8                                   // c2 08 00
    }
}
