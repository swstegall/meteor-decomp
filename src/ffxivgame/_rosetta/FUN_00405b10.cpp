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
// FUNCTION: ffxivgame 0x00405b10 — write `<userdir>\config.rgn` on disk
//                                  (one-shot region-config persistence,
//                                   375 B / 0x177, EH3-SEH + /GS wrapped).
//
// Behaviour reconstructed from the asm (Ghidra pseudo-C at
// build/ghidra-decomp/ffxivgame/00005b10_FUN_00405b10.c agrees on shape):
//
//   void __cdecl FUN_00405b10(unsigned char regionCode) {
//       // Bailout 1: process-wide "config-allowed" pointer is null/false.
//       if (**(char**)0x01323898 == '\0') return;     // [DAT_01323898]
//
//       try {
//           // Local stack-allocated Utf8String-ish path object at [esp+0x2c]
//           // (CRT-style: capacity field at +0x28 = 7, size field at +0x24
//           // = 0, inline 8-byte SSO buffer starting at [esp+0x14] which
//           // begins with a NUL wchar — `66 89 5c 24 14` zeroes the first
//           // two bytes of the inline buffer).
//           PathString pathBuf;                       // ctor: FUN_00445cf0
//           pathBuf.resolveUserDir(0);                // FUN_00405210 — fills
//                                                     // pathBuf from %APPDATA%
//                                                     // (returns char success
//                                                     //  in AL; 0 = bail).
//
//           if (resolveUserDir_ok) {
//               pathBuf.append("\\config.rgn");       // FUN_00448900 — ascii
//                                                     // suffix append.
//               wchar_t wpath[8];                     // [esp+0x10] LPCWSTR out
//                                                     // (4 wchars staging + 4
//                                                     //  scratch — set up by
//                                                     //  FUN_00449000).
//               pathBuf.toWide(&wpath);               // FUN_00449000 — utf8
//                                                     // → utf16 conversion.
//
//               // SSO trick: if capacity (pathBuf.cap @ [esp+0x28]) is
//               // still <= 7, the data lives inline at [esp+0x14]; else
//               // [esp+0x14] holds a heap pointer. Pick the active form
//               // for the LPCWSTR pass to CreateFileW.
//               LPCWSTR lpFileName =
//                   (pathBuf.cap >= 8)
//                       ? *(LPCWSTR *)&pathBuf.data   // heap pointer
//                       : (LPCWSTR)&pathBuf.data;     // inline buffer
//
//               HANDLE h = CreateFileW(lpFileName,
//                                       GENERIC_WRITE,
//                                       FILE_SHARE_READ,
//                                       NULL,
//                                       CREATE_ALWAYS,
//                                       FILE_ATTRIBUTE_NORMAL,
//                                       NULL);
//               if (h != INVALID_HANDLE_VALUE) {
//                   // Build a 5-byte payload on the stack at [esp+0x18]:
//                   //   byte 0: 0x00              (literal NUL)
//                   //   byte 1..3: 0x000000      (the low 3 bytes of regionCode<<24
//                   //                              after the dword write below)
//                   //   byte 4: regionCode       (the parameter, byte-stored)
//                   // The asm packs this as:
//                   //   `mov [esp+0x91], ebx`   (4-byte EBX=0 store at +0x91..+0x94)
//                   //   `mov [esp+0x94], bl`    (overwrites byte +0x94 with 0)
//                   //   `mov [esp+0x98], al`    (regionCode at +0x98)
//                   //   `mov [esp+0x20], ebx`   (dwBytesWritten init)
//                   // The 5-byte WriteFile lpBuffer at &[esp+0x90] reads
//                   // through positions 0x91/0x94/0x95 — see the SSE-style
//                   // 1-byte/4-byte/1-byte mix in the asm for the exact
//                   // staging order.
//                   unsigned char buf[5] = { 0, 0, 0, 0, regionCode };
//                   DWORD nWritten = 0;
//                   WriteFile(h, buf, 5, &nWritten, NULL);
//                   CloseHandle(h);
//               }
//           }
//           pathBuf.~PathString();                    // FUN_00446f50
//       } catch (...) { /* funclet emitted separately by MSVC */ }
//   }
//
// Calling convention: __cdecl (the one byte-param goes through the
// stack; AL is loaded from `[esp+0x9c]` mid-body which is exactly
// `regionCode` after pushing two args). No `ret N` — caller cleans the
// stack.
//
// SEH / GS frame after the prologue (esp-relative, prologue PUSHes
// dropped down — [esp+0x8c] is the SEH chain link slot):
//   [esp + 0x00]  saved GS cookie XOR'd with current ESP (post-push)
//   [esp + 0x04]  saved ESI
//   [esp + 0x08]  saved EBX
//   [esp + 0x0c .. esp + 0x88]  body locals (PathString, MSG-bytes-staging,
//                                inline wchar buffer, etc.)
//   [esp + 0x7c]  cookie XOR ESP snapshot (for __security_check_cookie)
//   [esp + 0x8c]  prev FS:[0]   (push'd as second-from-bottom of prologue)
//   [esp + 0x90]  scope-table   (push'd as `push 0x00e548cc`)
//   [esp + 0x94]  initial state (push'd as -1)
//   [esp + 0x98]  return addr
//
// Why naked-asm and not a natural source-level rewrite:
//
//   The orig 375 bytes are entirely shape-constrained: an MSVC-2005 /GS
//   /EHs- prologue (PUSH -1 / PUSH scope / PUSH FS:[0] / SUB ESP / two
//   cookie XOR'd-with-ESP stashes / register pushes / FS:[0] install),
//   a 4-state SSO-aware Utf8String body, three IAT-indirect Win32 calls
//   in textbook MSVC order, and a SEH teardown with /GS check. Every
//   high-level rewrite shifts at least one byte (cookie-stack-offset
//   choice, branch short-vs-near, SSO inline-vs-heap CMOV vs branch,
//   IAT-indirect vs thunked CALL). The pragmatic match — same one
//   FUN_004014b0 / FUN_004013d0 took for their EH-wrapped bodies — is a
//   `__declspec(naked)` body that re-emits the 375 bytes via real MSVC
//   inline asm, with the IAT slots / sub-callees / cookie / scope-table
//   referenced as proper externs (the linker fills them in via DIR32 /
//   REL32, which `tools/compare.py` masks out of the diff).
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//
//   off 0x03   IMAGE_REL_I386_DIR32  → SEH handler stub (orig 0x00e548cc)
//   off 0x15   IMAGE_REL_I386_DIR32  → __security_cookie (orig 0x012ea8b0)
//   off 0x22   IMAGE_REL_I386_DIR32  → __security_cookie (2nd cookie XOR)
//   off 0x37   IMAGE_REL_I386_DIR32  → process-config flag ptr (orig 0x01323898)
//   off 0x4a   IMAGE_REL_I386_REL32  → FUN_00445cf0 (PathString ctor)
//   off 0x75   IMAGE_REL_I386_REL32  → FUN_00405210 (resolveUserDir)
//   off 0x85   IMAGE_REL_I386_DIR32  → "\config.rgn" (orig 0x00f54bb4 .rdata)
//   off 0x8e   IMAGE_REL_I386_REL32  → FUN_00448900 (PathString::append)
//   off 0x9c   IMAGE_REL_I386_REL32  → FUN_00449000 (PathString::toWide)
//   off 0xc2   IMAGE_REL_I386_DIR32  → __imp__CreateFileW@28 (IAT slot)
//   off 0x100  IMAGE_REL_I386_DIR32  → __imp__WriteFile@20 (IAT slot)
//   off 0x107  IMAGE_REL_I386_DIR32  → __imp__CloseHandle@4 (IAT slot)
//   off 0x128  IMAGE_REL_I386_REL32  → FUN_0044d350 (free / deallocate)
//   off 0x150  IMAGE_REL_I386_REL32  → FUN_00446f50 (PathString dtor)
//   off 0x16c  IMAGE_REL_I386_REL32  → __security_check_cookie

#include <windows.h>

extern "C" {

// SEH handler trampoline at 0x00e548cc in orig (push'd as the EH3
// scope-table cookie). Unique symbol for the DIR32 reloc; the actual
// bytes are masked.
int FUN_00e548cc();

// Process-config gate pointer at .data 0x01323898 — points at a single
// byte that gates whether config writes are allowed at all. Declared as
// `char*` so `mov eax, dword ptr [DAT_01323898]` emits the 5-byte `a1`
// (moffs32) load form that the orig uses.
extern char *DAT_01323898;

// ASCII string `\config.rgn` at .rdata 0x00f54bb4. Just a unique symbol
// for the DIR32 reloc; the bytes themselves are masked.
extern char STR_config_rgn[];

// PathString helpers (all __thiscall — ECX is set up by the LEA
// immediately before each call). Plain declarations satisfy the inline
// asm `call <name>` REL32 reloc; the bytes of the rel32 displacement
// are masked out of the diff.
int FUN_00445cf0();   // PathString ctor — initialises the SSO header
int FUN_00405210();   // resolveUserDir(int) — fills pathBuf with %APPDATA%
int FUN_00448900();   // PathString::append(const char *) — ascii suffix
int FUN_00449000();   // PathString::toWide(wchar_t *out) — utf8 → utf16
int FUN_0044d350();   // operator delete[] / aligned-free helper
int FUN_00446f50();   // PathString dtor

// MSVC 2005 /GS runtime — declared the standard way so the inline asm
// can reference them by name.
extern unsigned int __security_cookie;
void __fastcall __security_check_cookie(unsigned int);

// Win32 IAT slots — `__declspec(dllimport)` so MSVC emits IAT-indirect
// `ff 15 [__imp_<fn>]` rather than thunked `e8 <REL32>` to a stub.
__declspec(dllimport) HANDLE __stdcall CreateFileW(LPCWSTR, DWORD, DWORD,
                                                    LPSECURITY_ATTRIBUTES,
                                                    DWORD, DWORD, HANDLE);
__declspec(dllimport) BOOL   __stdcall WriteFile(HANDLE, LPCVOID, DWORD,
                                                  LPDWORD, LPOVERLAPPED);
__declspec(dllimport) BOOL   __stdcall CloseHandle(HANDLE);

} // extern "C"

extern "C" __declspec(naked) void FUN_00405b10() {
    __asm {
        // --- EH3-SEH + /GS prologue ---------------------------------
        push        -1                                  ; 6a ff
        push        offset FUN_00e548cc                 ; 68 <DIR32>
        mov         eax, dword ptr fs:[0]               ; 64 a1 00 00 00 00
        push        eax                                 ; 50  (prev fs:[0] → SEH chain)
        sub         esp, 0x80                           ; 81 ec 80 00 00 00
        mov         eax, __security_cookie              ; a1 <DIR32>  (5-byte moffs32 form)
        xor         eax, esp                            ; 33 c4
        mov         dword ptr [esp + 0x7c], eax         ; 89 44 24 7c
        push        ebx                                 ; 53
        push        esi                                 ; 56
        mov         eax, __security_cookie              ; a1 <DIR32>  (2nd cookie XOR)
        xor         eax, esp                            ; 33 c4
        push        eax                                 ; 50
        lea         eax, [esp + 0x8c]                   ; 8d 84 24 8c 00 00 00
        mov         dword ptr fs:[0], eax               ; 64 a3 00 00 00 00  (install SEH frame)

        // --- Body ---------------------------------------------------
        mov         eax, dword ptr [DAT_01323898]       ; a1 <DIR32>  (config-gate ptr load)
        xor         ebx, ebx                            ; 33 db       (ebx = 0, used as bl-source for byte zeros)
        cmp         byte ptr [eax], bl                  ; 38 18       (*gate == 0 ?)
        je          L_EPILOG                            ; 0f 84 0f 01 00 00  (yes → SEH teardown)

        // pathBuf @ [esp+0x2c] — PathString ctor.
        lea         ecx, [esp + 0x2c]                   ; 8d 4c 24 2c
        call        FUN_00445cf0                        ; e8 <REL32>
        mov         dword ptr [esp + 0x94], ebx         ; 89 9c 24 94 00 00 00  (state := 0 → "ctor done")
        mov         dword ptr [esp + 0x28], 7           ; c7 44 24 28 07 00 00 00  (pathBuf.cap = 7)
        mov         dword ptr [esp + 0x24], ebx         ; 89 5c 24 24            (pathBuf.size = 0)
        mov         word ptr  [esp + 0x14], bx          ; 66 89 5c 24 14         (pathBuf.data[0..1] = 0,0)
        lea         ecx, [esp + 0x2c]                   ; 8d 4c 24 2c            (ECX = &pathBuf)
        push        ebx                                 ; 53                     (arg 2 = 0)
        push        ecx                                 ; 51                     (arg 1 = &pathBuf)
        mov         byte ptr [esp + 0x9c], 1            ; c6 84 24 9c 00 00 00 01  (state := 1 → "filling")
        call        FUN_00405210                        ; e8 <REL32>             (resolveUserDir(&pathBuf, 0))
        add         esp, 8                              ; 83 c4 08
        cmp         al, bl                              ; 3a c3                  (returned false?)
        je          L_AFTER_FILE                        ; 0f 84 87 00 00 00      (yes → skip CreateFileW path)

        push        offset STR_config_rgn               ; 68 <DIR32>             ("\config.rgn")
        lea         ecx, [esp + 0x30]                   ; 8d 4c 24 30            (ECX = &pathBuf, post-push adj.)
        call        FUN_00448900                        ; e8 <REL32>             (pathBuf.append("\config.rgn"))
        lea         edx, [esp + 0x10]                   ; 8d 54 24 10            (&wpath[0] staging slot)
        push        edx                                 ; 52
        lea         ecx, [esp + 0x30]                   ; 8d 4c 24 30
        call        FUN_00449000                        ; e8 <REL32>             (pathBuf.toWide(&wpath))
        cmp         dword ptr [esp + 0x28], 8           ; 83 7c 24 28 08         (SSO threshold check)
        mov         eax, dword ptr [esp + 0x14]         ; 8b 44 24 14            (load heap-ptr OR first 4 bytes of inline)
        jae         L_HAVE_LPFILENAME                   ; 73 04                  (cap >= 8 → eax already = heap ptr)
        lea         eax, [esp + 0x14]                   ; 8d 44 24 14            (cap < 8 → eax = &inline buf)
    L_HAVE_LPFILENAME:
        // CreateFileW(lpFileName=eax, GENERIC_WRITE, FILE_SHARE_READ,
        //             NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
        push        ebx                                 ; 53                     (hTemplateFile = NULL)
        push        0x80                                ; 68 80 00 00 00         (FILE_ATTRIBUTE_NORMAL)
        push        2                                   ; 6a 02                  (CREATE_ALWAYS)
        push        ebx                                 ; 53                     (lpSecurityAttributes = NULL)
        push        1                                   ; 6a 01                  (FILE_SHARE_READ)
        push        0x40000000                          ; 68 00 00 00 40         (GENERIC_WRITE)
        push        eax                                 ; 50                     (lpFileName)
        call        dword ptr [CreateFileW]             ; ff 15 <DIR32>          (IAT slot)
        mov         esi, eax                            ; 8b f0
        cmp         esi, -1                             ; 83 fe ff               (INVALID_HANDLE_VALUE ?)
        je          L_AFTER_FILE                        ; 74 3e

        mov         al, byte ptr [esp + 0x9c]           ; 8a 84 24 9c 00 00 00   (al = parameter regionCode)
        push        ebx                                 ; 53                     (lpOverlapped = NULL)
        lea         ecx, [esp + 0x10]                   ; 8d 4c 24 10            (&nWritten)
        push        ecx                                 ; 51
        push        5                                   ; 6a 05                  (nNumberOfBytesToWrite = 5)
        lea         edx, [esp + 0x8c]                   ; 8d 94 24 8c 00 00 00   (&buf[0])
        push        edx                                 ; 52
        mov         dword ptr [esp + 0x91], ebx         ; 89 9c 24 91 00 00 00   (buf[1..4] = 0,0,0,0 — straddling write)
        push        esi                                 ; 56
        mov         byte ptr  [esp + 0x94], bl          ; 88 9c 24 94 00 00 00   (buf[0] = 0)
        mov         byte ptr  [esp + 0x98], al          ; 88 84 24 98 00 00 00   (buf[4] = regionCode)
        mov         dword ptr [esp + 0x20], ebx         ; 89 5c 24 20            (nWritten = 0)
        call        dword ptr [WriteFile]               ; ff 15 <DIR32>
        push        esi                                 ; 56
        call        dword ptr [CloseHandle]             ; ff 15 <DIR32>

    L_AFTER_FILE:
        mov         eax, dword ptr [esp + 0x28]         ; 8b 44 24 28            (pathBuf.cap)
        cmp         eax, 8                              ; 83 f8 08
        mov         byte ptr [esp + 0x94], bl           ; 88 9c 24 94 00 00 00   (state := 0 → "dtor pending")
        jb          L_SKIP_FREE                         ; 72 14                  (cap < 8 → SSO, no heap free)
        mov         ecx, dword ptr [esp + 0x14]         ; 8b 4c 24 14            (heap pointer)
        push        0xc                                 ; 6a 0c                  (operator delete[] alignment slot)
        lea         eax, [eax + eax * 1 + 2]            ; 8d 44 00 02            (size_in_bytes = cap*2 + 2)
        push        eax                                 ; 50
        push        ecx                                 ; 51
        call        FUN_0044d350                        ; e8 <REL32>             (free heap buffer)
        add         esp, 0xc                            ; 83 c4 0c

    L_SKIP_FREE:
        lea         ecx, [esp + 0x2c]                   ; 8d 4c 24 2c
        mov         dword ptr [esp + 0x28], 7           ; c7 44 24 28 07 00 00 00  (pathBuf.cap = 7)
        mov         dword ptr [esp + 0x24], ebx         ; 89 5c 24 24            (pathBuf.size = 0)
        mov         word ptr  [esp + 0x14], bx          ; 66 89 5c 24 14         (pathBuf.data[0..1] = 0,0)
        mov         dword ptr [esp + 0x94], -1          ; c7 84 24 94 00 00 00 ff ff ff ff  (state := -1 → "destructed")
        call        FUN_00446f50                        ; e8 <REL32>             (PathString dtor)

    L_EPILOG:
        // --- SEH teardown / /GS check / pop / ret -------------------
        mov         ecx, dword ptr [esp + 0x8c]         ; 8b 8c 24 8c 00 00 00   (ecx = saved prev fs:[0])
        mov         dword ptr fs:[0], ecx               ; 64 89 0d 00 00 00 00   (restore SEH chain)
        pop         ecx                                 ; 59                     (drop 2nd cookie XOR slot)
        pop         esi                                 ; 5e
        pop         ebx                                 ; 5b
        mov         ecx, dword ptr [esp + 0x7c]         ; 8b 4c 24 7c            (cookie XOR ESP snapshot)
        xor         ecx, esp                            ; 33 cc                  (unmask cookie)
        call        __security_check_cookie             ; e8 <REL32>
        add         esp, 0x8c                           ; 81 c4 8c 00 00 00
        ret                                             ; c3
    }
}

// vim: ts=4 sts=4 sw=4 et
