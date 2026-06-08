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
// FUNCTION: ffxivgame 0x00022ce0 — smart-pointer release + free helper
//                                  (__stdcall, 1 arg, 39 B / 0x27)
//
// void __stdcall FUN_00422ce0(IHolder* pHolder)
//
// 1. Null-guard on pHolder.
// 2. Loads obj = pHolder->ptr.
// 3. If obj non-null, calls the __stdcall function pointer at vtable
//    slot 2 of obj (offset +8 in the vtbl pointer table), passing obj
//    as the explicit first argument.
// 4. Zeroes pHolder->ptr and frees pHolder via FUN_009d1b17 (__stdcall).
//
// Calling convention: __stdcall — 1 arg, RET 0x4.
// Frame: none (/Oy — ESI is the only callee-save; no EBP frame).
//
// Register assignment:
//   ESI = pHolder (callee-saved parameter)
//   EAX = obj = *pHolder (volatile; pushed as vtable call arg)
//   ECX = vtable pointer (temp for double-indirection load)
//   EDX = function pointer at vtable[2] (loaded via [ECX+8])
//
// Asm (39 bytes @ orig RVA 0x00022ce0):
//   56                   PUSH ESI
//   8b 74 24 08          MOV ESI, [ESP+0x8]       ; pHolder
//   85 f6                TEST ESI, ESI
//   74 1d                JZ  end
//   8b 06                MOV EAX, [ESI]            ; obj = pHolder->ptr
//   85 c0                TEST EAX, EAX
//   74 08                JZ  cleanup
//   8b 08                MOV ECX, [EAX]            ; vtable ptr
//   8b 51 08             MOV EDX, [ECX+0x8]        ; vtable slot 2
//   50                   PUSH EAX                  ; arg = obj
//   ff d2                CALL EDX
// cleanup:
//   56                   PUSH ESI                  ; arg = pHolder
//   c7 06 00 00 00 00    MOV dword ptr [ESI], 0x0  ; pHolder->ptr = 0
//   e8 14 ee 5a 00       CALL FUN_009d1b17         ; (reloc)
// end:
//   5e                   POP ESI
//   c2 04 00             RET 0x4

struct IHolder;
struct IObj;

typedef void (__stdcall *PFN_Release)(IObj*);

struct IObj_vtbl {
    void* dummy0;   // vtable slot 0 (+0)
    void* dummy1;   // vtable slot 1 (+4)
    PFN_Release Release;  // vtable slot 2 (+8)
};

struct IObj {
    IObj_vtbl* vptr;
};

struct IHolder {
    IObj* ptr;
};

extern "C" void FUN_009d1b17(IHolder*);  // __cdecl — caller cleans (ADD ESP,4)

extern "C" void __stdcall FUN_00422ce0(IHolder* pHolder) {
    if (pHolder) {
        IObj* obj = pHolder->ptr;
        if (obj) {
            obj->vptr->Release(obj);
        }
        pHolder->ptr = 0;
        FUN_009d1b17(pHolder);
    }
}
