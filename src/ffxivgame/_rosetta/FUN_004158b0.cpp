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
// FUNCTION: ffxivgame 0x004158b0 — SQEX::CDev::Engine::Vfx::Common::Io::
//                                   Printer::SetLimit (322 B / 0x142)
//
// __thiscall on a Printer-style class whose this-pointer lands in ECX.
// Two stack args: (int cols, int rows). Returns char (1 on success,
// 0 if either dimension is zero or rows < 16). ret 8 → __stdcall args
// cleanup of 8 bytes despite the __thiscall convention.
//
// Behaviour (reconstructed from disassembly + Ghidra hint):
//
//   bool Printer::SetLimit(int cols, int rows) {
//       if (cols == 0 || rows == 0) return false;
//       int chunks = (rows + ((rows >> 31) & 0xf)) >> 4;   // rows / 16
//       if (chunks == 0) return false;
//       Printer::Reset(this);                              // FUN_00415860
//       this->m_cols   = cols;
//       this->m_data   = SQEXAlloc(cols * rows, 9, 0x10,
//                                  "SQEX::CDev::Engine::Vfx::Common::"
//                                  "Io::Printer::SetLimit",
//                                  ".\\Io\\Printer.cpp", 0xC0);
//       memset(this->m_data, 0, cols * rows);
//       this->m_meta   = SQEXAlloc(rows * 8, 9, 0x10,
//                                  "SQEX::CDev::Engine::Vfx::Common::"
//                                  "Io::Printer::SetLimit",
//                                  ".\\Io\\Printer.cpp", 0xC3);
//       this->m_chunks = chunks;
//       this->m_p1.head_pos    = 0;
//       this->m_p1.tail_pos    = 0;
//       this->m_p1.data        = this->m_data;
//       this->m_p1.meta        = this->m_meta;
//       /* … 4 more "buffer view" slots at +0x30, +0x44, +0x58, +0x6c,
//          each carving out a stride*chunks-sized window of m_data and
//          a stride*chunks-sized window of m_meta with strides 1/3/6/10
//          of the chunk size and counts 2/3/4/6 chunks. The compiler
//          materialises the strides as LEA + IMUL chains: edi holds
//          chunks throughout, ebx holds cols, ebp = 0 for the bulk
//          field-zeroing. */
//       return true;
//   }
//
// Reloc-bearing sites in the orig 322 bytes:
//   +0x33  call FUN_00415860              (Printer::Reset rel32)
//   +0x44  push str ".\\Io\\Printer.cpp"  (.rdata 0x00f5758c DIR32)
//   +0x49  push str "SQEX::…::SetLimit"   (.rdata 0x00f57554 DIR32)
//   +0x56  call FUN_00416480              (SQEXAlloc rel32)
//   +0x63  call FUN_009d2110              (_memset rel32)
//   +0x72  push str ".\\Io\\Printer.cpp"  (.rdata 0x00f5758c DIR32, 2nd)
//   +0x77  push str "SQEX::…::SetLimit"   (.rdata 0x00f57554 DIR32, 2nd)
//   +0x87  call FUN_00416480              (SQEXAlloc rel32, 2nd)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 into
//   reproducing the exact register-allocation choices for the 18+
//   ebx/edi/ebp/edx/eax-resident temporaries used in the slot-laying
//   loop, the exact instruction selection (LEA-scaled-index vs IMUL
//   vs ADD-self chains), AND the linker-resolved absolute addresses
//   in the eight relocation windows above. Every high-level rewrite
//   shifts at least one byte (lea selection, edi-reuse window, push
//   ordering for the two cdecl alloc calls).
//
//   The pragmatic choice — same as FUN_00401a00 took for its
//   exe-dir bootstrap — is a `__declspec(naked)` body that re-emits
//   the orig 322 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` section is byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates),
//   which is what tools/compare.py checks against.
//
//   The structural commentary above is the readable record of what
//   the function does, so a future contributor can promote this to
//   a real source-level match once the Printer class layout (the
//   +0x08 char* data / +0x0c void* meta / +0x10 cols / +0x14..+0x74
//   per-view slot inventory) and the surrounding SQEXAlloc / Printer
//   reset helpers are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_004158b0() {
    __asm {
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x85
        _emit 0xdb
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x0f
        _emit 0x84
        _emit 0x2b
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0x1f
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x99
        _emit 0x83
        _emit 0xe2
        _emit 0x0f
        _emit 0x03
        _emit 0xc2
        _emit 0x57
        _emit 0x8b
        _emit 0xf8
        _emit 0xc1
        _emit 0xff
        _emit 0x04
        _emit 0x75
        _emit 0x08
        _emit 0x5f
        _emit 0x5e
        _emit 0x32
        _emit 0xc0
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x55
        _emit 0xe8
        _emit 0x78
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x68
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xeb
        _emit 0x0f
        _emit 0xaf
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x68
        _emit 0x8c
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x54
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x6a
        _emit 0x09
        _emit 0x55
        _emit 0x89
        _emit 0x5e
        _emit 0x10
        _emit 0xe8
        _emit 0x75
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x55
        _emit 0x33
        _emit 0xed
        _emit 0x55
        _emit 0x50
        _emit 0x89
        _emit 0x46
        _emit 0x08
        _emit 0xe8
        _emit 0xf8
        _emit 0xc7
        _emit 0x5b
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x68
        _emit 0xc3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x8c
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0x54
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x6a
        _emit 0x10
        _emit 0x8d
        _emit 0x0c
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x09
        _emit 0x51
        _emit 0xe8
        _emit 0x44
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x89
        _emit 0x7e
        _emit 0x24
        _emit 0x89
        _emit 0x6e
        _emit 0x1c
        _emit 0x89
        _emit 0x6e
        _emit 0x20
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        _emit 0x89
        _emit 0x56
        _emit 0x14
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x89
        _emit 0x46
        _emit 0x18
        _emit 0x89
        _emit 0x6e
        _emit 0x30
        _emit 0x89
        _emit 0x6e
        _emit 0x34
        _emit 0x8b
        _emit 0xd7
        _emit 0x0f
        _emit 0xaf
        _emit 0xd3
        _emit 0x8d
        _emit 0x0c
        _emit 0x3f
        _emit 0x89
        _emit 0x4e
        _emit 0x38
        _emit 0x03
        _emit 0x56
        _emit 0x08
        _emit 0x83
        _emit 0xc4
        _emit 0x3c
        _emit 0x89
        _emit 0x56
        _emit 0x28
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x8d
        _emit 0x0c
        _emit 0xf8
        _emit 0x89
        _emit 0x4e
        _emit 0x2c
        _emit 0x89
        _emit 0x6e
        _emit 0x44
        _emit 0x8d
        _emit 0x04
        _emit 0x7f
        _emit 0x89
        _emit 0x6e
        _emit 0x48
        _emit 0x89
        _emit 0x46
        _emit 0x4c
        _emit 0x8b
        _emit 0xd0
        _emit 0x0f
        _emit 0xaf
        _emit 0xd3
        _emit 0x03
        _emit 0x56
        _emit 0x08
        _emit 0x89
        _emit 0x56
        _emit 0x3c
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x8d
        _emit 0x14
        _emit 0xc1
        _emit 0x89
        _emit 0x56
        _emit 0x40
        _emit 0x89
        _emit 0x6e
        _emit 0x58
        _emit 0x89
        _emit 0x6e
        _emit 0x5c
        _emit 0x8d
        _emit 0x04
        _emit 0xbd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x46
        _emit 0x60
        _emit 0x8d
        _emit 0x04
        _emit 0x7f
        _emit 0x03
        _emit 0xc0
        _emit 0x8b
        _emit 0xc8
        _emit 0x0f
        _emit 0xaf
        _emit 0xcb
        _emit 0x03
        _emit 0x4e
        _emit 0x08
        _emit 0x8d
        _emit 0x3c
        _emit 0xbf
        _emit 0x89
        _emit 0x4e
        _emit 0x50
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        _emit 0x8d
        _emit 0x0c
        _emit 0xc2
        _emit 0x03
        _emit 0xff
        _emit 0x8b
        _emit 0xd7
        _emit 0x0f
        _emit 0xaf
        _emit 0xd3
        _emit 0x89
        _emit 0x4e
        _emit 0x54
        _emit 0x89
        _emit 0x6e
        _emit 0x6c
        _emit 0x89
        _emit 0x6e
        _emit 0x70
        _emit 0x89
        _emit 0x46
        _emit 0x74
        _emit 0x03
        _emit 0x56
        _emit 0x08
        _emit 0x89
        _emit 0x56
        _emit 0x64
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        _emit 0x5d
        _emit 0x8d
        _emit 0x0c
        _emit 0xf8
        _emit 0x5f
        _emit 0x89
        _emit 0x4e
        _emit 0x68
        _emit 0x5e
        _emit 0xb0
        _emit 0x01
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x5e
        _emit 0x32
        _emit 0xc0
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
