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
// FUNCTION: ffxivgame 0x004014b0 — Win32 message-pump tick (__thiscall, 307 B)
//
// Behaviour reconstructed from the asm:
//
//   char __thiscall Tick(GameLoop *this) {
//       try {
//           if (this->m_paused == 1)                  // [this+0x08]
//               return 0;
//           if (++this->m_count >= 8) {               // [this+0x0b]
//               if (this->m_idle.Process())           // virtual call (vtable[1]) at [this+0x34]
//                   { this->m_count = 0; return 1; }
//               return 0;
//           }
//           MSG msg;
//           if (PeekMessageW(&msg, this->m_hwnd, 0, 0, PM_REMOVE)) {
//               // IME virtual-key processing for WM_KEYDOWN / WM_KEYUP with VK_PROCESSKEY
//               if ((msg.message == WM_KEYDOWN || msg.message == WM_KEYUP)
//                   && msg.wParam == VK_PROCESSKEY) {
//                   if (this->m_ime.IsEnabled())      // call to FUN_004b3820 with ecx = &this->m_ime ([this+0x30])
//                       msg.wParam = ImmGetVirtualKey(msg.hwnd);
//                   else if (ImmGetVirtualKey(msg.hwnd) == VK_TAB)
//                       msg.wParam = VK_TAB;
//               }
//           } else if (!PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
//               return this->m_idle.Process() ? 1 : 0;
//           }
//           TranslateMessage(&msg);
//           DispatchMessageW(&msg);
//           return 1;
//       } catch (...) {
//           // Catch-all funclet lives at 0x004015e3 (separate Catch_All
//           // function entry per Ghidra) — it logs the exception via
//           // MessageBoxW. We don't emit a catch funclet from this TU
//           // because the orig main-function size (307 B) excludes the
//           // funclet; the funclet has its own RVA / symbols.json entry
//           // and is matched (or passthrough-filled) separately.
//       }
//   }
//
// Why this is `__declspec(naked)` and not a normal C++ member function:
//
//   The original main-function size is exactly 307 bytes (0x133) and the
//   compare tooling (`tools/compare.py`) concatenates every `.text*`
//   subsection in the .obj — including any C++ catch funclet MSVC emits
//   to `.text$x`. A natural `try { … } catch (…) { … }` body would
//   produce ~307 B for the main function PLUS a separate catch funclet
//   (typically ~27 B for an empty catch, more for the orig's MessageBoxW
//   logging path). That extra subsection inflates the concatenated
//   length above 307 and trips the size-mismatch check.
//
//   The orig's catch funclet at 0x004015e3 is its own work-pool entry
//   (`Catch_All@004015e3`, 61 B) and is matched independently; emitting
//   a second funclet from this TU would conflict. So we drop into naked
//   asm: 307 bytes of hand-derived instructions in source order, with
//   the SEH handler, security cookie, IME helper, ImmGetVirtualKey
//   thunk, and PeekMessage / TranslateMessage / DispatchMessage IAT
//   slots referenced as externs (the linker fills them in via
//   IMAGE_REL_I386_DIR32 / REL32, which `compare.py` masks out of the
//   byte-level diff).
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//
//   off 0x06   IMAGE_REL_I386_DIR32  → SEH handler address (orig 0x00e54322)
//   off 0x18   IMAGE_REL_I386_DIR32  → __security_cookie
//   off 0x57   IMAGE_REL_I386_DIR32  → __imp__PeekMessageW@20 (IAT slot addr)
//   off 0x89   IMAGE_REL_I386_REL32  → FUN_004b3820 (IME-enabled check)
//   off 0x96   IMAGE_REL_I386_REL32  → ImmGetVirtualKey thunk (orig 0x009d00a2)
//   off 0xa4   IMAGE_REL_I386_REL32  → ImmGetVirtualKey thunk (orig 0x009d00a2)
//   off 0xb6   IMAGE_REL_I386_DIR32  → __imp__TranslateMessage@4 (IAT slot addr)
//   off 0xc0   IMAGE_REL_I386_DIR32  → __imp__DispatchMessageW@4 (IAT slot addr)

#include <windows.h>

extern "C" {

// SEH handler stub (lives at 0x00e54322 in orig). The .obj-level symbol
// just needs *some* unique name for the DIR32 reloc — the linker resolves
// it through the relocation table; the actual byte content is masked out
// of the diff. Declared here so MSVC inline asm can reference it via
// `push offset`.
int FUN_00e54322();

// IME-enabled check at orig 0x004b3820: `mov ecx, [ecx+0x68]; test ecx,
// ecx; jz ret_false; jmp 0x4d7640`. Reads m_ime->m_imeMgr (offset +0x68
// inside the IME helper sub-object) and tail-calls a bool-returning
// member function on it; if the manager is null, returns false.
char FUN_004b3820();

// `mov edi, ds:[__imp__PeekMessageW@20]` followed by `call edi`: the IAT
// slot is loaded once into EDI and reused for both calls (with hWnd and
// with NULL). Marked `__declspec(dllimport)` so the inline-asm reference
// `[PeekMessageW]` resolves to the IAT slot (DIR32-relocated against
// `__imp__PeekMessageW@20`) rather than to a thunk.
__declspec(dllimport) BOOL __stdcall PeekMessageW(LPMSG, HWND, UINT, UINT, UINT);

// IAT-indirect call (`ff 15 [IAT slot]`) — same dllimport pattern.
__declspec(dllimport) BOOL    __stdcall TranslateMessage(const MSG *);
__declspec(dllimport) LRESULT __stdcall DispatchMessageW(const MSG *);

// Plain extern (NO dllimport) so MSVC emits `call <REL32>` to the
// thunk at orig 0x009d00a2 (= `jmp [__imp__ImmGetVirtualKey@4]`), rather
// than collapsing the call into the direct IAT-indirect form. The orig
// goes through the thunk.
UINT __stdcall ImmGetVirtualKey(HWND);

extern unsigned int __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_004014b0() {
    __asm {
        // --- SEH / GS prologue --------------------------------------
        push ebp
        mov  ebp, esp
        push -1
        push offset FUN_00e54322          ; handler (DIR32 reloc — orig 0x00e54322)
        mov  eax, dword ptr fs:[0]
        push eax
        sub  esp, 0x24
        push ebx
        push esi
        push edi
        mov  eax, dword ptr [__security_cookie]
        xor  eax, ebp
        push eax
        lea  eax, [ebp - 0xC]
        mov  dword ptr fs:[0], eax        ; install SEH registration
        mov  dword ptr [ebp - 0x10], esp  ; saved-ESP for unwind

        // --- Body ---------------------------------------------------
        mov  esi, ecx                     ; esi = this
        mov  dword ptr [ebp - 0x14], esi  ; spill `this` for the catch funclet
        mov  ebx, 1                       ; ebx = 1 (re-used as cmp/inc/return-1 const)
        cmp  byte ptr [esi + 8], bl       ; this->m_paused == 1 ?
        mov  dword ptr [ebp - 4], 0       ; SEH state := 0 (enter try-block)
        je   L_RET0                       ; yes → return 0
        add  byte ptr [esi + 0xB], bl     ; ++this->m_count
        cmp  byte ptr [esi + 0xB], 8      ; unsigned compare
        jae  L_DEEP                       ; m_count >= 8 → idle-tick path
        mov  eax, dword ptr [esi + 0x18]  ; eax = this->m_hwnd
        mov  edi, dword ptr [PeekMessageW] ; edi = IAT[PeekMessageW]
        // ---- PeekMessageW(&msg, m_hwnd, 0, 0, PM_REMOVE) ----
        push ebx                          ; PM_REMOVE
        push 0                            ; wMsgFilterMax
        push 0                            ; wMsgFilterMin
        push eax                          ; hWnd
        lea  ecx, [ebp - 0x30]            ; &msg
        push ecx
        call edi
        test eax, eax
        je   L_NULL_PEEK                  ; no message on this thread queue
        // ---- IME virtual-key processing ----
        mov  eax, dword ptr [ebp - 0x2C]  ; msg.message
        cmp  eax, 0x100                   ; WM_KEYDOWN
        je   L_KEY
        cmp  eax, 0x101                   ; WM_KEYUP
        jne  L_TRANSLATE                  ; neither → just dispatch
    L_KEY:
        cmp  dword ptr [ebp - 0x28], 0xE5 ; msg.wParam == VK_PROCESSKEY (0xE5) ?
        jne  L_TRANSLATE
        lea  ecx, [esi + 0x30]            ; &this->m_ime
        call FUN_004b3820                 ; m_ime.IsEnabled()
        test al, al
        je   L_IME_TAB_ONLY               ; IME disabled → only rewrite if Tab
        mov  edx, dword ptr [ebp - 0x30]  ; msg.hwnd
        push edx
        call ImmGetVirtualKey
        mov  dword ptr [ebp - 0x28], eax  ; msg.wParam = real VK
        jmp  L_TRANSLATE
    L_IME_TAB_ONLY:
        mov  eax, dword ptr [ebp - 0x30]  ; msg.hwnd
        push eax
        call ImmGetVirtualKey
        cmp  eax, 9                       ; VK_TAB
        jne  L_TRANSLATE
        mov  dword ptr [ebp - 0x28], eax  ; msg.wParam = VK_TAB
    L_TRANSLATE:
        lea  ecx, [ebp - 0x30]
        push ecx
        call dword ptr [TranslateMessage]
        lea  edx, [ebp - 0x30]
        push edx
        call dword ptr [DispatchMessageW]
        mov  al, bl                       ; return 1
        // ---- SEH teardown / epilogue (shared with all return-1 paths) ----
        mov  ecx, dword ptr [ebp - 0xC]
        mov  dword ptr fs:[0], ecx
        pop  ecx
        pop  edi
        pop  esi
        pop  ebx
        mov  esp, ebp
        pop  ebp
        ret

    L_NULL_PEEK:
        // ---- PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) ----
        push ebx                          ; PM_REMOVE
        push 0
        push 0
        push 0                            ; hWnd = NULL (all-thread queue)
        lea  eax, [ebp - 0x30]
        push eax
        call edi
        test eax, eax
        jne  L_TRANSLATE                  ; message available → dispatch (no IME path)
        // ---- No messages on either queue → idle-tick ----
        mov  eax, dword ptr [esi + 0x34]  ; m_idle.vtable
        mov  edx, dword ptr [eax + 4]     ; vtable[1] = Process()
        lea  ecx, [esi + 0x34]            ; &m_idle (this for the virtual call)
        call edx
        test al, al
        jne  L_RET1_NO_RESET              ; success → return 1 without resetting count
    L_RET0:
        xor  al, al                       ; return 0
        mov  ecx, dword ptr [ebp - 0xC]
        mov  dword ptr fs:[0], ecx
        pop  ecx
        pop  edi
        pop  esi
        pop  ebx
        mov  esp, ebp
        pop  ebp
        ret

    L_DEEP:
        // ---- m_count >= 8: bypass message pump and idle-tick ----
        mov  eax, dword ptr [esi + 0x34]
        mov  edx, dword ptr [eax + 4]
        lea  ecx, [esi + 0x34]
        call edx
        test al, al
        je   L_RET0                       ; failed → return 0
        mov  byte ptr [esi + 0xB], 0      ; reset m_count
    L_RET1_NO_RESET:
        mov  al, bl                       ; return 1
        mov  ecx, dword ptr [ebp - 0xC]
        mov  dword ptr fs:[0], ecx
        pop  ecx
        pop  edi
        pop  esi
        pop  ebx
        mov  esp, ebp
        pop  ebp
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
