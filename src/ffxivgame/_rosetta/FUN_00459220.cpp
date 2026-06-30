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
// FUNCTION: ffxivgame 0x00459220 — `__stdcall` COM-style dispatch wrapper
//                                   that queries a count via FUN_00458f30 and,
//                                   if non-zero, invokes vtable[2] on the
//                                   object's sink field (66 B / 0x42).
//
// __stdcall HRESULT FUN_00459220(FSqexObj *pThis, void *pData, void *pArg2)
//   stack layout (RET 0xC — callee cleans 3 dwords):
//     [ESP+0x04] : pThis  (FSqexObj pointer)
//     [ESP+0x08] : pData  (data pointer, forwarded to FUN_00458f30)
//     [ESP+0x0C] : pArg2  (void*; cleared before call; may be updated by callee)
//
// Structure (matches asm flow, orig RVA 0x00059220, 66 bytes):
//
//   prologue:
//     MOV EDX,[ESP+0x08]          ; EDX = pData   (loaded before PUSH ESI)
//     MOV EAX,[ESP+0x0C]          ; EAX = pArg2   (loaded before PUSH ESI)
//     PUSH ESI                    ; save ESI
//     MOV ESI,[ESP+0x08]          ; ESI = pThis   (now at ESP+8 after push)
//     LEA ECX,[ESP+0x10]          ; ECX = &pArg2  (stack slot at ESP+0x10)
//     PUSH ECX                    ; push &pArg2   (second arg to FUN_00458f30)
//     PUSH EDX                    ; push pData    (first  arg to FUN_00458f30)
//     MOV ECX,ESI                 ; ECX = pThis   (this for __thiscall)
//     MOV [EAX],0                 ; *pArg2 = 0    (initialize output)
//     CALL FUN_00458f30           ; n = pThis->FUN_00458f30(pData, &pArg2)
//
//   if (n <= 0):
//     MOV EAX,0x80070057          ; E_INVALIDARG
//     POP ESI
//     RET 0xC
//
//   success path:
//     MOV ECX,[ESI+8]             ; ECX = pThis->m_sink
//     MOV EAX,[ECX]              ; EAX = vtable pointer
//     MOV EDX,[ESP+0x10]         ; EDX = pArg2 (possibly updated by callee)
//     MOV EAX,[EAX+8]            ; EAX = vtable[2]
//     PUSH EDX                   ; push pArg2
//     CALL EAX                   ; pThis->m_sink->VFunc2(pArg2)
//     XOR EAX,EAX                ; return 0 (S_OK)
//     POP ESI
//     RET 0xC

struct FSqexSink {
    void **vftable;
};

struct FSqexObj {
    int f0;
    int f4;
    FSqexSink *m_sink;  // at offset 0x08

    unsigned int FUN_00458f30(void *pData, void **ppOut);
};

extern "C" long __stdcall FUN_00459220(FSqexObj *pThis, void *pData, void *pArg2)
{
    *(int *)pArg2 = 0;
    unsigned int n = pThis->FUN_00458f30(pData, (void **)&pArg2);
    if (n > 0) {
        FSqexSink *pSink = pThis->m_sink;
        ((void (__thiscall *)(FSqexSink *, void *))pSink->vftable[2])(pSink, pArg2);
        return 0;
    }
    return (long)0x80070057;
}
