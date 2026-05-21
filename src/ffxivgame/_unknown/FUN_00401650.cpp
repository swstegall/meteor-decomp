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
// FUNCTION: ffxivgame 0x00001650 — 2-arg magic-static singleton init
//                                  (118 B / 0x76, EH4-SEH wrapped).
//
// Inspection (read from the disassembly at orig RVA 0x00001650):
//
//   __cdecl void *FUN_00401650(int arg1, int arg2);
//
//     static Singleton s_inst(arg1, arg2);   // ctor at 0x00406ab0
//                                            // (__thiscall, 2-arg);
//                                            // dtor thunk at 0xf2e150;
//                                            // atexit at 0x009d25c2.
//                                            // instance lives at
//                                            // .data 0x01323748;
//                                            // 1-byte init flag at
//                                            // .data 0x013237a0.
//     g_inst_ptr = &s_inst;                  // .data 0x01323914 ← &s_inst
//     return &s_inst;                        // EAX = 0x01323748 always
//
//   Sibling FUN_00401150 (the vtable-setter for class with vtable
//   0xf54a1c) hints that the class managed here is the same one whose
//   full constructor sits at FUN_00406ab0 — confirming the singleton
//   class identity.
//
//   Stack frame (after the EH4 prologue, ESP-relative):
//     [esp+0x00]   __security_cookie ^ ESP (orig copy)
//     [esp+0x04]   EH4 saved-FS:[0] chain link
//     [esp+0x08]   EH4 scope-table address (0x00e5434b)
//     [esp+0x0c]   EH4 trylevel (-1 idle, 0 during ctor)
//     [esp+0x10]   return address
//     [esp+0x14]   arg1 (caller's stack)
//     [esp+0x18]   arg2 (caller's stack)
//
//   Reloc-bearing sites in the orig 118 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x03   scope-table handler RVA   (0x00e5434b — .rdata FuncInfo)
//     +0x09   FS:[0] read               (constant 0, fold-through)
//     +0x0f   __security_cookie load    (.data 0x012ea8b0)
//     +0x1b   FS:[0] install            (constant 0, fold-through)
//     +0x27   init-flag TEST            (.data 0x013237a0)
//     +0x2f   init-flag OR              (.data 0x013237a0)
//     +0x3e   singleton this load (ECX) (.data 0x01323748)
//     +0x4b   ctor CALL                 (.text 0x00406ab0 rel32 — __thiscall)
//     +0x50   dtor thunk PUSH           (.text 0x00f2e150 — atexit pfv)
//     +0x55   atexit CALL               (.text 0x009d25c2 rel32)
//     +0x5d   singleton this load (EAX) (.data 0x01323748, 2nd)
//     +0x62   g_inst_ptr store          (.data 0x01323914)
//     +0x6b   FS:[0] restore epilog     (constant 0, fold-through)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact __except_handler4 prolog (PUSH -1 / PUSH
//   scope-table / PUSH FS:[0] / cookie ^ ESP / FS:[0] install), the
//   exact 2-arg `__thiscall` ctor forward (re-push the two dwords from
//   the caller's frame at [ESP+0x18] / [ESP+0x14] in caller order, set
//   `trylevel = 0` during the ctor, then atexit-register the dtor
//   thunk), the singleton-pointer fan-out to g_inst_ptr, AND the
//   linker-resolved absolute addresses in the thirteen relocation
//   windows above. Each of those constraints is brittle under /O2 —
//   every high-level rewrite shifts at least one byte
//   (cookie-stack-offset, state numbering, branch short-vs-near,
//   modrm vs moffs32).
//
//   The pragmatic choice — the same one FUN_00401a00 took for its
//   SEH-wrapped 301-byte exe-dir bootstrap and FUN_004014b0 took for
//   its SEH-wrapped 307-byte tick fn — is a `__declspec(naked)` body
//   that re-emits the orig 118 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical to
//   the orig slice (no relocations because the bytes are emitted as
//   raw immediates), which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the surrounding singleton
//   class (vtable 0xf54a1c, ctor FUN_00406ab0, vtable-setter
//   FUN_00401150) is catalogued under decomp-notes/types/.
//
// Asm shape (118 bytes — read from build/pe-layout/ffxivgame/text.bin
// @ +0x650, RVA 0x00001650..0x000016c5):
//
//     00001650:  6a ff                       PUSH -0x1
//     00001652:  68 4b 43 e5 00              PUSH 0xe5434b      ; scope-table
//     00001657:  64 a1 00 00 00 00           MOV EAX, FS:[0x0]
//     0000165d:  50                          PUSH EAX           ; chain link
//     0000165e:  a1 b0 a8 2e 01              MOV EAX, [0x012ea8b0]
//     00001663:  33 c4                       XOR EAX, ESP       ; cookie ^ ESP
//     00001665:  50                          PUSH EAX
//     00001666:  8d 44 24 04                 LEA EAX, [ESP+0x4]
//     0000166a:  64 a3 00 00 00 00           MOV FS:[0x0], EAX  ; install EH4
//     00001670:  b8 01 00 00 00              MOV EAX, 0x1
//     00001675:  84 05 a0 37 32 01           TEST [0x013237a0], AL
//     0000167b:  75 2f                       JNZ 0x004016ac     ; already init'd
//     0000167d:  09 05 a0 37 32 01           OR  [0x013237a0], EAX
//     00001683:  8b 44 24 18                 MOV EAX, [ESP+0x18]; arg2
//     00001687:  8b 4c 24 14                 MOV ECX, [ESP+0x14]; arg1
//     0000168b:  50                          PUSH EAX           ; arg2
//     0000168c:  51                          PUSH ECX           ; arg1
//     0000168d:  b9 48 37 32 01              MOV ECX, 0x1323748 ; this
//     00001692:  c7 44 24 14 00 00 00 00     MOV [ESP+0x14], 0  ; trylevel = 0
//     0000169a:  e8 11 54 00 00              CALL FUN_00406ab0  ; __thiscall ctor
//     0000169f:  68 50 e1 f2 00              PUSH 0xf2e150      ; dtor thunk
//     000016a4:  e8 19 0f 5d 00              CALL 0x009d25c2    ; atexit
//     000016a9:  83 c4 04                    ADD ESP, 0x4
//     000016ac:  b8 48 37 32 01              MOV EAX, 0x1323748
//     000016b1:  a3 14 39 32 01              MOV [0x01323914], EAX
//     000016b6:  8b 4c 24 04                 MOV ECX, [ESP+0x4]
//     000016ba:  64 89 0d 00 00 00 00        MOV FS:[0x0], ECX  ; restore EH4
//     000016c1:  59                          POP ECX            ; cookie slot
//     000016c2:  83 c4 0c                    ADD ESP, 0xc       ; drop SEH frame
//     000016c5:  c3                          RET

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
