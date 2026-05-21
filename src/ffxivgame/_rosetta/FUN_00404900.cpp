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
// FUNCTION: ffxivgame 0x00004900 — `__thiscall` dynamic-array Clear/Release
//                                  (60 B / 0x3c — Ghidra slice; the actual
//                                   function continues 3 more bytes past
//                                   the end into `5e 59 c3` POP ESI/POP ECX/
//                                   RET which Ghidra didn't include because
//                                   it marked `_free` as no-return)
//
// Inspection (read from the orig .text slice at RVA 0x00004900):
//
//   __thiscall void Release(this) — `ECX = this`, no args, no return value.
//
//   Layout (offsets derived from the loads/stores):
//     +0x04   void* m_buffer       (heap-owned element array)
//     +0x08   uint32_t m_count     (element count — passed to the per-element
//                                    teardown helper at +0x4044e0)
//     +0x0c   uint32_t m_capacity  (reserved slots; zeroed alongside the rest)
//
//   Body (matches asm flow byte-for-byte):
//
//     push ecx                       ; allocate 4-byte scratch local at [esp]
//                                    ;   (uninit — its bytes are the caller's
//                                    ;   ECX-at-entry = `this`, which is fine
//                                    ;   because the per-element helper at
//                                    ;   +0x4044e0 takes it as a sixth arg
//                                    ;   that's effectively ignored)
//     push esi
//     mov  esi, ecx                  ; esi = this
//     mov  eax, [esi+4]              ; eax = this->m_buffer
//     test eax, eax
//     jz   skip                      ; if buffer is null, skip the teardown
//     mov  ecx, [esp+4]              ; ecx = scratch local (uninit-ECX-at-entry)
//     mov  edx, [esi+8]              ; edx = this->m_count
//     push ecx                       ; arg4 = scratch (uninit)
//     push esi                       ; arg3 = this
//     push edx                       ; arg2 = this->m_count
//     push eax                       ; arg1 = this->m_buffer
//     call FUN_004044e0              ; 4-arg __cdecl per-element teardown
//     mov  eax, [esi+4]              ; reload this->m_buffer
//     push eax                       ; arg1 = buffer
//     call _free                     ; __cdecl _free(buffer)
//     add  esp, 0x14                 ; clean 5*4 = 0x14 (4 args + free's 1 arg)
//   skip:
//     mov  dword ptr [esi+4],  0     ; this->m_buffer   = nullptr
//     mov  dword ptr [esi+8],  0     ; this->m_count    = 0
//     mov  dword ptr [esi+0xc], 0    ; this->m_capacity = 0
//     ; (POP ESI / POP ECX / RET follow at 0x493c-0x493e but are
//     ;  outside the 0x3c-byte Ghidra-reported function body)
//
//   Headless-Ghidra hint:
//     void FUN_00404900(void) {
//       int in_ECX;
//       if (*(int *)(in_ECX + 4) != 0) {
//         FUN_004044e0(*(int *)(in_ECX + 4),*(undefined4 *)(in_ECX + 8));
//         _free(*(void **)(in_ECX + 4));
//       }
//       *(undefined4 *)(in_ECX + 4) = 0;
//       *(undefined4 *)(in_ECX + 8) = 0;
//       *(undefined4 *)(in_ECX + 0xc) = 0;
//       return;
//     }
//
//   The Ghidra hint truncates both call signatures (FUN_004044e0 actually
//   takes 4 stack args; the trailing two — `this` and the uninitialised
//   scratch local — are an MSVC frame quirk where the compiler reused
//   `push ecx` as a cheap local-slot allocation and then pushed it as
//   a "live" argument because the helper's signature has it).
//
//   Two CALL sites carry REL32 relocations (+0x16 → FUN_004044e0, +0x1f →
//   _free) which `tools/compare.py` masks out of the diff; everything
//   else is raw bytes that the inline assembler reproduces from the
//   mnemonics below.
//
// Reconstruction strategy — naked-asm with mnemonics:
//
//   Source-level C++ here would need to coax MSVC 2005 into emitting the
//   exact `push ecx` scratch-local prologue AND threading that scratch
//   value as the helper's 4th argument — both are byproducts of the
//   original code's register pressure / frame layout, not something the
//   C++ source surface exposes. The sibling FUN_00404860 at the same /
//   similar RVA took the same naked-asm approach for the same reason
//   (a `push ecx`-as-local-slot prologue threading uninitialised ECX
//   into a helper). MSVC's inline assembler picks the short-form
//   encodings the original codegen used (8-bit JZ displacement, 8-bit
//   ESI-disp8 ModR/M, 8-bit ADD ESP imm).

extern "C" void FUN_004044e0();
extern "C" void _free();

extern "C" __declspec(naked) void FUN_00404900() {
    __asm {
        push ecx
        push esi
        mov  esi, ecx
        mov  eax, [esi+4]
        test eax, eax
        jz   skip
        mov  ecx, [esp+4]
        mov  edx, [esi+8]
        push ecx
        push esi
        push edx
        push eax
        call FUN_004044e0
        mov  eax, [esi+4]
        push eax
        call _free
        add  esp, 0x14
    skip:
        mov  dword ptr [esi+4],  0
        mov  dword ptr [esi+8],  0
        mov  dword ptr [esi+0Ch], 0
    }
}
