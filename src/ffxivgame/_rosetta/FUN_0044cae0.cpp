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
// FUNCTION: ffxivgame 0x0004cae0 — EnumWindows callback: find own window
//                                  and post WM_CLOSE (79 B / 0x4f,
//                                  __stdcall 2-arg, no /GS, no frame vars).
//
// This is the EnumWindowsProc passed to EnumWindows by FUN_0044cb30.
// FUN_0044cb30 calls:
//     EnumWindows(FUN_0044cae0, 0x132ce58);
// only when the global flag at 0x132cec8 is non-zero.
//
// Behaviour:
//   For each HWND in the system, get its owning process ID via
//   GetWindowThreadProcessId. Compare against the expected process ID
//   stored at lParam->pid (*(DWORD*)(lParam+8) = ctx.field8). If they
//   match, post WM_CLOSE (0x10) to the window, clear the global
//   "initialised" flag at 0x132cec8, and return FALSE (0) to stop
//   the enumeration. Otherwise return TRUE (1) to continue.
//
// Disassembly (orig RVA 0x0004cae0, 79 bytes):
//
//   0004cae0:  51                   PUSH ECX
//   0004cae1:  56                   PUSH ESI
//   0004cae2:  8b 74 24 0c          MOV ESI,[ESP+0xC]        ; hwnd
//   0004cae6:  8d 44 24 04          LEA EAX,[ESP+4]          ; &pid (old ECX slot)
//   0004caea:  50                   PUSH EAX                 ; arg2: &pid
//   0004caeb:  56                   PUSH ESI                 ; arg1: hwnd
//   0004caec:  c7 44 24 0c 00000000 MOV dword ptr [ESP+0xC],0 ; pid = 0
//   0004caf4:  ff 15 b4 e4 f3 00    CALL [GetWindowThreadProcessId]
//   0004cafa:  8b 4c 24 10          MOV ECX,[ESP+0x10]       ; ctx
//   0004cafe:  8b 51 08             MOV EDX,[ECX+8]          ; ctx->pid
//   0004cb01:  3b 54 24 04          CMP EDX,[ESP+4]          ; vs actual pid
//   0004cb05:  75 1e                JNZ +0x1e                ; mismatch → 0x4cb25
//   0004cb07:  6a 00                PUSH 0                   ; lParam
//   0004cb09:  6a 00                PUSH 0                   ; wParam
//   0004cb0b:  6a 10                PUSH 0x10                ; WM_CLOSE
//   0004cb0d:  56                   PUSH ESI                 ; hwnd
//   0004cb0e:  ff 15 24 e4 f3 00    CALL [PostMessageA]
//   0004cb14:  c7 05 c8ce3201 00000000  MOV dword ptr [g_initialized],0
//   0004cb1e:  33 c0                XOR EAX,EAX              ; return FALSE
//   0004cb20:  5e                   POP ESI
//   0004cb21:  59                   POP ECX
//   0004cb22:  c2 08 00             RET 8
//   0004cb25:  b8 01000000          MOV EAX,1                ; return TRUE
//   0004cb2a:  5e                   POP ESI
//   0004cb2b:  59                   POP ECX
//   0004cb2c:  c2 08 00             RET 8
//
// Frame idiom: PUSH ECX allocates the 4-byte `pid` local (ECX not otherwise
// needed); MSVC 2005 /O2 uses this form when the only local is a single DWORD
// passed by address to an outparam. Matched by POP ECX in the epilogue.
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x14  GetWindowThreadProcessId IAT (4 B DIR32)
//   +0x2e  PostMessageA             IAT (4 B DIR32)
//   +0x36  g_initialized            DIR32 (4 B)

extern "C" {

__declspec(dllimport) unsigned long __stdcall GetWindowThreadProcessId(void *hWnd, unsigned long *lpdwProcessId);
__declspec(dllimport) int __stdcall PostMessageA(void *hWnd, unsigned int Msg, unsigned int wParam, unsigned int lParam);

extern unsigned int g_initialized;   // @ 0x0132cec8

extern "C" __declspec(naked) void FUN_0044cae0() {
    __asm {
        push    ecx                              // 51  allocate local DWORD (pid)
        push    esi                              // 56  save ESI
        mov     esi, dword ptr [esp + 0xc]       // 8b 74 24 0c  ESI = hwnd (arg1)
        lea     eax, dword ptr [esp + 4]         // 8d 44 24 04  EAX = &pid
        push    eax                              // 50  arg2: &pid
        push    esi                              // 56  arg1: hwnd
        mov     dword ptr [esp + 0xc], 0         // c7 44 24 0c 00000000  pid = 0
        call    dword ptr [GetWindowThreadProcessId]  // ff 15 [DIR32]
        mov     ecx, dword ptr [esp + 0x10]      // 8b 4c 24 10  ECX = ctx (arg2)
        mov     edx, dword ptr [ecx + 8]         // 8b 51 08  EDX = ctx->pid
        cmp     edx, dword ptr [esp + 4]         // 3b 54 24 04  vs actual pid
        jne     not_equal                        // 75 1e
        push    0                                // 6a 00  lParam = 0
        push    0                                // 6a 00  wParam = 0
        push    0x10                             // 6a 10  WM_CLOSE
        push    esi                              // 56  hwnd
        call    dword ptr [PostMessageA]         // ff 15 [DIR32]
        mov     dword ptr [g_initialized], 0     // c7 05 [DIR32] 00000000
        xor     eax, eax                         // 33 c0  return FALSE
        pop     esi                              // 5e
        pop     ecx                              // 59
        ret     8                                // c2 08 00
    not_equal:
        mov     eax, 1                           // b8 01000000  return TRUE
        pop     esi                              // 5e
        pop     ecx                              // 59
        ret     8                                // c2 08 00
    }
}

}  // extern "C"
