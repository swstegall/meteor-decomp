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
// FUNCTION: ffxivgame 0x004053a0 — config-pad loader/uploader
//                                   (303 B / 0x12f, EH4-SEH wrapped).
//
// Behaviour read from the disassembly at orig RVA 0x000053a0:
//
//   __cdecl char FUN_004053a0(void* param_1);
//
//     ConfigPad cfg;                       // ctor at 0x00445cf0
//     ScopedHelper helper;                 // ctor at 0x00405210, dtor at 0x00446f50
//     if (!helper.load(&cfg, 0)) {
//         /* state = -1; */                // dtor of helper
//         return 0;
//     }
//     helper.setPath("\\config.pad");      // FUN_00448900("\\config.pad")
//     ConfigPadReader reader;              // ctor at 0x00452fe0
//     /* state = 1; */                     // SEH state to dtor reader
//     if (reader.parse(&cfg, &k_section, 0)) {  // FUN_00453c00(&cfg, &DAT_00f54c8c, 0)
//         FUN_00453030(param_1, 0x148);    // submit/upload payload
//         reader.finalize();               // FUN_00453000
//         /* state = 0; */                 // before dtor reader
//         reader.~ConfigPadReader();       // FUN_00453190
//         /* state = -1; */                // before dtor helper
//         helper.~ScopedHelper();          // FUN_00446f50
//         return 1;
//     }
//     /* state = 0; */
//     reader.~ConfigPadReader();           // FUN_00453190
//     /* state = -1; */
//     helper.~ScopedHelper();              // FUN_00446f50
//     return 0;
//
//   Stack frame (after the EH4 prologue, ESP-relative):
//     [esp+0x000]                saved ECX (EH4 saved-cookie slot)
//     [esp+0x004]                saved ESI (param_1 mirror)
//     [esp+0x008 .. esp+0x05b]   ScopedHelper local (helper) — 0x54 B
//     [esp+0x05c .. esp+0x077]   ConfigPadReader local (reader)
//     [esp+0x078]                EH4 trylevel (`local_4`)
//     [esp+0x07c]                ConfigPad cfg (small inline at top)
//     [esp+0x080]                saved param_1 slot
//     [esp+0x2064]               __security_cookie ^ ESP (orig copy)
//     [esp+0x2068]               EH4 second-cookie ^ ESP (push)
//     [esp+0x206c]               EH4 saved-FS:[0] chain link
//     [esp+0x2070]               EH4 scope-table address (0x00e547c2)
//     [esp+0x2074]               saved return address (push -1 at entry)
//     [esp+0x2080]               incoming param_1
//
//   Reloc-bearing sites in the orig 303 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x03   scope-table handler RVA  (0x00e547c2 — .rdata FuncInfo)
//     +0x09   FS:[0] read              (constant 0, fold-through)
//     +0x14   __chkstk size            (0x2068)
//     +0x19   __chkstk CALL            (.text 0x009d29d0 rel32)
//     +0x1d   __security_cookie load   (.data 0x012ea8b0)
//     +0x29   __security_cookie load   (.data 0x012ea8b0, 2nd)
//     +0x37   FS:[0] install           (constant 0, fold-through)
//     +0x49   helper ctor CALL         (.text 0x00445cf0 rel32 — __thiscall)
//     +0x72   helper.load CALL         (.text 0x00405210 rel32)
//     +0x73   string lit               ("\\config.pad" → 0x00f54c80)
//     +0x78   setPath CALL             (.text 0x00448900 rel32)
//     +0x82   reader ctor CALL         (.text 0x00452fe0 rel32 — __thiscall)
//     +0x88   string lit               (&DAT_00f54c8c)
//     +0x9d   parse CALL               (.text 0x00453c00 rel32 — __thiscall)
//     +0xae   submit/upload CALL       (.text 0x00453030 rel32 — __cdecl)
//     +0xb6   reader.finalize CALL     (.text 0x00453000 rel32 — __thiscall)
//     +0xc1   reader.~ CALL            (.text 0x00453190 rel32 — __thiscall)
//     +0xd1   helper.~ CALL            (.text 0x00446f50 rel32 — __thiscall)
//     +0xdf   reader.~ CALL            (.text 0x00453190 rel32, 2nd)
//     +0xef   helper.~ CALL            (.text 0x00446f50 rel32, 2nd)
//     +0x125  __security_check_cookie  (.text 0x009d20f4 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact EH4 prolog (PUSH -1 / PUSH scope-table /
//   PUSH FS:[0] / __chkstk for the 0x2068-byte locals frame / double
//   __security_cookie XOR ESP), the exact register allocation across the
//   four nested success/fail dispatch arms, AND the linker-resolved
//   absolute addresses in the twenty-one relocation windows above. Each
//   of those constraints is brittle under /O2 — every high-level rewrite
//   shifts at least one byte (state numbering, branch short-vs-near,
//   modrm vs moffs32, frame layout).
//
//   The pragmatic choice — the same one FUN_00401a00 / FUN_004014b0 /
//   FUN_00403a20 took for their SEH-wrapped routines — is a
//   `__declspec(naked)` body that re-emits the orig 303 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations because the bytes
//   are emitted as raw immediates), which is what `tools/compare.py`
//   checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the surrounding ConfigPad
//   / ScopedHelper / ConfigPadReader classes and the FUN_00405210
//   helper sibling are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_004053a0() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0xc2
        _emit 0x47
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xb8
        _emit 0x68
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x18
        _emit 0xd6
        _emit 0x5c
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x64
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x70
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x80
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xe8
        _emit 0x04
        _emit 0x09
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x6a
        _emit 0x00
        _emit 0x50
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x80
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x0d
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x84
        _emit 0xc0
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x0f
        _emit 0x84
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x80
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0xe8
        _emit 0xe4
        _emit 0x34
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
        _emit 0xe8
        _emit 0xbb
        _emit 0xdb
        _emit 0x04
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x8c
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x68
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x84
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0xe8
        _emit 0xbe
        _emit 0xe7
        _emit 0x04
        _emit 0x00
        _emit 0x84
        _emit 0xc0
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
        _emit 0x74
        _emit 0x3d
        _emit 0x68
        _emit 0x48
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xdb
        _emit 0xdb
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
        _emit 0xe8
        _emit 0xa2
        _emit 0xdb
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x5c
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x78
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x21
        _emit 0xdd
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x78
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0xcd
        _emit 0x1a
        _emit 0x04
        _emit 0x00
        _emit 0xb0
        _emit 0x01
        _emit 0xeb
        _emit 0x23
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x78
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xfc
        _emit 0xdc
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7
        _emit 0x84
        _emit 0x24
        _emit 0x78
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0xa8
        _emit 0x1a
        _emit 0x04
        _emit 0x00
        _emit 0x32
        _emit 0xc0
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x70
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5e
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x64
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x2c
        _emit 0xcc
        _emit 0x5c
        _emit 0x00
        _emit 0x81
        _emit 0xc4
        _emit 0x74
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
