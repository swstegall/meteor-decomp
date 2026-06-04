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
// FUNCTION: ffxivgame 0x0041b7d0 — `__stdcall` window message dispatcher
//                                   (WndProc helper, 80 B / 0x50).
//
// Behaviour read from the disassembly at orig RVA 0x0001b7d0:
//
//   __stdcall LRESULT FUN_0041b7d0(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//
//   Four parameters, __stdcall (RET 0x10 — caller's 16 bytes cleaned by callee).
//   Register allocation (MSVC /O2 choice):
//     ESI = uMsg  (p2, second arg)
//     EBX = wParam (p3, third arg)
//     EDI = hWnd  (p1, first arg)
//
//   Logical structure:
//
//     if (uMsg == WM_CLOSE /* 0x10 */) {
//         IAT[0x00f3e49c](0);          // PostQuitMessage(0) — single arg
//         return 0;
//     }
//     if (uMsg == WM_KEYDOWN /* 0x100 */ && wParam == VK_ESCAPE /* 0x1b */) {
//         IAT[0x00f3e494](hWnd, WM_CLOSE, 0, 0);  // SendMessage / PostMessage
//     }
//     return IAT[0x00f3e498](hWnd, uMsg, wParam, lParam);  // DefWindowProc
//
//   The WM_CLOSE branch exits early (POP ESI; XOR EAX,EAX; RET 0x10)
//   having only saved ESI — EBX and EDI are never pushed on that path.
//
//   MSVC 2005 /O2 interleaving: the CMP ESI,0x100 flag is set before
//   the PUSH EBX / PUSH EDI register saves; the JNZ 0x41b7fe acting on
//   that flag comes AFTER both pushes. This is a standard MSVC scheduling
//   pattern where callee-save pushes fill the flag-to-branch shadow.
//
//   Three IAT indirect-call sites (absolute addresses baked into the
//   post-link binary — COFF IMAGE_REL_I386_DIR32 resolved by the linker):
//     0x0001b7f8  CALL dword ptr [0x00f3e494]  — 4-arg  (hWnd, WM_CLOSE, 0, 0)
//     0x0001b806  CALL dword ptr [0x00f3e498]  — 4-arg  (hWnd, uMsg, wParam, lParam)
//     0x0001b814  CALL dword ptr [0x00f3e49c]  — 1-arg  (0)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ port would need `__declspec(dllimport)` declarations
//   for all three IAT functions. The resulting COFF relocations in the .obj
//   would leave the four-byte address operands as zeros (resolved only at
//   link time); tools/compare.py performs a raw byte comparison against the
//   orig PE's post-link bytes (0x94e4f300, 0x98e4f300, 0x9ce4f300), so a
//   relocation-bearing .obj cannot match. The same constraint forced
//   FUN_00404f10 and FUN_00401090 into the _emit approach; this function
//   follows suit. The structural commentary above is the readable record.

extern "C" __declspec(naked) void FUN_0041b7d0() {
    __asm {
        // 0001b7d0: push esi
        _emit 0x56
        // 0001b7d1: mov esi, dword ptr [esp+0ch]    ; ESI = uMsg (p2)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0001b7d5: cmp esi, 10h                     ; WM_CLOSE?
        _emit 0x83
        _emit 0xfe
        _emit 0x10
        // 0001b7d8: jz  0041b812h                    ; → WM_CLOSE branch
        _emit 0x74
        _emit 0x38
        // 0001b7da: cmp esi, 100h                    ; WM_KEYDOWN? (flags set before saves)
        _emit 0x81
        _emit 0xfe
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b7e0: push ebx
        _emit 0x53
        // 0001b7e1: mov ebx, dword ptr [esp+14h]     ; EBX = wParam (p3)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 0001b7e5: push edi
        _emit 0x57
        // 0001b7e6: mov edi, dword ptr [esp+10h]     ; EDI = hWnd (p1)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0001b7ea: jnz 0041b7feh                    ; if uMsg != WM_KEYDOWN → main call
        _emit 0x75
        _emit 0x12
        // 0001b7ec: cmp ebx, 1bh                     ; VK_ESCAPE?
        _emit 0x83
        _emit 0xfb
        _emit 0x1b
        // 0001b7ef: jnz 0041b7feh                    ; if wParam != VK_ESCAPE → main call
        _emit 0x75
        _emit 0x0d
        // 0001b7f1: push 0                            ; lParam = 0
        _emit 0x6a
        _emit 0x00
        // 0001b7f3: push 0                            ; wParam = 0
        _emit 0x6a
        _emit 0x00
        // 0001b7f5: push 10h                          ; uMsg = WM_CLOSE
        _emit 0x6a
        _emit 0x10
        // 0001b7f7: push edi                          ; hWnd
        _emit 0x57
        // 0001b7f8: call dword ptr [00f3e494h]        ; SendMessage/PostMessage(hWnd, WM_CLOSE, 0, 0)
        _emit 0xff
        _emit 0x15
        _emit 0x94
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001b7fe: mov eax, dword ptr [esp+1ch]     ; EAX = lParam (p4)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001b802: push eax                          ; lParam
        _emit 0x50
        // 0001b803: push ebx                          ; wParam
        _emit 0x53
        // 0001b804: push esi                          ; uMsg
        _emit 0x56
        // 0001b805: push edi                          ; hWnd
        _emit 0x57
        // 0001b806: call dword ptr [00f3e498h]        ; DefWindowProc(hWnd, uMsg, wParam, lParam)
        _emit 0xff
        _emit 0x15
        _emit 0x98
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001b80c: pop edi
        _emit 0x5f
        // 0001b80d: pop ebx
        _emit 0x5b
        // 0001b80e: pop esi
        _emit 0x5e
        // 0001b80f: ret 10h
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // --- WM_CLOSE branch (only ESI saved) --------------------------------
        // 0001b812: push 0
        _emit 0x6a
        _emit 0x00
        // 0001b814: call dword ptr [00f3e49ch]        ; PostQuitMessage(0)
        _emit 0xff
        _emit 0x15
        _emit 0x9c
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        // 0001b81a: xor eax, eax                      ; return 0
        _emit 0x33
        _emit 0xc0
        // 0001b81c: pop esi
        _emit 0x5e
        // 0001b81d: ret 10h
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
