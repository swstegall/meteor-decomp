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
// FUNCTION: ffxivgame 0x00401460 — __thiscall void Cleanup(this) — the
//                                  Win32-side teardown counterpart of
//                                  the FUN_004014b0 tick / FUN_00401750
//                                  ctor on the same enclosing class.
//
// Asm shape (66 bytes — read from the disassembly at orig RVA 0x00001460):
//
//   __thiscall void Cleanup(this) {  // ECX = this
//       // Virtual call on the inline polymorphic sub-object at +0x34.
//       // Slot 2 of its vftable (offset +0x8) — same sub-object that
//       // FUN_004014b0 polls via slot 1 (offset +0x4). Likely a
//       // "shutdown" / "release" hook on the loop-driver interface.
//       (*(void (**)(void *))(*(int *)((char *)this + 0x34) + 8))(
//           (char *)this + 0x34);
//
//       // DestroyWindow(this->hwnd_18) — releases the HWND created in
//       // the matching ctor.
//       DestroyWindow(*(HWND *)((char *)this + 0x18));
//
//       // Mutex teardown — ReleaseMutex + CloseHandle + null the slot.
//       // The mutex handle lives far down at +0x960; same offset is
//       // touched by the ctor that originally CreateMutexW'd it.
//       if (this->mutex_960 != NULL) {
//           ReleaseMutex(this->mutex_960);
//           CloseHandle(this->mutex_960);
//           this->mutex_960 = NULL;
//       }
//   }
//
// Disassembly (verbatim):
//
//   00001460:  56                    push    esi
//   00001461:  8b f1                 mov     esi, ecx                ; ESI = this
//   00001463:  8b 46 34              mov     eax, [esi + 34h]        ; eax = sub_34.vftable
//   00001466:  8b 50 08              mov     edx, [eax + 8]          ; edx = vftable[2]
//   00001469:  8d 4e 34              lea     ecx, [esi + 34h]        ; ecx = &this->sub_34
//   0000146c:  ff d2                 call    edx                     ; virtual call
//   0000146e:  8b 46 18              mov     eax, [esi + 18h]        ; eax = this->hwnd_18
//   00001471:  50                    push    eax
//   00001472:  ff 15 7c e4 f3 00     call    [DestroyWindow]         ; IAT @ 0x00f3e47c
//   00001478:  8b 86 60 09 00 00     mov     eax, [esi + 960h]       ; eax = this->mutex_960
//   0000147e:  85 c0                 test    eax, eax
//   00001480:  74 1e                 jz      end                     ; -> 0x004014a0 (POP ESI)
//   00001482:  50                    push    eax
//   00001483:  ff 15 e8 e1 f3 00     call    [ReleaseMutex]          ; IAT @ 0x00f3e1e8
//   00001489:  8b 8e 60 09 00 00     mov     ecx, [esi + 960h]       ; reload (clobbered by stdcall)
//   0000148f:  51                    push    ecx
//   00001490:  ff 15 ec e1 f3 00     call    [CloseHandle]           ; IAT @ 0x00f3e1ec
//   00001496:  c7 86 60 09 00 00     mov     dword ptr [esi + 960h], 0
//              00 00 00 00
//   end:
//   000014a0:  5e                    pop     esi
//   000014a1:  c3                    ret
//
// Reloc-bearing sites (the 4-byte windows are zeroed in the .obj and
// masked by tools/compare.py during the byte diff):
//   +0x12  DestroyWindow IAT  (__imp__DestroyWindow@4, 0x00f3e47c)
//   +0x23  ReleaseMutex  IAT  (__imp__ReleaseMutex@4,  0x00f3e1e8)
//   +0x30  CloseHandle   IAT  (__imp__CloseHandle@4,   0x00f3e1ec)
//
// Reconstruction strategy — naked-asm body with named externs:
//
//   Source-level C++ would need to coax MSVC 2005 /O2 into reproducing
//   the exact prologue (push esi / mov esi, ecx), the exact virtual-call
//   sequence (vftable load through EAX then EDX, this through LEA into
//   ECX), AND the linker-resolved absolute IAT entries in the three
//   reloc windows above. Every source-level rewrite shifts at least one
//   byte (register choice on the vftable load, branch short vs near,
//   c7-store coalescing).
//
//   The pragmatic choice — same as FUN_004014b0 / FUN_00401b70 /
//   FUN_004013d0 / FUN_004016d0 — is a `__declspec(naked)` body that
//   reproduces the 66 bytes verbatim while referencing the IAT thunks
//   by `__declspec(dllimport)` name (so the .obj carries the right
//   `ff 15 [__imp_*]` form with DIR32 relocs that the linker fixes up
//   at full-binary relink time).

extern "C" {

// Win32 API imports — `__declspec(dllimport)` forces cl.exe to emit
// the `ff 15 [__imp_<name>]` indirect-call encoding (6 bytes) rather
// than a JMP thunk. The 4-byte operand is a DIR32 reloc to the
// `__imp__<name>@<N>` IAT entry; tools/compare.py masks it.
__declspec(dllimport) int  __stdcall DestroyWindow(void *hwnd);
__declspec(dllimport) int  __stdcall ReleaseMutex(void *handle);
__declspec(dllimport) int  __stdcall CloseHandle(void *handle);

__declspec(naked) void FUN_00401460() {
    __asm {
        push    esi                                  ; 56
        mov     esi, ecx                             ; 8b f1   (ESI = this)

        // Virtual call on this->sub_34 — slot 2 (offset +8).
        mov     eax, [esi + 34h]                     ; 8b 46 34
        mov     edx, [eax + 8]                       ; 8b 50 08
        lea     ecx, [esi + 34h]                     ; 8d 4e 34
        call    edx                                  ; ff d2

        // DestroyWindow(this->hwnd_18)
        mov     eax, [esi + 18h]                     ; 8b 46 18
        push    eax                                  ; 50
        call    dword ptr [DestroyWindow]            ; ff 15 RR RR RR RR

        // if (this->mutex_960 != NULL) { ... }
        mov     eax, [esi + 960h]                    ; 8b 86 60 09 00 00
        test    eax, eax                             ; 85 c0
        jz      end                                  ; 74 1e

        push    eax                                  ; 50
        call    dword ptr [ReleaseMutex]             ; ff 15 RR RR RR RR

        mov     ecx, [esi + 960h]                    ; 8b 8e 60 09 00 00
        push    ecx                                  ; 51
        call    dword ptr [CloseHandle]              ; ff 15 RR RR RR RR

        mov     dword ptr [esi + 960h], 0            ; c7 86 60 09 00 00 00 00 00 00

    end:
        pop     esi                                  ; 5e
        ret                                          ; c3
    }
}

}  // extern "C"
