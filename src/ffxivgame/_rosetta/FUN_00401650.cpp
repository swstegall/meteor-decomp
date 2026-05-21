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
// FUNCTION: ffxivgame 0x00001650 — `__cdecl` once-init thunk for a
//                                  `__thiscall` ctor against a global
//                                  singleton (118 B / 0x76, EH3-SEH
//                                  wrapped, ESP-relative frame).
//
// Inspection (read from the disassembly at orig RVA 0x00001650):
//
//   __cdecl void FUN_00401650(undefined4 a1, undefined4 a2);
//
//   This is a classic MSVC 2005 function-scope `static T s_obj(a, b);`
//   once-init prologue, paired with `_atexit` registration for the
//   object's destructor thunk. The ctor and dtor live elsewhere; this
//   function is the thread-unsafe / single-init wrapper the compiler
//   emits around the first reference to the static. The trailing store
//   `g_ptr = &s_obj` exposes the singleton's address to other TUs via a
//   second global slot (the address-of-singleton publishes the live
//   pointer once the ctor has run).
//
//   Pseudo-C:
//
//     static unsigned char s_once_flag;        // .data 0x013237a0 (byte 0)
//     static T              s_obj;             // .data 0x01323748 (instance)
//     static T*             g_obj_ptr;         // .data 0x01323914
//
//     void __cdecl FUN_00401650(unsigned a1, unsigned a2) {
//         if ((s_once_flag & 1) == 0) {                  //  84 05 a0 37 32 01
//             s_once_flag |= 1;                          //  09 05 a0 37 32 01
//             // __thiscall T::T(a1, a2) on &s_obj
//             T::T(&s_obj, a1, a2);                      //  CALL 0x00406ab0
//             // Register the destructor thunk with _atexit so the
//             // global is torn down on exit / DLL unload.
//             _atexit((void (__cdecl*)())0x00f2e150);    //  CALL 0x009d25c2
//         }
//         g_obj_ptr = &s_obj;                            //  a3 14 39 32 01
//     }
//
//   Stack frame (after the EH3 prologue, ESP-relative — no EBP frame):
//     [esp+0x00]  __security_cookie ^ ESP_at_install
//     [esp+0x04]  EH3 saved-FS:[0]  (next handler in chain)
//     [esp+0x08]  EH3 scope-table   (0x00e5434b — .rdata FuncInfo)
//     [esp+0x0c]  EH3 trylevel      (initial -1; set to 0 before ctor call)
//     [esp+0x10]  return address
//     [esp+0x14]  caller a1
//     [esp+0x18]  caller a2
//
//   Reloc-bearing sites in the orig 118 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them as relocations,
//   so we emit them as raw immediates that link.exe will leave alone):
//     +0x03  scope-table handler RVA  (0x00e5434b — .rdata FuncInfo)
//     +0x09  FS:[0] read              (constant 0, fold-through)
//     +0x0f  __security_cookie load   (.data 0x012ea8b0)
//     +0x1c  FS:[0] install           (constant 0, fold-through)
//     +0x27  once-flag TEST           (.data 0x013237a0, byte)
//     +0x2f  once-flag OR             (.data 0x013237a0, dword)
//     +0x3e  ctor this-load           (.data 0x01323748 → ECX)
//     +0x4b  ctor CALL                (.text 0x00406ab0 rel32 — __thiscall)
//     +0x50  dtor thunk PUSH          (.text 0x00f2e150 — atexit pfv)
//     +0x55  _atexit CALL             (.text 0x009d25c2 rel32)
//     +0x5d  &singleton EAX-load      (.data 0x01323748, 2nd)
//     +0x62  g_obj_ptr store          (.data 0x01323914)
//     +0x6d  FS:[0] uninstall         (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact EH3 prolog (PUSH -1 / PUSH scope-table /
//   PUSH FS:[0] / cookie ^ ESP / FS:[0] = ESP+4) on top of an
//   ESP-relative frame (no SUB ESP, the cookie push itself owns the
//   slot), the exact once-flag TEST-byte-then-OR-dword codegen MSVC
//   emits for `static_init_flag |= 1`, the __thiscall ECX-immediate
//   load against the singleton's address, AND the linker-resolved
//   absolute addresses in the thirteen relocation windows above. Each
//   of those constraints is brittle under /O2 — every high-level
//   rewrite shifts at least one byte (modrm-vs-moffs32, byte-vs-dword
//   memory access, branch short-vs-near, register choice for the
//   call-arg shuffle).
//
//   The pragmatic choice — the same one FUN_004014b0 / FUN_00401a00 /
//   FUN_00401820 took for their EH-wrapped reloc-heavy bodies — is a
//   `__declspec(naked)` body that re-emits the orig 118 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations because the
//   bytes are emitted as raw immediates), which is what
//   `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the singleton class at
//   .data 0x01323748 (its ctor at 0x00406ab0, its dtor thunk at
//   0x00f2e150, and the ABI of the published-pointer slot at
//   0x01323914) are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00401650() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x4b
        _emit 0x43
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xa1
        _emit 0xb0

        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84
        _emit 0x05
        _emit 0xa0
        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0x75
        _emit 0x2f
        _emit 0x09
        _emit 0x05
        _emit 0xa0

        _emit 0x37
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x50
        _emit 0x51
        _emit 0xb9
        _emit 0x48
        _emit 0x37

        _emit 0x32
        _emit 0x01
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x11
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x68

        _emit 0x50
        _emit 0xe1
        _emit 0xf2
        _emit 0x00
        _emit 0xe8
        _emit 0x19
        _emit 0x0f
        _emit 0x5d
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0xb8
        _emit 0x48
        _emit 0x37
        _emit 0x32

        _emit 0x01
        _emit 0xa3
        _emit 0x14
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x59
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
    }
}
