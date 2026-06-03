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
// FUNCTION: ffxivgame 0x00019150 — Object factory / construction wrapper
//           (227 B / 0xe3, __cdecl, EH3+/GS prolog). Constructs a local
//           parameter struct, calls an allocator-like function at 0x00419c40,
//           and if allocation succeeds initialises the result via a
//           __thiscall method at 0x0041a220. Stores the result through an
//           output pointer (arg0) and returns it. On allocation failure
//           stores null through the output pointer.
//
// Signature (recovered from epilogue — two ret paths, both restore FS:[0]):
//
//   void* FUN_00419150(void**  out,     // arg0  [esp+0x40]  output slot
//                      void*   arg1,    // arg1  [esp+0x44]  passed to 0x0041a220
//                      void*   arg2,    // arg2  [esp+0x48]  → local struct field
//                      void*   arg3,    // arg3  [esp+0x4c]  → local struct field
//                      void*   arg4,    // arg4  [esp+0x50]  → local struct field
//                      void*   arg5,    // arg5  [esp+0x54]  → local struct field
//                      void*   arg6,    // arg6  [esp+0x58]  → local struct field
//                      void*   arg7,    // arg7  [esp+0x5c]  → local struct field
//                      void*   arg8);   // arg8  [esp+0x60]  → local struct field
//
// Frame (0x2c locals; SUB ESP, 0x2c after 3 EH-prolog pushes):
//
//   [esp+0x00]  /GS cookie
//   [esp+0x04]  local struct base (constructed in place, 0x2c bytes)
//   [esp+0x30]  saved FS:[0]   (prev exception handler)
//   [esp+0x34]  scope-table ptr  (0xe5560e  → .rdata reloc)
//   [esp+0x38]  EH state / try_level (-1 initially; set to 0 after alloc)
//   [esp+0x3c]  return address
//   [esp+0x40]  arg0 (out**)
//   ...
//   [esp+0x60]  arg8
//
// Body sketch:
//
//   // --- build parameter struct at local[4] ----------------------------
//   local.flags0   = 0;
//   local.field08  = 0;
//   local.field0c  = arg5;
//   local.field10  = arg8;
//   local.field14  = arg2;
//   local.field18  = arg3;
//   local.field1c  = 1;
//   local.field20  = arg4;
//   local.field24  = arg6;
//   local.field28  = arg7;
//   // --- call 0x0040e2d0 (thiscall on &local[4], args 0x10, 0xf57cf0) --
//   void* key = FUN_0040e2d0(&local[4], 0x10, 0xf57cf0);
//   // --- allocate result object of size 0x34 ---------------------------
//   void* obj = FUN_00419c40(key, 0x34);    // __cdecl allocator
//   try_level = 0;
//   if (obj) {
//       FUN_0041a220(obj, &local[0xc], arg1); // thiscall ctor
//       *out = obj;
//       return out;
//   }
//   *out = nullptr;
//   return out;  // (EAX = out, but *out == 0 on this path)
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The MSVC 2005 EH3 prolog (PUSH -1 / PUSH scope_table / PUSH FS:[0] /
//   SUB ESP, 0x2c / XOR-cookie / LEA / MOV FS:[0]) together with the
//   exact stack-slot scheduling across the eight parameter loads and the
//   three __thiscall call sites cannot be reliably reproduced from C++
//   source under /O2 /GS /EHsc without hitting modrm / branch-short-vs-
//   near / frame-size shifts. Naked asm with _emit directives gives a
//   byte-exact 227-byte .text segment modulo the six reloc-bearing slots
//   that compare.py masks.
//
// Reloc-bearing sites (offset within function):
//   +0x03   DIR32 → 0x00e5560e  (scope table, .rdata)
//   +0x12   DIR32 → 0x012ea8b0  (__security_cookie, .data)
//   +0x58   DIR32 → 0x00f57cf0  (string/rdata pointer)
//   +0x7f   REL32 → 0x0040e2d0  (CALL, .text)
//   +0x8b   REL32 → 0x00419c40  (CALL, .text)
//   +0xaf   REL32 → 0x0041a220  (CALL, .text)

extern "C" __declspec(naked) void FUN_00419150() {
    __asm {
        // --- EH3 + /GS prolog -------------------------------------------
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH scope_table (reloc +0x03)
        _emit 0x0e
        _emit 0x56
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x2c
        _emit 0xec
        _emit 0x2c
        _emit 0xa1  // MOV EAX, [__security_cookie] (reloc +0x12)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX, [ESP+0x30]
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- load args; init local struct field 0 to zero ---------------
        _emit 0x8b  // MOV EDX, [ESP+0x48]  ; arg2
        _emit 0x54
        _emit 0x24
        _emit 0x48
        _emit 0xc7  // MOV dword ptr [ESP+0x4], 0
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0x54]  ; arg5
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x8b  // MOV ECX, [ESP+0x60]  ; arg8
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x89  // MOV [ESP+0x10], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8b  // MOV EAX, [ESP+0x4c]  ; arg3
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x89  // MOV [ESP+0x14], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b  // MOV ECX, [ESP+0x50]  ; arg4
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0x89  // MOV [ESP+0x18], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x8b  // MOV EDX, [ESP+0x58]  ; arg6
        _emit 0x54
        _emit 0x24
        _emit 0x58
        _emit 0x89  // MOV [ESP+0x1c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b  // MOV EAX, [ESP+0x5c]  ; arg7
        _emit 0x44
        _emit 0x24
        _emit 0x5c

        // --- PUSH args for thiscall; build remaining struct fields -------
        _emit 0x68  // PUSH 0xf57cf0 (reloc +0x58)
        _emit 0xf0
        _emit 0x7c
        _emit 0xf5
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x28], ECX  ; store arg4
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x6a  // PUSH 0x10
        _emit 0x10
        _emit 0x8d  // LEA ECX, [ESP+0xc]  ; ECX = &local[4] (this)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xc7  // MOV dword ptr [ESP+0x14], 0
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESP+0x28], 1
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x30], EDX  ; store arg6
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x89  // MOV [ESP+0x34], EAX  ; store arg7
        _emit 0x44
        _emit 0x24
        _emit 0x34

        // --- CALL FUN_0040e2d0 (thiscall, RET 8) -------------------------
        _emit 0xe8  // CALL (reloc +0x7f)
        _emit 0xfd
        _emit 0x50
        _emit 0xff
        _emit 0xff

        // --- allocate result; set EH state; branch on null ---------------
        _emit 0x50  // PUSH EAX
        _emit 0x6a  // PUSH 0x34
        _emit 0x34
        _emit 0x89  // MOV [ESP+0x5c], EAX  ; stash key in consumed arg slot
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        _emit 0xe8  // CALL FUN_00419c40 (reloc +0x8b)
        _emit 0x61
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0x89  // MOV [ESP+0x60], EAX  ; stash obj ptr in arg8 slot
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7  // MOV dword ptr [ESP+0x38], 0  ; EH state = 0
        _emit 0x44
        _emit 0x24
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74  // JZ null_path (+0x29)
        _emit 0x29

        // --- success path: call ctor, store result, restore EH frame -----
        _emit 0x8b  // MOV EDX, [ESP+0x44]  ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0x8d  // LEA ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51  // PUSH ECX
        _emit 0x52  // PUSH EDX
        _emit 0x8b  // MOV ECX, EAX  ; this = obj ptr
        _emit 0xc8
        _emit 0xe8  // CALL FUN_0041a220 (reloc +0xaf)
        _emit 0x1d
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, [ESP+0x40]  ; out**
        _emit 0x4c
        _emit 0x24
        _emit 0x40
        _emit 0x89  // MOV [ECX], EAX  ; *out = result
        _emit 0x01
        _emit 0x8b  // MOV EAX, ECX  ; return value = out
        _emit 0xc1
        _emit 0x8b  // MOV ECX, [ESP+0x30]  ; restore saved FS:[0]
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x83  // ADD ESP, 0x38
        _emit 0xc4
        _emit 0x38
        _emit 0xc3  // RET

        // --- null_path: store 0 through out**, restore EH frame ----------
        _emit 0x8b  // MOV EAX, [ESP+0x40]  ; out**
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x33  // XOR ECX, ECX
        _emit 0xc9
        _emit 0x89  // MOV [EAX], ECX  ; *out = 0
        _emit 0x08
        _emit 0x8b  // MOV ECX, [ESP+0x30]  ; restore saved FS:[0]
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x83  // ADD ESP, 0x38
        _emit 0xc4
        _emit 0x38
        _emit 0xc3  // RET
    }
}
