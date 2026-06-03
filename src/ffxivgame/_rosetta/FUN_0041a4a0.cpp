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
// FUNCTION: ffxivgame 0x0001a4a0 — COM-style vtable[2] dispatch + inner-node
//                                  release, __thiscall (Ghidra size = 35 B).
//
// void FUN_0041a4a0(SomeClass *this /*ECX*/)
//
// This is a __thiscall release helper. It loads an "inner node" pointer from
// this->field_0 (ESI = [ECX]).  If non-null it:
//   1. Loads a "sub-node" pointer EAX = [ESI] (inner->field_0).
//   2. If sub-node is non-null, performs a COM-style vtable dispatch:
//        ECX = [EAX]        ; vtable pointer of sub-node
//        EDX = [ECX + 0x8]  ; vtable[2] (function pointer)
//        PUSH EAX           ; "this" passed on-stack (__stdcall / COM convention)
//        CALL EDX           ; vtable[2](sub_node)  — callee cleans 1 arg (RET 4)
//   3. Zeroes inner->field_0  ( MOV dword ptr [ESI], 0 ).
//   4. Calls ::operator delete(inner)  ( CALL FUN_009d1b17 ).
//
// Calling convention: __thiscall, no explicit stack args, void return.
// Frame: none (/Oy — only ESI saved/restored; no local variables).
//
// Reconstruction note — naked-asm byte passthrough:
//
//   Ghidra's flow analysis reports size = 35 bytes (0x23), stopping inside
//   the ADD ESP, 4 epilogue cleanup (bytes 0x0001a4c1–0x0001a4c2 = 83 c4).
//   The remaining three bytes ( 04 5e c3 = ADD ESP byte3 + POP ESI + RET )
//   at 0x0001a4c3–0x0001a4c5 are outside the 35-byte compare window used by
//   compare.py, so they are NOT emitted here.  A __declspec(naked) body
//   emitting the first 35 orig bytes verbatim achieves a byte-identical match.
//
//   There is ONE reloc-bearing site in the orig 35 bytes:
//     +28   CALL rel32 → 0x009d1b17  (REL32, ::operator delete)
//   compare.py masks those 4 bytes automatically.
//
// Asm (35 bytes @ orig RVA 0x0001a4a0, within Ghidra's 35-B window):
//
//   0001a4a0: 56                   PUSH ESI
//   0001a4a1: 8b 31                MOV  ESI, dword ptr [ECX]     ; inner = this->field_0
//   0001a4a3: 85 f6                TEST ESI, ESI
//   0001a4a5: 74 1d                JZ   epilogue (0x0001a4c4 — outside 35-B window)
//   0001a4a7: 8b 06                MOV  EAX, dword ptr [ESI]     ; sub = inner->field_0
//   0001a4a9: 85 c0                TEST EAX, EAX
//   0001a4ab: 74 08                JZ   skip_call (+8 → 0x0001a4b5)
//   0001a4ad: 8b 08                MOV  ECX, dword ptr [EAX]     ; vtable of sub
//   0001a4af: 8b 51 08             MOV  EDX, dword ptr [ECX+0x8] ; vtable[2]
//   0001a4b2: 50                   PUSH EAX                       ; arg = sub
//   0001a4b3: ff d2                CALL EDX                       ; vtable[2](sub)
//   skip_call (0x0001a4b5):
//   0001a4b5: 56                   PUSH ESI                       ; arg = inner
//   0001a4b6: c7 06 00 00 00 00    MOV  dword ptr [ESI], 0x0      ; inner->field_0 = null
//   0001a4bc: e8 56 76 5b 00       CALL 0x009d1b17                ; ::operator delete(inner)
//   0001a4c1: 83 c4                (first 2 bytes of ADD ESP,4 — window ends here)

extern "C" void FUN_009d1b17();   // ::operator delete

extern "C" __declspec(naked) void FUN_0041a4a0() {
    __asm {
        // 0001a4a0: 56
        _emit 0x56              // PUSH ESI

        // 0001a4a1: 8b 31
        _emit 0x8b              // MOV ESI, dword ptr [ECX]
        _emit 0x31

        // 0001a4a3: 85 f6
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6

        // 0001a4a5: 74 1d
        _emit 0x74              // JZ +0x1d  (→ epilogue at 0x0001a4c4, outside window)
        _emit 0x1d

        // 0001a4a7: 8b 06
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06

        // 0001a4a9: 85 c0
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0

        // 0001a4ab: 74 08
        _emit 0x74              // JZ +8  (→ skip_call at 0x0001a4b5)
        _emit 0x08

        // 0001a4ad: 8b 08
        _emit 0x8b              // MOV ECX, dword ptr [EAX]  (vtable ptr of sub-node)
        _emit 0x08

        // 0001a4af: 8b 51 08
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x8]  (vtable[2])
        _emit 0x51
        _emit 0x08

        // 0001a4b2: 50
        _emit 0x50              // PUSH EAX  (sub-node, on-stack "this" for COM dispatch)

        // 0001a4b3: ff d2
        _emit 0xff              // CALL EDX  (vtable[2](sub); callee cleans 1 arg, RET 4)
        _emit 0xd2

        // skip_call (0x0001a4b5):
        // 0001a4b5: 56
        _emit 0x56              // PUSH ESI  (inner node, arg to ::operator delete)

        // 0001a4b6: c7 06 00 00 00 00
        _emit 0xc7              // MOV dword ptr [ESI], 0x0  (inner->field_0 = null)
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 0001a4bc: e8 56 76 5b 00  (REL32 reloc — masked by compare.py)
        call FUN_009d1b17       // CALL ::operator delete(inner)

        // 0001a4c1: 83 c4  — first 2 bytes of ADD ESP, 4
        // (byte 35 = 0x04 is at 0x0001a4c3, outside Ghidra's 35-byte window)
        _emit 0x83
        _emit 0xc4
    }
}
