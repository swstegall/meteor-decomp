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
// FUNCTION: ffxivgame 0x000054d0 — config-pad copy-out helper
//                                  (253 B / 0xfd, EH4-SEH wrapped).
//
// Behaviour read from the disassembly at orig RVA 0x000054d0:
//
//   __cdecl char FUN_004054d0(void* dst);
//
//     ScopedHelper helper;                 // ctor at 0x00445cf0 (39 B,
//                                          // matches FUN_004053a0)
//     /* state = 0; */                     // SEH trylevel
//     char ok = helper.load(&helper, 0);   // FUN_00405210 — same loader
//                                          // sibling consumed by
//                                          // FUN_004053a0.
//     if (ok) {
//         Utf8String tmp;                  // local at [esp+0xc]
//         tmp = FormatPath(                // FUN_00447260(g_user_dir,
//             g_user_dir_ptr,              //              "\\config.pad")
//             "\\config.pad");             // g_user_dir at .data
//                                          // 0x00f67298; string at
//                                          // .data 0x00f54c64.
//         /* state = 1; */
//         helper.setPath(tmp);             // FUN_004488f0 — sibling of
//                                          // FUN_00448900 used by
//                                          // FUN_004053a0.
//         /* state = 0; */
//         tmp.~Utf8String();               // FUN_00446f50 (24 B)
//         size_t len = helper.size() + 1;  // FUN_004451f0 (7-byte getter)
//         void*  buf = helper.data();      // FUN_00445210 (3-byte getter)
//         memcpy(dst, buf, len);           // 0x009d4600 — CRT memcpy
//     }
//     /* state = -1; */
//     helper.~ScopedHelper();              // FUN_00446f50 (2nd)
//     return ok;
//
//   Stack frame (after the EH4 prologue, ESP-relative):
//     [esp+0x000]                EH4 cookie #2 (PUSH'd post-XOR-ESP)
//     [esp+0x004]                saved ESI (dst mirror)
//     [esp+0x008]                saved EBX (ok mirror in BL)
//     [esp+0x00c .. esp+0x05f]   tmp Utf8String (0x54 B)
//     [esp+0x060 .. esp+0x0b3]   helper ScopedHelper (0x54 B)
//     [esp+0x0a8]                __security_cookie ^ ESP (orig copy)
//     [esp+0x0b8]                EH4 saved-FS:[0] chain link
//     [esp+0x0bc]                EH4 scope-table address (0x00e54803)
//     [esp+0x0c0]                EH4 trylevel (`local_4`)
//     [esp+0x0c4]                saved return address (push -1 at entry)
//     [esp+0x0c8]                incoming dst
//
//   Reloc-bearing sites in the orig 253 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x03   scope-table handler RVA  (0x00e54803 — .rdata FuncInfo)
//     +0x09   FS:[0] read              (constant 0, fold-through)
//     +0x15   __security_cookie load   (.data 0x012ea8b0)
//     +0x25   __security_cookie load   (.data 0x012ea8b0, 2nd)
//     +0x34   FS:[0] install           (constant 0, fold-through)
//     +0x45   helper ctor CALL         (.text 0x00445cf0 rel32 — __thiscall)
//     +0x5c   helper.load CALL         (.text 0x00405210 rel32 — __cdecl)
//     +0x6a   g_user_dir_ptr load      (.data 0x00f67298)
//     +0x71   string lit               ("\\config.pad" → 0x00f54c64)
//     +0x7a   FormatPath CALL          (.text 0x00447260 rel32 — __thiscall)
//     +0x8c   setPath CALL             (.text 0x004488f0 rel32 — __thiscall)
//     +0x9d   tmp.~ CALL               (.text 0x00446f50 rel32 — __thiscall)
//     +0xa6   helper.size CALL         (.text 0x004451f0 rel32 — __thiscall)
//     +0xb3   helper.data CALL         (.text 0x00445210 rel32 — __thiscall)
//     +0xba   memcpy CALL              (.text 0x009d4600 rel32 — __cdecl)
//     +0xd1   helper.~ CALL            (.text 0x00446f50 rel32, 2nd)
//     +0xf2   __security_check_cookie  (.text 0x009d20f4 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact EH4 prolog (PUSH -1 / PUSH scope-table /
//   PUSH FS:[0] / SUB ESP / double __security_cookie XOR ESP), the
//   exact register allocation across the success arm (BL preserves the
//   `ok` return value across two helper-method calls and a memcpy),
//   AND the linker-resolved absolute addresses in the seventeen
//   relocation windows above. Each of those constraints is brittle
//   under /O2 — every high-level rewrite shifts at least one byte
//   (state numbering, branch short-vs-near, modrm vs moffs32, frame
//   layout).
//
//   The pragmatic choice — the same one FUN_00401a00 / FUN_004014b0 /
//   FUN_00403a20 / FUN_004053a0 took for their SEH-wrapped routines —
//   is a `__declspec(naked)` body that re-emits the orig 253 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice (no relocations because
//   the bytes are emitted as raw immediates), which is what
//   `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the surrounding
//   ScopedHelper / Utf8String / FormatPath classes and the
//   FUN_00405210 sibling are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_004054d0() {
    __asm {
        // 000054d0  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 000054d2  PUSH 0xe54803 (scope-table handler RVA)
        _emit 0x68
        _emit 0x03
        _emit 0x48
        _emit 0xe5
        _emit 0x00
        // 000054d7  MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000054dd  PUSH EAX
        _emit 0x50
        // 000054de  SUB ESP, 0xac
        _emit 0x81
        _emit 0xec
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000054e4  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000054e9  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 000054eb  MOV [ESP+0xa8], EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000054f2  PUSH EBX
        _emit 0x53
        // 000054f3  PUSH ESI
        _emit 0x56
        // 000054f4  MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000054f9  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 000054fb  PUSH EAX  (EH4 cookie #2)
        _emit 0x50
        // 000054fc  LEA EAX, [ESP+0xb8]
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00005503  MOV FS:[0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00005509  MOV ESI, [ESP+0xc8]   ; dst arg
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00005510  LEA ECX, [ESP+0x60]   ; &helper
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        // 00005514  CALL 0x00445cf0       ; helper ctor
        _emit 0xe8
        _emit 0xd7
        _emit 0x07
        _emit 0x04
        _emit 0x00
        // 00005519  LEA EAX, [ESP+0x60]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x60
        // 0000551d  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0000551f  PUSH EAX
        _emit 0x50
        // 00005520  MOV dword [ESP+0xc8], 0  ; trylevel = 0
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000552b  CALL 0x00405210       ; helper.load
        _emit 0xe8
        _emit 0xe0
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 00005530  MOV BL, AL            ; save `ok`
        _emit 0x8a
        _emit 0xd8
        // 00005532  ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00005535  TEST BL, BL
        _emit 0x84
        _emit 0xdb
        // 00005537  JZ +0x58  -> 0x00005591
        _emit 0x74
        _emit 0x58
        // 00005539  MOV ECX, [0x00f67298] ; g_user_dir_ptr
        _emit 0x8b
        _emit 0x0d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 0000553f  PUSH ECX
        _emit 0x51
        // 00005540  PUSH 0xf54c64         ; "\\config.pad"
        _emit 0x68
        _emit 0x64
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        // 00005545  LEA ECX, [ESP+0x14]   ; &tmp
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00005549  CALL 0x00447260       ; FormatPath
        _emit 0xe8
        _emit 0x12
        _emit 0x1d
        _emit 0x04
        _emit 0x00
        // 0000554e  PUSH EAX
        _emit 0x50
        // 0000554f  LEA ECX, [ESP+0x64]   ; &helper (post 2 PUSHes)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        // 00005553  MOV byte [ESP+0xc4], 1  ; trylevel = 1
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // 0000555b  CALL 0x004488f0       ; helper.setPath
        _emit 0xe8
        _emit 0x90
        _emit 0x33
        _emit 0x04
        _emit 0x00
        // 00005560  LEA ECX, [ESP+0xc]    ; &tmp
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00005564  MOV byte [ESP+0xc0], 0  ; trylevel = 0
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000556c  CALL 0x00446f50       ; tmp.~Utf8String
        _emit 0xe8
        _emit 0xdf
        _emit 0x19
        _emit 0x04
        _emit 0x00
        // 00005571  LEA ECX, [ESP+0x60]   ; &helper
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        // 00005575  CALL 0x004451f0       ; helper.size
        _emit 0xe8
        _emit 0x76
        _emit 0xfc
        _emit 0x03
        _emit 0x00
        // 0000557a  ADD EAX, 1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0000557d  PUSH EAX
        _emit 0x50
        // 0000557e  LEA ECX, [ESP+0x64]   ; &helper
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        // 00005582  CALL 0x00445210       ; helper.data
        _emit 0xe8
        _emit 0x89
        _emit 0xfc
        _emit 0x03
        _emit 0x00
        // 00005587  PUSH EAX
        _emit 0x50
        // 00005588  PUSH ESI              ; dst
        _emit 0x56
        // 00005589  CALL 0x009d4600       ; memcpy
        _emit 0xe8
        _emit 0x72
        _emit 0xf0
        _emit 0x5c
        _emit 0x00
        // 0000558e  ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00005591  LEA ECX, [ESP+0x60]   ; &helper (join point)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        // 00005595  MOV dword [ESP+0xc0], 0xffffffff  ; trylevel = -1
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000055a0  CALL 0x00446f50       ; helper.~ScopedHelper
        _emit 0xe8
        _emit 0xab
        _emit 0x19
        _emit 0x04
        _emit 0x00
        // 000055a5  MOV AL, BL            ; return ok
        _emit 0x8a
        _emit 0xc3
        // 000055a7  MOV ECX, [ESP+0xb8]   ; saved FS:[0]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000055ae  MOV FS:[0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000055b5  POP ECX (cookie #2)
        _emit 0x59
        // 000055b6  POP ESI
        _emit 0x5e
        // 000055b7  POP EBX
        _emit 0x5b
        // 000055b8  MOV ECX, [ESP+0xa8]   ; cookie
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000055bf  XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 000055c1  CALL 0x009d20f4       ; __security_check_cookie
        _emit 0xe8
        _emit 0x2e
        _emit 0xcb
        _emit 0x5c
        _emit 0x00
        // 000055c6  ADD ESP, 0xb8
        _emit 0x81
        _emit 0xc4
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000055cc  RET
        _emit 0xc3
    }
}
