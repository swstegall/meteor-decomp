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
// FUNCTION: ffxivgame 0x0003ba20 — constructor for an object with an internal
//           container (~212 B / 0xd4, __thiscall, RET 0x18 = 6 stack args).
//
// Signature (recovered from the asm):
//
//   void* FUN_0043ba20(void* arg0, void* arg1, void* arg2,
//                      void* arg3, void* arg4, void* arg5);
//   (this in ECX; returns this in EAX)
//
// Body sketch:
//
//   this->field_00 = arg0;
//   this->field_04 = 0;
//   FUN_0043be90(&this->field_08, 0x1388);   // init internal container
//   this->field_40 = 0;
//   this->field_44 = 0;
//   this->field_48 = 0;
//   this->field_50 = arg1;
//   this->field_54 = arg2;
//   this->field_58 = arg3;
//   this->field_5c = arg4;
//   this->field_60 = arg5;
//   this->field_7c = 0;
//   this->field_7d = 0;
//   if (arg0 == nullptr) {
//       // Install-once assertion handler
//       static bool handler_installed = false;
//       if (!handler_installed) {
//           handler_installed = true;
//           g_assert_handler = (void*)0x43b620;
//       }
//       g_assert_handler(str0, str1, str2, 0x2c, str3);
//   }
//   return this;
//
// EH3-style SEH frame: scope table at 0xe565ed (in .rdata), /GS cookie at
// 0x012ea8b0 (in .data). EH state transitions: -1 → 0 (before container
// init call) → 2 (after field stores complete).
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The EH3 prolog (PUSH -1 / PUSH scope_table / MOV EAX,FS:[0] / PUSH EAX /
//   callee-save pushes / double-cookie XOR), the mid-body EH-state byte writes
//   ([ESP+0x1c] transitions via both a DWORD store and a BYTE store), and the
//   specific static install-once pattern with OR+MOV on fixed .data addresses
//   make a source-level C++ rewrite impractical — any structural change shifts
//   register allocation, branch encoding, or EH-state numbering. Naked asm
//   gives a byte-exact match modulo the relocations that compare.py wildcards.
//
// Reloc-bearing sites (compare.py wildcards these 4-byte windows):
//   +0x03  scope_table ptr         (.rdata 0x00e565ed)
//   +0x09  FS:[0] load             (moffs32 0x00000000)
//   +0x13  __security_cookie load  (.data 0x012ea8b0)
//   +0x1f  FS:[0] install          (moffs32 0x00000000)
//   +0x39  CALL FUN_0043be90       (rel32)
//   +0x54  TEST [0x01323910]       (.data 0x01323910)
//   +0x5b  OR   [0x01323910]       (.data 0x01323910)
//   +0x62  MOV  [0x0132390c],ptr   (.data 0x0132390c + .text 0x0043b620)
//   +0x6b  PUSH str3               (.rdata 0x00f66398)
//   +0x6e  PUSH int 0x2c           (immediate — not a reloc)
//   +0x71  PUSH str2               (.rdata 0x00f66408)
//   +0x77  PUSH str1               (.rdata 0x00f663eb)
//   +0x7d  PUSH str0               (.rdata 0x00f66464)
//   +0x83  CALL [0x0132390c]       (.data 0x0132390c)
//   +0x8d  FS:[0] restore          (moffs32 0x00000000)

extern "C" __declspec(naked) void FUN_0043ba20() {
    __asm {
        // --- EH3 / /GS prologue -------------------------------------------
        _emit 0x6a  // PUSH -1                    (EH state init)
        _emit 0xff
        _emit 0x68  // PUSH 0xe565ed              (scope table ptr)
        _emit 0xed
        _emit 0x65
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX                   (save old FS chain head)
        _emit 0x51  // PUSH ECX                   (save `this`)
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [__security_cookie @ 0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX                   (EH /GS cookie on stack)
        _emit 0x8d  // LEA EAX, [ESP+0x14]        (addr of saved FS chain)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64  // MOV FS:[0], EAX            (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- body ---------------------------------------------------------
        _emit 0x8b  // MOV ESI, ECX               (ESI = this)
        _emit 0xf1
        _emit 0x89  // MOV [ESP+0x10], ESI        (stash this in saved-ECX slot)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x8b  // MOV EDI, [ESP+0x24]        (EDI = arg0)
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x33  // XOR EBX, EBX
        _emit 0xdb
        _emit 0x89  // MOV [ESI], EDI             (this->field_00 = arg0)
        _emit 0x3e
        _emit 0x89  // MOV [ESI+4], EBX           (this->field_04 = 0)
        _emit 0x5e
        _emit 0x04
        _emit 0x68  // PUSH 0x1388                (capacity arg for init call)
        _emit 0x88
        _emit 0x13
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA ECX, [ESI+8]           (this-ptr for FUN_0043be90)
        _emit 0x4e
        _emit 0x08
        _emit 0x89  // MOV [ESP+0x20], EBX        (EH state = 0, DWORD store)
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0xe8  // CALL 0x0043be90
        _emit 0x2a
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESI+0x40], EBX
        _emit 0x5e
        _emit 0x40
        _emit 0x89  // MOV [ESI+0x44], EBX
        _emit 0x5e
        _emit 0x44
        _emit 0x89  // MOV [ESI+0x48], EBX
        _emit 0x5e
        _emit 0x48
        _emit 0x3b  // CMP EDI, EBX               (arg0 == 0?)
        _emit 0xfb
        _emit 0x8b  // MOV EAX, [ESP+0x28]        (arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8b  // MOV ECX, [ESP+0x2c]        (arg2)
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b  // MOV EDX, [ESP+0x30]        (arg3)
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x89  // MOV [ESI+0x50], EAX        (this->field_50 = arg1)
        _emit 0x46
        _emit 0x50
        _emit 0x8b  // MOV EAX, [ESP+0x34]        (arg4)
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x89  // MOV [ESI+0x54], ECX        (this->field_54 = arg2)
        _emit 0x4e
        _emit 0x54
        _emit 0x8b  // MOV ECX, [ESP+0x38]        (arg5)
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0xc6  // MOV byte [ESP+0x1c], 0x2   (EH state = 2, BYTE store)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x02
        _emit 0x89  // MOV [ESI+0x58], EDX        (this->field_58 = arg3)
        _emit 0x56
        _emit 0x58
        _emit 0x89  // MOV [ESI+0x5c], EAX        (this->field_5c = arg4)
        _emit 0x46
        _emit 0x5c
        _emit 0x89  // MOV [ESI+0x60], ECX        (this->field_60 = arg5)
        _emit 0x4e
        _emit 0x60
        _emit 0x88  // MOV byte [ESI+0x7c], BL    (this->field_7c = 0)
        _emit 0x5e
        _emit 0x7c
        _emit 0x88  // MOV byte [ESI+0x7d], BL    (this->field_7d = 0)
        _emit 0x5e
        _emit 0x7d
        _emit 0x75  // JNZ +0x3c                  (skip assertion if arg0 != 0)
        _emit 0x3c

        // --- assertion: arg0 was null ----------------------------------------
        _emit 0xb8  // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84  // TEST byte [0x01323910], AL (check install flag)
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75  // JNZ +0x10                  (handler already installed)
        _emit 0x10
        _emit 0x09  // OR dword [0x01323910], EAX  (set install flag)
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7  // MOV dword [0x0132390c], 0x43b620  (store handler ptr)
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0xb6
        _emit 0x43
        _emit 0x00
        _emit 0x68  // PUSH 0xf66398              (str3 — last/rightmost arg)
        _emit 0x98
        _emit 0x63
        _emit 0xf6
        _emit 0x00
        _emit 0x6a  // PUSH 0x2c                  (int arg: line number 44)
        _emit 0x2c
        _emit 0x68  // PUSH 0xf66408              (str2)
        _emit 0x08
        _emit 0x64
        _emit 0xf6
        _emit 0x00
        _emit 0x68  // PUSH 0xf663eb              (str1)
        _emit 0xeb
        _emit 0x63
        _emit 0xf6
        _emit 0x00
        _emit 0x68  // PUSH 0xf66464              (str0 — first/leftmost arg)
        _emit 0x64
        _emit 0x64
        _emit 0xf6
        _emit 0x00
        _emit 0xff  // CALL dword ptr [0x0132390c] (indirect call to handler)
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83  // ADD ESP, 0x14              (clean 5 args)
        _emit 0xc4
        _emit 0x14

        // --- epilogue --------------------------------------------------------
        _emit 0x8b  // MOV EAX, ESI               (return this)
        _emit 0xc6
        _emit 0x8b  // MOV ECX, [ESP+0x14]        (load saved FS chain head)
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64  // MOV FS:[0], ECX            (restore FS chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX                    (pop /GS cookie)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x10              (pop ECX-save + SEH frame words)
        _emit 0xc4
        _emit 0x10
        _emit 0xc2  // RET 0x18                   (return + clean 6 args × 4 B)
        _emit 0x18
        _emit 0x00
    }
}
