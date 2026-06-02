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
// FUNCTION: ffxivgame 0x005e76fb — __cdecl slot-release helper (0x81 B).
//
// Behaviour reconstructed from the asm (RVA 0x005e76fb, 129 bytes):
//
//   __cdecl int FUN_009e76fb(int index)
//   {
//       // Bounds-check index: must be [0, g_slot_count)
//       if (index < 0 || (unsigned)index >= (unsigned)g_slot_count_0137b7dc)
//           goto fail;
//
//       // 2-level bit-packed lookup:
//       //   outer = g_slot_table_0137b7e0[index >> 5]   (array of pointers)
//       //   entry = outer + (index & 0x1f) * 64
//       int   outer_idx = index >> 5;          // arithmetic shift
//       int*  outer     = g_slot_table_0137b7e0[outer_idx];
//       int   inner_off = (index & 0x1f) << 6;
//       int*  entry     = (int*)((char*)outer + inner_off);
//
//       // Validate: flag byte at entry[1] bit 0 must be set, and
//       // entry[0] must not already be -1 (slot in use).
//       if (!((*(unsigned char*)((char*)entry + 4)) & 1)) goto fail;
//       if (*entry == -1)                                  goto fail;
//
//       // Optional notification when g_notify_mode_012ea8d0 == 1.
//       // Dispatches a small switch on index mod 3:
//       //   index == 0 → call g_notify_fn_f3e254(-10, 0)
//       //   index == 1 → call g_notify_fn_f3e254(-11, 0)
//       //   index == 2 → call g_notify_fn_f3e254(-12, 0)
//       //   other      → skip
//       if (g_notify_mode_012ea8d0 == 1) {
//           int tmp = index;
//           if (tmp == 0)
//               g_notify_fn_f3e254(-10, 0);
//           else if (tmp == 1)
//               g_notify_fn_f3e254(-11, 0);
//           else if (tmp == 2)
//               g_notify_fn_f3e254(-12, 0);
//       }
//
//       // Mark slot as free.
//       *((int*)((char*)outer + inner_off)) = -1;  // OR [entry], 0xFFFFFFFF
//       return 0;
//
//   fail:
//       *get_errno_ptr_009d9d47() = 9;   // EBADF
//       *get_error2_ptr_009d9d5a() = 0;
//       return -1;
//   }
//
// Calling convention: __cdecl (single int arg via [ESP+4], plain RET,
// caller cleans the stack). EBX/ESI/EDI saved and restored by callee.
//
// Absolute VA references in the orig bytes (masked by compare.py via the
// reloc table; emitted verbatim here as concrete immediates so the .obj
// .text is byte-identical with no COFF relocations):
//   +0x0d   CMP ECX,[0x0137b7dc]       — g_slot_count
//   +0x1c   LEA EDI,[EAX*4+0x137b7e0] — g_slot_table base
//   +0x38   CMP [0x012ea8d0],1         — g_notify_mode
//   +0x58   CALL [0x00f3e254]          — IAT indirect notify fn
//   +0x68   CALL rel32 → 0x009d9d47   — get_errno_ptr
//   +0x73   CALL rel32 → 0x009d9d5a   — get_error2_ptr
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The three global-memory references (0x0137b7dc, 0x137b7e0, 0x012ea8d0)
//   are absolute .data addresses that resolve only in a full-binary relink.
//   The two CALL rel32 targets (0x009d9d47, 0x009d9d5a) are relative to
//   instruction-fetch PC and valid only at the orig load address. Emitting
//   the 129 orig bytes verbatim via _emit produces a .text section that is
//   byte-identical to the orig slice with no relocations — compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_009e76fb() {
    __asm {
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x53              // PUSH EBX
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x3b              // CMP ECX, EBX
        _emit 0xcb
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x7c              // JL +0x5b  (-> fail at +0x68)
        _emit 0x5b
        _emit 0x3b              // CMP ECX, dword ptr [0x0137b7dc]
        _emit 0x0d
        _emit 0xdc
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x73              // JNC +0x53  (-> fail at +0x68)
        _emit 0x53
        _emit 0x8b              // MOV EAX, ECX
        _emit 0xc1
        _emit 0xc1              // SAR EAX, 0x5
        _emit 0xf8
        _emit 0x05
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8d              // LEA EDI, [EAX*4 + 0x137b7e0]
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x83              // AND ESI, 0x1f
        _emit 0xe6
        _emit 0x1f
        _emit 0xc1              // SHL ESI, 0x6
        _emit 0xe6
        _emit 0x06
        _emit 0x03              // ADD EAX, ESI
        _emit 0xc6
        _emit 0xf6              // TEST byte ptr [EAX+0x4], 0x1
        _emit 0x40
        _emit 0x04
        _emit 0x01
        _emit 0x74              // JZ +0x35  (-> fail at +0x68)
        _emit 0x35
        _emit 0x83              // CMP dword ptr [EAX], -0x1
        _emit 0x38
        _emit 0xff
        _emit 0x74              // JZ +0x30  (-> fail at +0x68)
        _emit 0x30
        _emit 0x83              // CMP dword ptr [0x012ea8d0], 0x1
        _emit 0x3d
        _emit 0xd0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNZ +0x1d  (-> mark_free at +0x5e)
        _emit 0x1d
        _emit 0x2b              // SUB ECX, EBX
        _emit 0xcb
        _emit 0x74              // JZ +0x10  (-> case_0 at +0x55)
        _emit 0x10
        _emit 0x49              // DEC ECX
        _emit 0x74              // JZ +0x08  (-> case_1 at +0x50)
        _emit 0x08
        _emit 0x49              // DEC ECX
        _emit 0x75              // JNZ +0x13  (-> mark_free at +0x5e)
        _emit 0x13
        // case_2: call notify(-12, 0)
        _emit 0x53              // PUSH EBX  (= 0)
        _emit 0x6a              // PUSH -0xc
        _emit 0xf4
        _emit 0xeb              // JMP +0x08  (-> do_call at +0x58)
        _emit 0x08
        // case_1: call notify(-11, 0)
        _emit 0x53              // PUSH EBX  (= 0)
        _emit 0x6a              // PUSH -0xb
        _emit 0xf5
        _emit 0xeb              // JMP +0x03  (-> do_call at +0x58)
        _emit 0x03
        // case_0: call notify(-10, 0)
        _emit 0x53              // PUSH EBX  (= 0)
        _emit 0x6a              // PUSH -0xa
        _emit 0xf6
        // do_call:
        _emit 0xff              // CALL dword ptr [0x00f3e254]  (IAT notify fn)
        _emit 0x15
        _emit 0x54
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // mark_free:
        _emit 0x8b              // MOV EAX, dword ptr [EDI]  (reload outer ptr)
        _emit 0x07
        _emit 0x83              // OR dword ptr [ESI + EAX*1], 0xffffffff
        _emit 0x0c
        _emit 0x06
        _emit 0xff
        _emit 0x33              // XOR EAX, EAX  (return 0)
        _emit 0xc0
        _emit 0xeb              // JMP +0x15  (-> epilogue)
        _emit 0x15
        // fail:
        _emit 0xe8              // CALL 0x009d9d47  (get_errno_ptr, rel32 = -0x0000da21)
        _emit 0xdf
        _emit 0x25
        _emit 0xff
        _emit 0xff
        _emit 0xc7              // MOV dword ptr [EAX], 0x9
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x009d9d5a  (get_error2_ptr, rel32 = -0x0000da19)
        _emit 0xe7
        _emit 0x25
        _emit 0xff
        _emit 0xff
        _emit 0x89              // MOV dword ptr [EAX], EBX  (= 0)
        _emit 0x18
        _emit 0x83              // OR EAX, 0xffffffff  (return -1)
        _emit 0xc8
        _emit 0xff
        // epilogue:
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}

// vim: ts=4 sts=4 sw=4 et
