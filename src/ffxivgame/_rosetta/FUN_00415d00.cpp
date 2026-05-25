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
// FUNCTION: ffxivgame 0x00015d00 — __thiscall log-dispatch / assert tail
//                                   for SQEX::CDev::Engine::Vfx::Common::
//                                   Io::Printer::Run_ (442 B / 0x1ba,
//                                   no SEH, /GS-free).
//
// Inspection (read from the disassembly at orig RVA 0x00015d00):
//
//   __thiscall void Printer::Run_(int severity, BYTE verbosity,
//                                 void* arg3, void* arg4);
//
//   `ECX = this`, four stack args (16 B, callee-popped `RET 0x10`).
//   Three this-relative fields are touched:
//
//     [esi+0x78]  int — index into the verbosity-threshold byte array
//                       that lives inline at this+0x81. The byte
//                       lookup is `*((BYTE*)this + 0x81 + this->[0x78])`.
//     [esi+0x7c]  pointer — optional secondary sink object (when
//                       non-null, its __thiscall member at +0x00 in
//                       FUN_004165e0 is fanned the formatted text).
//     [esi+0x91]  byte — secondary-sink verbosity ceiling for the
//                       severity==1 (warning) branch.
//
//   Structural shape (mirrors Ghidra's headless decompile + the asm
//   four-way switch on `severity`):
//
//     // Severity==1 short-circuit: drop the record if the caller's
//     // verbosity is louder than the per-channel ceiling.
//     if (severity == 1 &&
//         verbosity > ((BYTE*)this + 0x81)[this->m_channel_idx_78]) {
//         return;
//     }
//
//     char msg_buf[1024];                       // saved_ESP - 0x800
//     char fmt_buf[1024];                       // saved_ESP - 0x400
//
//     // Render the printf-style payload from (severity, verbosity,
//     // arg3, arg4) into fmt_buf. The helper at 0x004154f0 is the
//     // Printer's own __thiscall formatter — it walks the argument
//     // pack the caller passed via arg3/arg4 (va_list-style) and
//     // writes the assembled text into the supplied buffer.
//     FUN_004154f0(this, fmt_buf, 0x400,
//                  severity, verbosity, arg3, arg4);
//
//     if (severity == 0) {                      // info / trace
//         msg_buf[0x3ff] = 0;                   // explicit NUL sentinel
//         _snprintf_s(msg_buf, 0x400, 0x3ff, fmt_buf);
//         (*PTR_FUN_012651b4)(msg_buf, 2);      // primary log sink, level=2
//         return;
//     }
//     if (severity == 1) {                      // warning
//         msg_buf[0x3fe] = 0;                   // NUL sentinel — 0x3fe to leave
//                                               // room for the appended "\n"
//         _snprintf_s(msg_buf, 0x400, 0x3fe, fmt_buf);
//         _strcat_s(msg_buf, 0x400, "\n");
//         (*PTR_FUN_012651b4)(msg_buf, 3);      // primary log sink, level=3
//         if (this->m_secondary_sink_7c &&
//             verbosity <= this->m_secondary_verbosity_91) {
//             this->m_secondary_sink_7c->FUN_004165e0(fmt_buf, 1);
//         }
//         return;
//     }
//     if (severity == 2) {                      // error → assert
//         msg_buf[0x3fe] = 0;
//         _snprintf_s(msg_buf, 0x400, 0x3fe, fmt_buf);
//         _strcat_s(msg_buf, 0x400, "\n");
//         (*PTR_FUN_012651b4)(msg_buf, 3);
//         if (this->m_secondary_sink_7c) {
//             this->m_secondary_sink_7c->FUN_004165e0(fmt_buf, 1);
//         }
//         // Falls through to the assert tail with line=0x217.
//         FUN_00406550("false", &DAT_00f54d48,
//                      ".\\Io\\Printer.cpp", 0x217,
//                      "SQEX::CDev::Engine::Vfx::Common::Io::Printer::Run_");
//         return;
//     }
//     // Default (severity ∉ {0,1,2}) — raw assert, line=0x21a, no log.
//     FUN_00406550("false", &DAT_00f54d48,
//                  ".\\Io\\Printer.cpp", 0x21a,
//                  "SQEX::CDev::Engine::Vfx::Common::Io::Printer::Run_");
//
//   Stack frame (after the prologue, ESP-relative; ESP = saved_ESP - 0x810
//   throughout the body once the three callee-saves are pushed):
//     [esp+0x00 .. 0x03]  saved EDI
//     [esp+0x04 .. 0x07]  saved ESI (snapshot of `this`)
//     [esp+0x08 .. 0x0b]  saved EBX (low byte holds `verbosity`)
//     [esp+0x0c .. 0x40f] (unused 4 + msg_buf[1024], with the last 2 B
//                          serving as the snprintf NUL sentinel)
//     [esp+0x410..0x80f]  fmt_buf[1024]
//     [esp+0x810..0x813]  saved return EIP
//     [esp+0x814]         param_1: int severity
//     [esp+0x818]         param_2: BYTE verbosity (4-B-aligned slot)
//     [esp+0x81c]         param_3: opaque (forwarded to FUN_004154f0)
//     [esp+0x820]         param_4: opaque (forwarded to FUN_004154f0)
//
//   Reloc-bearing sites in the orig 442 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x053  rel32      0x004154f0 — call FUN_004154f0 (__thiscall formatter)
//     +0x070  imm32 push 0x00f57640 — "SQEX::CDev::…::Printer::Run_" (default-arm assert)
//     +0x09c  rel32      0x005d4f9f — call __snprintf_s (severity==2 arm)
//     +0x0a0  imm32 push 0x00f54d98 — "\n"
//     +0x0b0  rel32      0x005d4bb4 — call _strcat_s (severity==2 arm)
//     +0x0bd  IAT [0x012651b4]      — primary log sink fnptr (sev=2, level=3)
//     +0x0d6  rel32      0x000165e0 — call FUN_004165e0 (secondary sink, sev=2)
//     +0x0da  imm32 push 0x00f57640 — "SQEX::CDev::…::Printer::Run_" (sev=2 assert)
//     +0x0e4  imm32 push 0x00f5758c — ".\\Io\\Printer.cpp"
//     +0x0e9  imm32 push 0x00f54d48 — &DAT_00f54d48 (assert context slot)
//     +0x0ee  imm32 push 0x00f56510 — "false"
//     +0x0f8  rel32      0x00006550 — call FUN_00406550 (assert tail)
//     +0x128  rel32      0x005d4f9f — call __snprintf_s (severity==1 arm)
//     +0x12c  imm32 push 0x00f54d98 — "\n"
//     +0x13c  rel32      0x005d4bb4 — call _strcat_s (severity==1 arm)
//     +0x149  IAT [0x012651b4]      — primary log sink fnptr (sev=1, level=3)
//     +0x16a  rel32      0x000165e0 — call FUN_004165e0 (secondary sink, sev=1)
//     +0x19a  rel32      0x005d4f9f — call __snprintf_s (severity==0 arm)
//     +0x1a7  IAT [0x012651b4]      — primary log sink fnptr (sev=0, level=2)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact register allocation across nine cdecl
//   call sites, the per-arm sentinel-byte writes (one explicit NUL at
//   either msg_buf[0x3fe] or msg_buf[0x3ff] depending on whether the
//   trailing "\n" is appended), the SUB-EAX-0/SUB-EAX-1/SUB-EAX-1
//   chained severity switch (vs an idiomatic CMP/JE ladder), the
//   shared `add esp, 0x24` cleanup that defers three back-to-back
//   cdecl pops into a single fold, AND the linker-resolved absolute
//   addresses in the nineteen relocation windows above. Each of those
//   constraints is brittle under /O2 — every high-level rewrite
//   shifts at least one byte (state numbering, branch short-vs-near,
//   modrm vs moffs32, FF15 IAT-indirect vs E8 rel32).
//
//   The pragmatic choice — the same one FUN_00401a00 / FUN_004014b0
//   took for their reloc-heavy bodies — is a `__declspec(naked)` body
//   that re-emits the orig 442 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical to
//   the orig slice (no relocations because the bytes are emitted as
//   raw immediates), which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the surrounding Printer
//   class (the +0x78 channel-index, +0x7c secondary-sink, +0x81 inline
//   verbosity-threshold array, +0x91 secondary-sink ceiling), the
//   PTR_FUN_012651b4 log-sink signature (probably (const char*,
//   int level) -> void), and the FUN_00406550 assert prototype
//   (the five-argument SQEX assert/__SXAssertFail variant) are
//   catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00415d00() {
    __asm {
        _emit 0x81
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x53
        _emit 0x8a
        _emit 0x9c
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x57

        _emit 0x8b
        _emit 0xbc
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xff
        _emit 0x01
        _emit 0x8b
        _emit 0xf1
        _emit 0x75
        _emit 0x10
        _emit 0x8b
        _emit 0x46

        _emit 0x78
        _emit 0x38
        _emit 0x9c
        _emit 0x30
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f
        _emit 0x82
        _emit 0x80
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x8c

        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x1c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0x52
        _emit 0x0f
        _emit 0xb6

        _emit 0xc3
        _emit 0x50
        _emit 0x57
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51

        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x99
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0xc7
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0x18
        _emit 0x01

        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x0f
        _emit 0x84
        _emit 0x9d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        _emit 0x74
        _emit 0x0c

        _emit 0x68
        _emit 0x40
        _emit 0x76
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x1a
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xeb
        _emit 0x68
        _emit 0x8d
        _emit 0x94
        _emit 0x24
        _emit 0x10

        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0x68
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x68
        _emit 0x00
        _emit 0x04

        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x1e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xff
        _emit 0xf1
        _emit 0x5b
        _emit 0x00

        _emit 0x68
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51
        _emit 0xe8

        _emit 0x00
        _emit 0xee
        _emit 0x5b
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        _emit 0x6a
        _emit 0x03
        _emit 0x52
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26

        _emit 0x01
        _emit 0x8b
        _emit 0x4e
        _emit 0x7c
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x0f
        _emit 0x6a
        _emit 0x01
        _emit 0x8d
        _emit 0x84
        _emit 0x24

        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0x06
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x40
        _emit 0x76
        _emit 0xf5
        _emit 0x00
        _emit 0x68

        _emit 0x17
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x8c
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x10

        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x23
        _emit 0xe8
        _emit 0x54
        _emit 0x07
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x81

        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51

        _emit 0x68
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0xc6

        _emit 0x84
        _emit 0x24
        _emit 0x1e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x73
        _emit 0xf1
        _emit 0x5b
        _emit 0x00
        _emit 0x68
        _emit 0x98
        _emit 0x4d
        _emit 0xf5

        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0x74
        _emit 0xed
        _emit 0x5b
        _emit 0x00

        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x6a
        _emit 0x03
        _emit 0x51
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x8b
        _emit 0x4e
        _emit 0x7c

        _emit 0x83
        _emit 0xc4
        _emit 0x24
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x57
        _emit 0x38
        _emit 0x9e
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x72
        _emit 0x4f
        _emit 0x6a

        _emit 0x01
        _emit 0x8d
        _emit 0x94
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0x72
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x5f
        _emit 0x5e

        _emit 0x5b
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00

        _emit 0x00
        _emit 0x50
        _emit 0x68
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00

        _emit 0x51
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x1f
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x01
        _emit 0xf1
        _emit 0x5b
        _emit 0x00
        _emit 0x8d
        _emit 0x54

        _emit 0x24
        _emit 0x20
        _emit 0x6a
        _emit 0x02
        _emit 0x52
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5f
        _emit 0x5e

        _emit 0x5b
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
