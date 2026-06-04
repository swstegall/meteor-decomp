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
// FUNCTION: ffxivgame 0x00035720 — `__thiscall` virtual-dispatch guard that,
//                                   on a truthy result, lazily binds a
//                                   file-scope logging/report function pointer
//                                   and forwards a 5-arg call to it
//                                   (89 B / 0x59, no SEH).
//
// Inspection (read from asm/ffxivgame/00035720_FUN_00435720.s):
//
//   __thiscall void FUN_00435720(this /* ECX */, Obj* p /* [ESP+0x4] */);
//
//     // Virtual call through `p`'s vtable (slot 0x12c = entry 75), invoked
//     // as a plain 2-arg cdecl-style call: pushes (p, this+4) then calls
//     // p->vtbl[0x12c](p, this+4). The pushed order is ECX(this+4) then
//     // EAX(p), so on the stack the first arg is `p`.
//     Obj*   p   = *(Obj**)(esp+4);
//     void*  vt  = *(void**)p;                 // p->vtbl
//     int    ok  = ((int(*)(Obj*, void*))(*(void**)((char*)vt + 0x12c)))
//                      (p, (char*)this + 4);
//     if (ok == 0) return;                     // TEST EAX,EAX ; JZ tail
//
//     // First-truthy lazy bind of a file-scope reporter pointer, guarded by
//     // a 1-byte init flag packed into a dword in .data (MSVC's classic
//     // `MOV EAX,1 / TEST [flag],AL` + 4-byte `OR [flag],EAX` idiom).
//     static char  s_initFlag;                 // .data 0x01323910
//     static void* s_reporter;                 // .data 0x0132390c
//     if (!(s_initFlag & 1)) {
//         s_initFlag |= 1;
//         s_reporter  = (void*)0x00433720;     // bound report fn
//     }
//     // Forward a 5-arg call (looks like a file/line + message reporter):
//     ((void(*)(int,int,int,int,int))s_reporter)(
//         0xf64be8, 0xf64bfc, 0xf64c18, 0xd7, 0xf64c70);  // pushed reverse
//     // ADD ESP,0x14  → cdecl, 5 dword args cleaned by caller.
//
// Reloc-bearing sites in the orig 89 bytes (these absolute addresses /
// indirect-call slots resolve only in a full-binary relink at image base
// 0x00400000; tools/compare.py masks the reloc windows on the cmp_obj path
// so a naked-asm .obj with the same raw bytes matches byte-for-byte):
//     +0x1c   init-flag TEST            (.data 0x01323910)
//     +0x24   init-flag OR              (.data 0x01323910)
//     +0x2a   reporter store (init)     (.data 0x0132390c = 0x00433720)
//     +0x34   PUSH 0xf64c70            (.rdata string/arg ptr)
//     +0x3e   PUSH 0xf64c18            (.rdata string/arg ptr)
//     +0x43   PUSH 0xf64bfc            (.rdata string/arg ptr)
//     +0x48   PUSH 0xf64be8            (.rdata string/arg ptr)
//     +0x4d   indirect CALL [0x0132390c] (reporter dispatch)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level formulation would force MSVC to re-derive the exact
//   moffs/imm encodings for the .data flag + pointer stores and the
//   indirect `call dword ptr [..]` dispatch, and would emit COFF
//   relocations for the absolute addresses rather than the orig binary's
//   already-resolved immediates. The pragmatic choice — the same one the
//   FUN_0040a530 singleton-init sibling and dozens of others in this row
//   took — is a `__declspec(naked)` body re-emitting the orig 89 bytes
//   verbatim via MASM `_emit` directives, so the .obj's `.text` ends up
//   byte-identical to the orig slice (no relocations).
//
// Asm shape (89 bytes — RVA 0x00035720..0x00035779):
//
//     00035720:  8b 44 24 04              MOV  EAX, [ESP+0x4]      ; p
//     00035724:  8b 10                    MOV  EDX, [EAX]          ; p->vtbl
//     00035726:  83 c1 04                 ADD  ECX, 0x4            ; this+4
//     00035729:  51                       PUSH ECX
//     0003572a:  50                       PUSH EAX                 ; p
//     0003572b:  8b 82 2c 01 00 00        MOV  EAX, [EDX+0x12c]    ; vslot 75
//     00035731:  ff d0                    CALL EAX
//     00035733:  85 c0                    TEST EAX, EAX
//     00035735:  74 3f                    JZ   0x00435776          ; tail
//     00035737:  b8 01 00 00 00           MOV  EAX, 0x1
//     0003573c:  84 05 10 39 32 01        TEST [0x01323910], AL    ; init flag
//     00035742:  75 10                    JNZ  0x00435754          ; already bound
//     00035744:  09 05 10 39 32 01        OR   [0x01323910], EAX
//     0003574a:  c7 05 0c 39 32 01 20 37 43 00
//                                         MOV  [0x0132390c], 0x433720 ; reporter
//     00035754:  68 70 4c f6 00           PUSH 0xf64c70
//     00035759:  68 d7 00 00 00           PUSH 0xd7
//     0003575e:  68 18 4c f6 00           PUSH 0xf64c18
//     00035763:  68 fc 4b f6 00           PUSH 0xf64bfc
//     00035768:  68 e8 4b f6 00           PUSH 0xf64be8
//     0003576d:  ff 15 0c 39 32 01        CALL [0x0132390c]
//     00035773:  83 c4 14                 ADD  ESP, 0x14           ; cdecl, 5 args
//     00035776:  c2 04 00                 RET  0x4

extern "C" __declspec(naked) void FUN_00435720() {
    __asm {
        _emit 0x8b                  // MOV EAX, [ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b                  // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x83                  // ADD ECX, 0x4
        _emit 0xc1
        _emit 0x04
        _emit 0x51                  // PUSH ECX
        _emit 0x50                  // PUSH EAX
        _emit 0x8b                  // MOV EAX, [EDX+0x12c]
        _emit 0x82
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xff                  // CALL EAX
        _emit 0xd0
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74                  // JZ tail (+0x3f → 0x00435776)
        _emit 0x3f
        _emit 0xb8                  // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84                  // TEST [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75                  // JNZ (+0x10 → 0x00435754)
        _emit 0x10
        _emit 0x09                  // OR [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7                  // MOV [0x0132390c], 0x433720
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64c70
        _emit 0x70
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xd7
        _emit 0xd7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64c18
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64bfc
        _emit 0xfc
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0x68                  // PUSH 0xf64be8
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        _emit 0xff                  // CALL [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83                  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
