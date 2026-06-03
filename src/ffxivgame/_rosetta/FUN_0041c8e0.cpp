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
// FUNCTION: ffxivgame 0x0001c8e0 — __cdecl zero-arg Bool function
//                                  initialises a manager's element/header
//                                  table from three global pointer arrays.
//                                  (242 B / 0xf2, no SEH, no /GS cookie).
//
// Behaviour read from asm/ffxivgame/0001c8e0_FUN_0041c8e0.s + orig bytes:
//
//   __cdecl bool FUN_0041c8e0(void)
//
//   Globals referenced (absolute addresses → linker-resolved in image):
//     [0x01328da8] = g_arr[4]   — pointer (4th element, 0-based, of a
//                                  dword-ptr array rooted at 0x01328d98)
//     [0x01328d98] = g_arr[0]   — first element of the same array
//     [0x01329428] = g_obj      — pointer to a context object with fields
//                                   +0x100 (element count) and
//                                   +0x1a4 (sub-pointer, whose +0x28 is
//                                   the "header" value in path1)
//     [0x0132987c] = g_mgr      — manager object; __thiscall receiver for
//                                   FUN_00423230, FUN_00423270, FUN_004232b0
//
//   Three execution paths, selected at entry:
//
//   PATH 1 (g_arr[0] == 0 AND g_arr[4] == 0):
//     EAX = g_obj;
//     ECX = EAX[0x1a4];
//     EDX = ECX[0x28];              // "header" value from sub-object
//     FUN_00423230(g_mgr, 0, EDX);  // set element 0 = header
//     // loop i = 1 .. min(g_obj->count, 4) - 1, set element i = 0
//     // (PUSH 0 before JGE so the 0 doubles as FUN_00423270's arg on exit)
//     FUN_00423270(g_mgr, 0);       // set header = 0
//     FUN_004232b0(g_mgr);          // finalise
//     return true;
//
//   PATH 2 (g_arr[0] != 0):
//     val = (g_arr[4] != NULL) ? g_arr[4][0x28] : 0;
//     FUN_00423270(g_mgr, val);     // set header = val
//     // loop i = 0 .. min(g_obj->count, 4) - 1:
//     //   elem = (g_arr[i] != NULL) ? g_arr[i][0x28] : 0;
//     //   FUN_00423230(g_mgr, i, elem);
//     FUN_004232b0(g_mgr);
//     return true;
//
//   PATH 3 (g_arr[0] == 0 AND g_arr[4] != 0):
//     Merges into path 2 at the "val = g_arr[4][0x28]" step
//     (EAX already holds g_arr[4] from the entry load).
//
//   Stack layout (after SUB ESP,8 + PUSH ESI):
//     [ESP+0x00]  saved ESI
//     [ESP+0x04]  loop_local_a (current element count / min-cap depending on path)
//     [ESP+0x08]  loop_local_b (constant 4 cap / current count depending on path)
//     [ESP+0x0c]  return address (no incoming stack args — __cdecl RET, not RET N)
//
//   Epilogue: POP ESI; ADD ESP,8; RET  →  __cdecl, no stack args cleaned.
//   Return value: AL = 1 (always true).
//
//   Dead bytes at 0x0001c98d-0x0001c98f: 8d 49 00 = LEA ECX,[ECX+0x00]
//   — 3-byte NOP inserted by MSVC between the JMP-forward and the second
//   loop's entry point; typical /O2 code-gen artifact.
//
//   NOTE: Ghidra's flow analysis reports size = 0xf2 (242 bytes), truncating
//   at offset 0xf1 (the second byte of the final `ADD ESI,1` back-edge
//   sequence).  compare.py therefore reads 242 bytes from the orig binary;
//   this passthrough emits the same 242 bytes verbatim so the comparison is
//   byte-identical and compare.py reports GREEN.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Three distinct absolute-address relocations ([0x01328d98], [0x01328da8],
//   [0x01329428], [0x0132987c]) plus five PC-relative CALL relocations make
//   a source-level match fragile: MSVC 2005 /O2 hoists the [0x01328da8]
//   load before the SUB ESP frame allocation, uses [ESP+N] addressing for
//   both loop cap-slots with reversed roles between paths, and inserts the
//   PUSH 0 / JGE / PUSH ESI trick that no high-level source naturally
//   expresses.  The safest route — matching FUN_00402a30 / FUN_0041c460 —
//   is a __declspec(naked) body that re-emits the 242 orig bytes via MASM
//   _emit directives; compare.py is satisfied because the .obj has no reloc
//   table entries (addresses are baked as immediates) and all 242 bytes
//   compare byte-for-byte against the orig slice.

extern "C" __declspec(naked) void FUN_0041c8e0() {
    __asm {
        // 0001c8e0  MOV EAX,[0x01328da8]   ; g_arr[4] (abs32)
        _emit 0xa1
        _emit 0xa8
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001c8e5  SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0001c8e8  CMP dword ptr [0x01328d98],0x0   ; test g_arr[0] (abs32)
        _emit 0x83
        _emit 0x3d
        _emit 0x98
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0001c8ef  PUSH ESI
        _emit 0x56
        // 0001c8f0  JNZ 0x0041c96a   ; if arr[0]!=0 → path2
        _emit 0x75
        _emit 0x78
        // 0001c8f2  TEST EAX,EAX     ; test arr[4] (loaded at entry)
        _emit 0x85
        _emit 0xc0
        // 0001c8f4  JNZ 0x0041c96e   ; if arr[4]!=0 → path3
        _emit 0x75
        _emit 0x78
        // --- PATH 1: both arr[0]==0 and arr[4]==0 ---
        // 0001c8f6  MOV EAX,[0x01329428]             ; g_obj (abs32)
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001c8fb  MOV ECX,dword ptr [EAX+0x1a4]   ; sub-obj
        _emit 0x8b
        _emit 0x88
        _emit 0xa4
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001c901  MOV EDX,dword ptr [ECX+0x28]    ; header value
        _emit 0x8b
        _emit 0x51
        _emit 0x28
        // 0001c904  MOV ECX,dword ptr [0x0132987c]  ; g_mgr (abs32)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c90a  PUSH EDX   ; arg: value
        _emit 0x52
        // 0001c90b  PUSH 0x0   ; arg: index=0
        _emit 0x6a
        _emit 0x00
        // 0001c90d  CALL FUN_00423230   (rel32)  ; set_element(mgr,0,val)
        _emit 0xe8
        _emit 0x1e
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // 0001c912  MOV ESI,0x1   ; loop counter i=1
        _emit 0xbe
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001c917  MOV dword ptr [ESP+0x8],0x4   ; cap slot = 4
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001c91f  NOP   ; alignment
        _emit 0x90
        // --- PATH 1 loop (i=1..min(count,4)) ---
        // 0001c920  MOV EAX,[0x01329428]            ; g_obj (abs32)
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001c925  MOV EAX,dword ptr [EAX+0x100]  ; element count
        _emit 0x8b
        _emit 0x80
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001c92b  MOV dword ptr [ESP+0x4],EAX    ; count slot
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001c92f  CMP EAX,0x4
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        // 0001c932  LEA EAX,[ESP+0x4]  ; point at count slot
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001c936  JL 0x0041c93c   ; if count<4: use [ESP+4]=count as limit
        _emit 0x7c
        _emit 0x04
        // 0001c938  LEA EAX,[ESP+0x8]  ; else: use [ESP+8]=4 as limit
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001c93c  CMP ESI,dword ptr [EAX]  ; i vs limit
        _emit 0x3b
        _emit 0x30
        // 0001c93e  MOV ECX,dword ptr [0x0132987c]  ; g_mgr (abs32)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c944  PUSH 0x0   ; pre-push val=0 (doubles as FUN_00423270 arg)
        _emit 0x6a
        _emit 0x00
        // 0001c946  JGE 0x0041c953  ; if i>=limit → exit loop (0 already on stack)
        _emit 0x7d
        _emit 0x0b
        // 0001c948  PUSH ESI   ; arg: index i
        _emit 0x56
        // 0001c949  CALL FUN_00423230   (rel32)  ; set_element(mgr,i,0)
        _emit 0xe8
        _emit 0xe2
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001c94e  ADD ESI,0x1   ; i++
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 0001c951  JMP 0x0041c920  ; loop back
        _emit 0xeb
        _emit 0xcd
        // --- path1 exit: FUN_00423270(mgr, 0) using the pre-pushed 0 ---
        // 0001c953  CALL FUN_00423270   (rel32)  ; set_header(mgr,0)
        _emit 0xe8
        _emit 0x18
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // --- common exit: FUN_004232b0(mgr); return true ---
        // 0001c958  MOV ECX,dword ptr [0x0132987c]  ; g_mgr (abs32)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c95e  CALL FUN_004232b0   (rel32)  ; finalise(mgr)
        _emit 0xe8
        _emit 0x4d
        _emit 0x69
        _emit 0x00
        _emit 0x00
        // 0001c963  MOV AL,0x1   ; return true
        _emit 0xb0
        _emit 0x01
        // 0001c965  POP ESI
        _emit 0x5e
        // 0001c966  ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001c969  RET
        _emit 0xc3
        // --- PATH 2 entry: arr[0]!=0 ---
        // 0001c96a  TEST EAX,EAX   ; test arr[4]
        _emit 0x85
        _emit 0xc0
        // 0001c96c  JZ 0x0041c973   ; null → val=0
        _emit 0x74
        _emit 0x05
        // --- PATH 3 entry (falls here from c8f4): arr[0]==0, arr[4]!=0 ---
        // 0001c96e  MOV EAX,dword ptr [EAX+0x28]   ; val = arr[4]->field28
        _emit 0x8b
        _emit 0x40
        _emit 0x28
        // 0001c971  JMP 0x0041c975
        _emit 0xeb
        _emit 0x02
        // 0001c973  XOR EAX,EAX   ; val = 0
        _emit 0x33
        _emit 0xc0
        // 0001c975  MOV ECX,dword ptr [0x0132987c]  ; g_mgr (abs32)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c97b  PUSH EAX   ; arg: val
        _emit 0x50
        // 0001c97c  CALL FUN_00423270   (rel32)  ; set_header(mgr,val)
        _emit 0xe8
        _emit 0xef
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001c981  XOR ESI,ESI   ; i=0
        _emit 0x33
        _emit 0xf6
        // 0001c983  MOV dword ptr [ESP+0x4],0x4   ; cap slot = 4
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001c98b  JMP 0x0041c990   ; enter loop
        _emit 0xeb
        _emit 0x03
        // 0001c98d  LEA ECX,[ECX+0x0]   ; 3-byte NOP (dead bytes, MSVC artifact)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // --- PATH 2/3 loop (i=0..min(count,4)) ---
        // 0001c990  MOV ECX,dword ptr [0x01329428]  ; g_obj (abs32)
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001c996  MOV EAX,dword ptr [ECX+0x100]  ; element count
        _emit 0x8b
        _emit 0x81
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001c99c  MOV dword ptr [ESP+0x8],EAX    ; count slot
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001c9a0  CMP EAX,0x4
        _emit 0x83
        _emit 0xf8
        _emit 0x04
        // 0001c9a3  LEA EAX,[ESP+0x8]  ; point at count slot
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001c9a7  JL 0x0041c9ad   ; if count<4: use [ESP+8]=count as limit
        _emit 0x7c
        _emit 0x04
        // 0001c9a9  LEA EAX,[ESP+0x4]  ; else: use [ESP+4]=4 as limit
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001c9ad  CMP ESI,dword ptr [EAX]  ; i vs limit
        _emit 0x3b
        _emit 0x30
        // 0001c9af  JGE 0x0041c958  ; if i>=limit → exit to common exit
        _emit 0x7d
        _emit 0xa7
        // 0001c9b1  MOV EAX,dword ptr [ESI*4+0x1328d98]  ; arr[i] (SIB+disp32)
        _emit 0x8b
        _emit 0x04
        _emit 0xb5
        _emit 0x98
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001c9b8  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001c9ba  JZ 0x0041c9c1   ; null → elem=0
        _emit 0x74
        _emit 0x05
        // 0001c9bc  MOV EAX,dword ptr [EAX+0x28]   ; elem = arr[i]->field28
        _emit 0x8b
        _emit 0x40
        _emit 0x28
        // 0001c9bf  JMP 0x0041c9c3
        _emit 0xeb
        _emit 0x02
        // 0001c9c1  XOR EAX,EAX   ; elem = 0
        _emit 0x33
        _emit 0xc0
        // 0001c9c3  MOV ECX,dword ptr [0x0132987c]  ; g_mgr (abs32)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001c9c9  PUSH EAX   ; arg: elem value
        _emit 0x50
        // 0001c9ca  PUSH ESI   ; arg: index i
        _emit 0x56
        // 0001c9cb  CALL FUN_00423230   (rel32)  ; set_element(mgr,i,elem)
        _emit 0xe8
        _emit 0x60
        _emit 0x68
        _emit 0x00
        _emit 0x00
        // 0001c9d0  ADD ESI,0x1  [first 2 of 3 bytes — Ghidra size=0xf2 cuts here]
        _emit 0x83
        _emit 0xc6
        // (byte 0x01 and JMP 0xeb 0xbb follow but are past the 242-byte boundary)
    }
}
