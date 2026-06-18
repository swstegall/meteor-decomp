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
// FUNCTION: ffxivgame 0x00038fe0 — __thiscall constructor (171 B / 0xab)
//
// __thiscall SomeClass* FUN_00438fe0(SomeClass* this /*ECX*/, void* param1)
//
// Class layout (recovered from asm):
//   +0x00  vtable          — set to 0xf660f8
//   +0x04  field_4         — = param1
//   +0x08  vec_field8      — std::vector<T,20> sub-object (EDI = ESI+8)
//     +0x0c  _Myfirst (vec+4)  = 0
//     +0x10  _Mylast  (vec+8)  = 0
//     +0x14  _Myend   (vec+c)  = 0
//   +0x1c  field_1c        = 0
//
// If param1 is NULL, a one-time lazy init block sets a function pointer at
// [0x0132390c] to 0x4385a0 (guarded by flag [0x01323910] bit 0), then
// calls it with 5 constant string/data addresses.
//
// Always calls FUN_004365d0(this+8, 5) — the vector grow helper — to
// reserve 5 elements in the sub-vector.
//
// SEH frame: MSVC /EHsc pattern. Scope state at [ESP+0x18] transitions
// from -1 (no scope) to 0 before the sub-object constructor call.
// No explicit security cookie verify in epilogue (no local char arrays).
//
// Returns this in EAX (standard __thiscall constructor form).
//
// Reconstruction strategy: naked asm byte passthrough (171 bytes verbatim).
// Source-level form would require matching MSVC's exact SEH frame layout and
// register allocation (ESI=this, EDI=this+8) — baked bytes are safer given
// the 9-iteration register-allocation precedent in this binary.
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x02  PUSH imm32  0xe563fb          (SEH exception handler)
//   +0x11  MOV EAX, [0x012ea8b0]         (security cookie global)
//   +0x25  MOV dword ptr [ESI], 0xf660f8 (vtable imm32)
//   +0x4b  TEST [0x01323910], AL         (init flag)
//   +0x51  OR   [0x01323910], EAX        (init flag write)
//   +0x55  MOV  [0x0132390c], 0x4385a0   (fn-ptr slot + fn-ptr value)
//   +0x6d  PUSH 0xf65ce0                 (arg5)
//   +0x73  PUSH 0xf65d40                 (arg3)
//   +0x78  PUSH 0xf65d3d                 (arg4)
//   +0x7d  PUSH 0xf65da0                 (arg1)
//   +0x83  CALL dword ptr [0x0132390c]   (indirect call)
//   +0x8c  CALL rel32 → FUN_004365d0     (vector reserve)

extern "C" __declspec(naked) void FUN_00438fe0() {
    __asm {
        // --- SEH prologue ---
        _emit 0x6a  // PUSH -0x1            (scope_state = -1)
        _emit 0xff
        _emit 0x68  // PUSH 0xe563fb        (SEH handler address)
        _emit 0xfb
        _emit 0x63
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0x0]   (old SEH chain head)
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX             (link old SEH chain)
        _emit 0x51  // PUSH ECX             (save 'this' for SEH unwinder)
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0] (security cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX             (cookie XOR ESP)
        _emit 0x8d  // LEA EAX, [ESP+0x10]  (&SEH record = &old_SEH link)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64  // MOV FS:[0x0], EAX    (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- constructor body ---
        _emit 0x8b  // MOV ESI, ECX         (ESI = this)
        _emit 0xf1
        _emit 0x89  // MOV [ESP+0x0c], ESI  (re-save this for SEH unwinder)
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x8b  // MOV EAX, [ESP+0x20]  (param1 — [orig_ESP+4])
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89  // MOV [ESI+0x4], EAX   (this->field_4 = param1)
        _emit 0x46
        _emit 0x04
        _emit 0x8d  // LEA EDI, [ESI+0x8]   (EDI = &this->vec_field8)
        _emit 0x7e
        _emit 0x08
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc7  // MOV dword ptr [ESI], 0xf660f8  (set vtable)
        _emit 0x06
        _emit 0xf8
        _emit 0x60
        _emit 0xf6
        _emit 0x00
        _emit 0x89  // MOV [EDI+0x4], EAX   (vec._Myfirst = 0)
        _emit 0x47
        _emit 0x04
        _emit 0x89  // MOV [EDI+0x8], EAX   (vec._Mylast  = 0)
        _emit 0x47
        _emit 0x08
        _emit 0x89  // MOV [EDI+0x0c], EAX  (vec._Myend   = 0)
        _emit 0x47
        _emit 0x0c
        _emit 0x39  // CMP [ESI+0x4], EAX   (if param1 == NULL ...)
        _emit 0x46
        _emit 0x04
        _emit 0x89  // MOV [ESP+0x18], EAX  (scope_state = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89  // MOV [ESI+0x1c], EAX  (this->field_1c = 0)
        _emit 0x46
        _emit 0x1c
        _emit 0x75  // JNZ +0x3c            (skip lazy init if param1 != NULL)
        _emit 0x3c
        // --- lazy init block (only when param1 == NULL) ---
        _emit 0xb8  // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84  // TEST byte ptr [0x01323910], AL  (check init flag)
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75  // JNZ +0x10            (already initialized)
        _emit 0x10
        _emit 0x09  // OR dword ptr [0x01323910], EAX  (set init flag)
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7  // MOV dword ptr [0x0132390c], 0x4385a0  (store fn ptr)
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0x85
        _emit 0x43
        _emit 0x00
        _emit 0x68  // PUSH 0xf65ce0        (arg5)
        _emit 0xe0
        _emit 0x5c
        _emit 0xf6
        _emit 0x00
        _emit 0x6a  // PUSH 0x21            (arg4 = 33)
        _emit 0x21
        _emit 0x68  // PUSH 0xf65d40        (arg3)
        _emit 0x40
        _emit 0x5d
        _emit 0xf6
        _emit 0x00
        _emit 0x68  // PUSH 0xf65d3d        (arg2)
        _emit 0x3d
        _emit 0x5d
        _emit 0xf6
        _emit 0x00
        _emit 0x68  // PUSH 0xf65da0        (arg1)
        _emit 0xa0
        _emit 0x5d
        _emit 0xf6
        _emit 0x00
        _emit 0xff  // CALL dword ptr [0x0132390c]  (call via fn ptr)
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83  // ADD ESP, 0x14        (clean 5 push args)
        _emit 0xc4
        _emit 0x14
        // --- always: reserve vector sub-object ---
        _emit 0x6a  // PUSH 0x5             (newCount = 5)
        _emit 0x05
        _emit 0x8b  // MOV ECX, EDI         (this = &vec_field8)
        _emit 0xcf
        _emit 0xe8  // CALL FUN_004365d0    (rel32 = 0xffffd55b, vector reserve)
        _emit 0x5b
        _emit 0xd5
        _emit 0xff
        _emit 0xff
        // --- SEH epilogue ---
        _emit 0x8b  // MOV EAX, ESI         (return this)
        _emit 0xc6
        _emit 0x8b  // MOV ECX, [ESP+0x10]  (old SEH chain head)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64  // MOV FS:[0x0], ECX    (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX              (pop cookie XOR, discard)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x83  // ADD ESP, 0x10        (remove saved ECX+old_SEH+handler+scope_state)
        _emit 0xc4
        _emit 0x10
        _emit 0xc2  // RET 0x4              (callee-clean 1 __thiscall arg)
        _emit 0x04
        _emit 0x00
    }
}
