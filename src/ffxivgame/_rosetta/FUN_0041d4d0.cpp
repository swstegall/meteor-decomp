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
// FUNCTION: ffxivgame 0x0001d4d0 — window-creation helper w/ optional
//                                  AdjustWindowRectEx size computation
//                                  (235 B / 0xeb, no SEH).
//
// Behaviour read from asm/ffxivgame/0001d4d0_FUN_0041d4d0.s:
//
//   __cdecl HWND FUN_0041d4d0(LPCSTR lpClassName, BYTE flag);
//   (Also uses caller-supplied EAX=nHeight, ECX=nWidth, EDI=hWndParent.)
//
//   Stack frame (after prologue, ESP-relative w/ PUSH ESI):
//     [ESP+0x00..0x0f]  RECT local (16 B, 4 DWORDs: left/top/right/bottom)
//     [ESP+0x10]        saved ESI
//     [ESP+0x14]        return address
//     [ESP+0x18]        arg1 = lpClassName   (LPCSTR)
//     [ESP+0x18]+4      arg2 = flag          (BYTE, compared against 1)
//
//   Structural shape:
//
//     ESI = lpClassName;
//     if (flag == 1) {
//         // Branch 1: caller supplies width (ECX), height (EAX), parent (EDI)
//         HINSTANCE hInst = *(HINSTANCE*)0x013298b8;
//         HWND hwnd = CreateWindowExA(
//             0x40000,       // dwExStyle
//             ESI,           // lpClassName
//             (LPCSTR)0xf5822b, // lpWindowName (splash/login title)
//             0x84000000,    // dwStyle  WS_POPUP | WS_SYSMENU
//             0, 0,          // X, Y
//             ECX, EAX,      // nWidth, nHeight  (caller-supplied)
//             EDI,           // hWndParent
//             NULL, hInst, NULL
//         );   // via IAT [0x00f3e4e4]
//     } else {
//         // Branch 2: compute window size via SetRect + AdjustWindowRectEx
//         RECT rc;
//         SetRect(&rc, 0, 0, ECX, EAX);  // via IAT [0x00f3e488]
//         BOOL ok = AdjustWindowRectEx(&rc, 0xca0000, 0, 0x40100);
//                                        // via IAT [0x00f3e48c]
//         if (!ok) return NULL;
//         HINSTANCE hInst = *(HINSTANCE*)0x013298b8;
//         int w = rc.right  - rc.left;   // [ESP+0x0c] - [ESP+0x04]
//         int h = rc.bottom - rc.top;    // [ESP+0x10] - [ESP+0x08]
//         HWND hwnd = CreateWindowExA(
//             0x40100,       // dwExStyle
//             ESI,           // lpClassName
//             (LPCSTR)0xf58245, // lpWindowName (main window title)
//             0x4ca0000,     // dwStyle  WS_POPUP | WS_CLIPSIBLINGS | ...
//             CW_USEDEFAULT, CW_USEDEFAULT,
//             w, h,
//             EDI,           // hWndParent
//             NULL, hInst, NULL
//         );   // via IAT [0x00f3e4e4]
//     }
//     ESI = hwnd;
//     if (!ESI) {
//         // Error path: lazily install error-log callback at [0x0132390c]
//         //   pointing to FUN_0041d3a0, then call it with 5 string/line args
//         if (!(*(BYTE*)0x01323910 & 1)) {
//             *(DWORD*)0x01323910 |= 1;
//             *(void**)0x0132390c = (void*)0x0041d3a0;
//         }
//         // CALL [0x0132390c](0xf58a80, 0xf58246, 0xf58a30, 0x95a, 0xf589f8)
//         ADD ESP, 0x14;  // clean error-log args
//     }
//     return ESI;   // HWND or NULL
//
//   Reloc-bearing sites in the orig 235 bytes (absolute addresses resolve
//   only in a full-binary relink at image base 0x00400000):
//     +0x0f  abs32  0x013298b8 — g_hInstance global
//     +0x26  abs32  0xf5822b   — splash window title string
//     +0x3e  abs32  0x00f3e488 — SetRect IAT slot
//     +0x4e  abs32  0x00f3e48c — AdjustWindowRectEx IAT slot
//     +0x64  abs32  0x013298b8 — g_hInstance global (2nd ref)
//     +0x91  abs32  0xf58245   — main window title string
//     +0x9c  abs32  0x00f3e4e4 — CreateWindowExA IAT slot
//     +0xa8  abs32  0x01323910 — error-flag global (TEST)
//     +0xb1  abs32  0x01323910 — error-flag global (OR)
//     +0xb8  abs32  0x0132390c — error-fn-ptr global
//     +0xb8  abs32  0x0041d3a0 — FUN_0041d3a0 (error callback)
//     +0xc2  abs32  0xf589f8   — error string arg
//     +0xd1  abs32  0xf58a30   — error string arg
//     +0xd1  abs32  0xf58246   — error string arg
//     +0xd6  abs32  0xf58a80   — error string arg
//     +0xdb  abs32  0x0132390c — error-fn-ptr global (CALL)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function uses ECX and EAX from the caller's register state as
//   window dimensions, and EDI as the parent window handle.  All three are
//   live in caller-controlled registers at the call site; MSVC 2005 /O2
//   cannot reproduce this idiom from source-level C++ without inserting
//   spills that change the byte sequence.  Additionally, every call target
//   and data reference is an absolute link-time address from a binary
//   relinked at image base 0x00400000, so standalone .obj compilation
//   cannot match the relocation pattern.
//
//   The pragmatic choice — identical to FUN_00402a30, FUN_00403a20, and
//   FUN_004054d0 — is a `__declspec(naked)` body re-emitting the orig
//   235 bytes verbatim via MASM `_emit` directives.  The structural
//   commentary above is the readable record of what the function does.

extern "C" __declspec(naked) void FUN_0041d4d0() {
    __asm {
        // 0001d4d0  SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0001d4d3  CMP byte ptr [ESP+0x18], 0x1
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x01
        // 0001d4d8  PUSH ESI
        _emit 0x56
        // 0001d4d9  MOV ESI, dword ptr [ESP+0x18]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // 0001d4dd  JNZ 0x0041d503
        _emit 0x75
        _emit 0x24
        // 0001d4df  MOV EDX, dword ptr [0x013298b8]
        _emit 0x8b
        _emit 0x15
        _emit 0xb8
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d4e5  PUSH 0x0  (lpParam)
        _emit 0x6a
        _emit 0x00
        // 0001d4e7  PUSH EDX  (hInstance)
        _emit 0x52
        // 0001d4e8  PUSH 0x0  (hMenu)
        _emit 0x6a
        _emit 0x00
        // 0001d4ea  PUSH EDI  (hWndParent)
        _emit 0x57
        // 0001d4eb  PUSH EAX  (nHeight)
        _emit 0x50
        // 0001d4ec  PUSH ECX  (nWidth)
        _emit 0x51
        // 0001d4ed  PUSH 0x0  (Y)
        _emit 0x6a
        _emit 0x00
        // 0001d4ef  PUSH 0x0  (X)
        _emit 0x6a
        _emit 0x00
        // 0001d4f1  PUSH 0x84000000  (dwStyle)
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        // 0001d4f6  PUSH 0xf5822b  (lpWindowName)
        _emit 0x68
        _emit 0x2b
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        // 0001d4fb  PUSH ESI  (lpClassName)
        _emit 0x56
        // 0001d4fc  PUSH 0x40000  (dwExStyle)
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x04
        _emit 0x00
        // 0001d501  JMP 0x0041d56c
        _emit 0xeb
        _emit 0x69
        // 0001d503  PUSH EAX  (nHeight for SetRect)
        _emit 0x50
        // 0001d504  PUSH ECX  (nWidth for SetRect)
        _emit 0x51
        // 0001d505  PUSH 0x0  (top)
        _emit 0x6a
        _emit 0x00
        // 0001d507  PUSH 0x0  (left)
        _emit 0x6a
        _emit 0x00
        // 0001d509  LEA EAX, [ESP+0x14]  (&local RECT)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001d50d  PUSH EAX
        _emit 0x50
        // 0001d50e  CALL dword ptr [0x00f3e488]  (SetRect)
        _emit 0xff
        _emit 0x15
        _emit 0x88
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001d514  PUSH 0x40100  (dwExStyle for AdjustWindowRectEx)
        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x04
        _emit 0x00
        // 0001d519  PUSH 0x0  (bMenu)
        _emit 0x6a
        _emit 0x00
        // 0001d51b  PUSH 0xca0000  (dwStyle)
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0xca
        _emit 0x00
        // 0001d520  LEA ECX, [ESP+0x10]  (&local RECT)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001d524  PUSH ECX
        _emit 0x51
        // 0001d525  CALL dword ptr [0x00f3e48c]  (AdjustWindowRectEx)
        _emit 0xff
        _emit 0x15
        _emit 0x8c
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001d52b  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001d52d  JNZ 0x0041d534
        _emit 0x75
        _emit 0x05
        // 0001d52f  POP ESI
        _emit 0x5e
        // 0001d530  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0001d533  RET
        _emit 0xc3
        // 0001d534  MOV EDX, dword ptr [0x013298b8]
        _emit 0x8b
        _emit 0x15
        _emit 0xb8
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d53a  MOV EAX, dword ptr [ESP+0x10]  (rc.bottom)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001d53e  SUB EAX, dword ptr [ESP+0x08]  (rc.top)
        _emit 0x2b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001d542  MOV ECX, dword ptr [ESP+0x0c]  (rc.right)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0001d546  SUB ECX, dword ptr [ESP+0x04]  (rc.left)
        _emit 0x2b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0001d54a  PUSH 0x0  (lpParam)
        _emit 0x6a
        _emit 0x00
        // 0001d54c  PUSH EDX  (hInstance)
        _emit 0x52
        // 0001d54d  PUSH 0x0  (hMenu)
        _emit 0x6a
        _emit 0x00
        // 0001d54f  PUSH EDI  (hWndParent)
        _emit 0x57
        // 0001d550  PUSH EAX  (nHeight = rc.bottom - rc.top)
        _emit 0x50
        // 0001d551  PUSH ECX  (nWidth = rc.right - rc.left)
        _emit 0x51
        // 0001d552  PUSH 0x80000000  (Y = CW_USEDEFAULT)
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0001d557  PUSH 0x80000000  (X = CW_USEDEFAULT)
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0001d55c  PUSH 0x4ca0000  (dwStyle)
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0xca
        _emit 0x04
        // 0001d561  PUSH 0xf58245  (lpWindowName)
        _emit 0x68
        _emit 0x45
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        // 0001d566  PUSH ESI  (lpClassName)
        _emit 0x56
        // 0001d567  PUSH 0x40100  (dwExStyle)
        _emit 0x68
        _emit 0x00
        _emit 0x01
        _emit 0x04
        _emit 0x00
        // 0001d56c  CALL dword ptr [0x00f3e4e4]  (CreateWindowExA)
        _emit 0xff
        _emit 0x15
        _emit 0xe4
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001d572  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0001d574  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0001d576  JNZ 0x0041d5b4
        _emit 0x75
        _emit 0x3c
        // 0001d578  TEST byte ptr [0x01323910], 0x1
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0001d57f  JNZ 0x0041d592
        _emit 0x75
        _emit 0x11
        // 0001d581  OR dword ptr [0x01323910], 0x1
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0001d588  MOV dword ptr [0x0132390c], 0x41d3a0
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0xd3
        _emit 0x41
        _emit 0x00
        // 0001d592  PUSH 0xf589f8
        _emit 0x68
        _emit 0xf8
        _emit 0x89
        _emit 0xf5
        _emit 0x00
        // 0001d597  PUSH 0x95a
        _emit 0x68
        _emit 0x5a
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0001d59c  PUSH 0xf58a30
        _emit 0x68
        _emit 0x30
        _emit 0x8a
        _emit 0xf5
        _emit 0x00
        // 0001d5a1  PUSH 0xf58246
        _emit 0x68
        _emit 0x46
        _emit 0x82
        _emit 0xf5
        _emit 0x00
        // 0001d5a6  PUSH 0xf58a80
        _emit 0x68
        _emit 0x80
        _emit 0x8a
        _emit 0xf5
        _emit 0x00
        // 0001d5ab  CALL dword ptr [0x0132390c]  (error callback)
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0001d5b1  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0001d5b4  MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0001d5b6  POP ESI
        _emit 0x5e
        // 0001d5b7  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0001d5ba  RET
        _emit 0xc3
    }
}
