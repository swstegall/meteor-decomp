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
// FUNCTION: ffxivgame 0x00406133 — Win32 resource → buffer loader (96 B)
//
// Loads a Windows PE resource via the standard
// LoadResource / SizeofResource / LockResource triad, then funnels the
// raw bytes into a member-function buffer object (passed via the 5th
// stack slot, accessed at [esp+0x18] after the prologue / consumed-arg
// shuffle). The buffer object's three methods at RVAs 0x4061e0, 0x406280
// and 0x6ce260 (all __thiscall) appear to be a size-reserve, a head-set
// and a tail-reserve respectively, with the final memcpy filling the
// reserved bytes from the locked resource pointer.
//
// Reconstruction strategy — naked __asm:
//
//   The function has two characteristics that defeat any source-level
//   reconstruction:
//
//     (a) The prologue's `push ebx ; push esi ; push edi` doubles as
//         arg-marshalling for the very first Win32 call:
//           LoadResource(hModule = caller's edi, hResInfo = caller's esi)
//         consumes the saved edi and esi via its `ret 8` epilogue. The
//         subsequent `push esi ; push edi ; call SizeofResource` and
//         `push ebx ; call LockResource` likewise consume the value
//         in register (esi/edi/ebx) at that point. Only the original
//         saved ebx survives to the function's epilogue.
//
//     (b) The function has zero direct callers in the binary (no `e8`
//         rel32 or 4-byte literal references) — it's an unreachable
//         template leftover, so we cannot infer the calling convention
//         from a caller site.
//
//   MSVC will never emit a body where `push ebx` simultaneously serves
//   as a callee-save and as a __stdcall arg-push three instructions
//   later — it's a hand-tuned shape (likely the result of an inline
//   helper getting partially folded into a containing function during
//   /O2 + /Gy). A `__declspec(naked)` body lets us reproduce the 96
//   bytes verbatim while letting the linker fill in the 7 reloc-bearing
//   operands:
//
//     +0x03  CALL [LoadResource]      (IAT-indirect, ff 15 RR RR RR RR)
//     +0x11  CALL [SizeofResource]    (IAT-indirect, ff 15 RR RR RR RR)
//     +0x1f  CALL [LockResource]      (IAT-indirect, ff 15 RR RR RR RR)
//     +0x32  CALL FUN_004061e0        (rel32, e8 RR RR RR RR)
//     +0x3c  CALL FUN_00406280        (rel32, e8 RR RR RR RR)
//     +0x47  CALL FUN_006ce260        (rel32, e8 RR RR RR RR)
//     +0x4d  CALL memcpy              (rel32, e8 RR RR RR RR)
//
//   tools/compare.py masks all 7 windows in its diff.
//
// Asm (96 bytes, from orig RVA 0x6133):
//
//   406133:  53                    push  ebx                ; +arg2 for LR
//   406134:  56                    push  esi                ; +arg2 for LR
//   406135:  57                    push  edi                ; +arg1 for LR
//   406136:  ff 15 RR RR RR RR     call  [LoadResource]     ; ret 8 → cleans esi,edi
//   40613c:  8b d8                 mov   ebx, eax           ; ebx = hResData
//   40613e:  85 db                 test  ebx, ebx
//   406140:  74 4b                 je    fail               ; bail if NULL
//   406142:  56                    push  esi                ; arg2 for SR
//   406143:  57                    push  edi                ; arg1 for SR
//   406144:  ff 15 RR RR RR RR     call  [SizeofResource]   ; ret 8
//   40614a:  8b f0                 mov   esi, eax           ; esi = size
//   40614c:  85 f6                 test  esi, esi
//   40614e:  76 3d                 jbe   fail               ; bail if zero/negative
//   406150:  53                    push  ebx                ; arg for LkR
//   406151:  ff 15 RR RR RR RR     call  [LockResource]     ; ret 4
//   406157:  8b d8                 mov   ebx, eax           ; ebx = locked ptr
//   406159:  85 db                 test  ebx, ebx
//   40615b:  74 30                 je    fail               ; bail if NULL
//   40615d:  8b 7c 24 18           mov   edi, [esp+0x18]    ; edi = buffer obj `this`
//   406161:  56                    push  esi                ; arg
//   406162:  8b cf                 mov   ecx, edi
//   406164:  e8 RR RR RR RR        call  FUN_004061e0       ; this->reserve(size)
//   406169:  6a 00                 push  0
//   40616b:  56                    push  esi
//   40616c:  8b cf                 mov   ecx, edi
//   40616e:  e8 RR RR RR RR        call  FUN_00406280       ; this->set_head(size, 0)
//   406173:  56                    push  esi
//   406174:  53                    push  ebx
//   406175:  6a 00                 push  0
//   406177:  8b cf                 mov   ecx, edi
//   406179:  e8 RR RR RR RR        call  FUN_006ce260       ; eax = this->reserve_tail(0)
//   40617e:  50                    push  eax                ; → memcpy dest
//   40617f:  e8 RR RR RR RR        call  memcpy             ; memcpy(dest, src=ebx, n=esi)
//   406184:  83 c4 0c              add   esp, 0xC           ; cdecl cleanup (eax,ebx,esi)
//   406187:  5b                    pop   ebx                ; restore saved ebx
//   406188:  5f                    pop   edi                ; (scrambled — see note)
//   406189:  b0 01                 mov   al, 1              ; success
//   40618b:  5e                    pop   esi
//   40618c:  c3                    ret
//   40618d: fail:
//   40618d:  5b                    pop   ebx
//   40618e:  5f                    pop   edi
//   40618f:  32 c0                 xor   al, al             ; failure
//   406191:  5e                    pop   esi
//   406192:  c3                    ret

extern "C" {

// Windows-API imports through the IAT — referenced as
// `call dword ptr [Symbol]` → `ff 15 RR RR RR RR` with an IAT relocation.
__declspec(dllimport) void * __stdcall LoadResource(void *hModule, void *hResInfo);
__declspec(dllimport) unsigned long __stdcall SizeofResource(void *hModule, void *hResInfo);
__declspec(dllimport) void * __stdcall LockResource(void *hResData);

// Internal callees — declared as bare `extern "C" void f()` so the
// CALL emits a REL32 relocation that the linker resolves to the right
// RVA in the relinked output PE.
void FUN_004061e0();   // __thiscall member fn at 0x004061e0 (buffer reserve)
void FUN_00406280();   // __thiscall member fn at 0x00406280 (buffer head-set)
void FUN_006ce260();   // __thiscall member fn at 0x006ce260 (buffer tail-reserve)

// CRT memcpy at 0x009d4600 — declared `void f()` so the call site is a
// bare REL32 without any C++ name-mangling or signature interference.
void memcpy();

__declspec(naked) void FUN_00406133() {
    __asm {
        push    ebx
        push    esi
        push    edi
        call    dword ptr [LoadResource]
        mov     ebx, eax
        test    ebx, ebx
        je      fail
        push    esi
        push    edi
        call    dword ptr [SizeofResource]
        mov     esi, eax
        test    esi, esi
        jbe     fail
        push    ebx
        call    dword ptr [LockResource]
        mov     ebx, eax
        test    ebx, ebx
        je      fail
        mov     edi, [esp+18h]
        push    esi
        mov     ecx, edi
        call    FUN_004061e0
        push    0
        push    esi
        mov     ecx, edi
        call    FUN_00406280
        push    esi
        push    ebx
        push    0
        mov     ecx, edi
        call    FUN_006ce260
        push    eax
        call    memcpy
        add     esp, 0Ch
        pop     ebx
        pop     edi
        mov     al, 1
        pop     esi
        ret
    fail:
        pop     ebx
        pop     edi
        xor     al, al
        pop     esi
        ret
    }
}

}  // extern "C"
