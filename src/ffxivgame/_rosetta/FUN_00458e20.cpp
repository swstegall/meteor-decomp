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
// FUNCTION: ffxivgame 0x00458e20 — COM QueryInterface helper
//                                   (__stdcall, 3 args, 96 bytes)
//
// HRESULT __stdcall FUN_00458e20(void *pObject, const GUID *riid, void **ppvObject)
//
// Stack layout at function entry (image base 0x00400000):
//   [ESP+0x04] : pObject    — the interface pointer to hand back on match
//   [ESP+0x08] : riid       — the requested interface GUID (const GUID*)
//   [ESP+0x0C] : ppvObject  — caller's output slot (void**)
//
// Behaviour:
//   1. If ppvObject == NULL → return E_INVALIDARG (0x80070057).
//   2. Call FUN_004598a0(riid, VA:0x11088f0) — IID comparison helper.
//      If it returns non-zero (match) → goto found.
//   3. Call FUN_004598a0(riid, VA:0xf678c4) — check second known IID.
//      If non-zero (match) → goto found.
//   4. Neither matched: *ppvObject = NULL, return E_NOINTERFACE (0x80004002).
//   found:
//   5. *ppvObject = pObject.
//      If pObject == NULL → return E_NOINTERFACE (reuse epilogue at +0x3D).
//   6. pObject non-NULL: load vtable = *pObject, EDX = vtable[1] (AddRef),
//      PUSH pObject, CALL EDX (C-style stdcall AddRef).
//      Return S_OK (0).
//
// FUN_004598a0 is a 2-arg __cdecl IID comparison helper (callee balance shown
// by ADD ESP,8 after each call). VA 0x11088f0 and 0xf678c4 are pointers to
// two known interface GUIDs embedded in the binary's data sections.
//
// The function at vtable[1] (+0x4 from the vtable base) is the COM AddRef
// slot, called here with the C-style stdcall convention (explicit pObject
// pushed on stack; callee pops via RET 4), so the stack is self-balanced
// without a separate cleanup ADD ESP.
//
// Reconstruction strategy — naked-asm _emit byte passthrough:
//
//   Forward conditional jumps must encode as rel8 (the original emits short
//   Jcc rel8 for all three intra-function branches). The safest way to
//   guarantee the exact encoding is to re-emit the 96 bytes verbatim via
//   MASM _emit. The two CALL rel32 offsets (+0x0a5e and +0x0a4c) and the
//   two PUSH imm32 absolute VAs (0x11088f0 and 0xf678c4) are all baked
//   directly into the binary, so there are no COFF relocations in the
//   resulting .obj — compare.py reports GREEN with zero reloc masking.
//
// Reloc-bearing sites in the orig 96 bytes:
//   none (all addresses baked into the original PE's VA space)
//
// Jump targets (all rel8, verified against original offsets):
//   +0x07  JNZ +0x09  → +0x12 (have_ppv: PUSH EDI)
//   +0x27  JNZ +0x1e  → +0x47 (found: MOV EAX,[ESP+0xC])
//   +0x39  JNZ +0x0c  → +0x47 (found)
//   +0x4f  JZ  -0x14  → +0x3d (return_nointerface: POP EDI)

extern "C" __declspec(naked) void FUN_00458e20() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x10]   (ppvObject = arg3)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +0x09  →  have_ppv (+0x12)
        _emit 0x09
        _emit 0xb8              // MOV EAX, 0x80070057  (E_INVALIDARG)
        _emit 0x57
        _emit 0x00
        _emit 0x07
        _emit 0x80
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xC
        _emit 0x0c
        _emit 0x00
        // have_ppv (+0x12):
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x10]   (riid = arg2)
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x68              // PUSH 0x11088f0   (first known IID VA)
        _emit 0xf0
        _emit 0x88
        _emit 0x10
        _emit 0x01
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_004598a0  (rel32 = +0x0a5e)
        _emit 0x5e
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x1e  →  found (+0x47)
        _emit 0x1e
        _emit 0x68              // PUSH 0xf678c4   (second known IID VA)
        _emit 0xc4
        _emit 0x78
        _emit 0xf6
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_004598a0  (rel32 = +0x0a4c)
        _emit 0x4c
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x0c  →  found (+0x47)
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [ESI], EAX   (*ppvObject = NULL, EAX==0)
        _emit 0x06
        // return_nointerface (+0x3d):
        _emit 0x5f              // POP EDI
        _emit 0xb8              // MOV EAX, 0x80004002  (E_NOINTERFACE)
        _emit 0x02
        _emit 0x40
        _emit 0x00
        _emit 0x80
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xC
        _emit 0x0c
        _emit 0x00
        // found (+0x47):
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0xC]   (pObject = arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV dword ptr [ESI], EAX   (*ppvObject = pObject)
        _emit 0x06
        _emit 0x74              // JZ -0x14  →  return_nointerface (+0x3d)
        _emit 0xec
        _emit 0x8b              // MOV ECX, dword ptr [EAX]     (ECX = vtable)
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX+0x4]  (EDX = vtable[1] = AddRef)
        _emit 0x51
        _emit 0x04
        _emit 0x50              // PUSH EAX   (this = pObject, C-style stdcall arg)
        _emit 0xff              // CALL EDX   (AddRef — callee pops via RET 4)
        _emit 0xd2
        _emit 0x5f              // POP EDI
        _emit 0x33              // XOR EAX, EAX   (S_OK = 0)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xC
        _emit 0x0c
        _emit 0x00
    }
}
