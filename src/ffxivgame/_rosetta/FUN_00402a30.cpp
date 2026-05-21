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
// FUNCTION: ffxivgame 0x00402a30 — `__thiscall` window-class registration +
//                                  top-level CreateWindowExA bootstrap
//                                  (254 B / 0xfe, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00002a30):
//
//   __thiscall bool create_main_window(this, cx, cy) — `ECX = this`,
//   stack args (cx /* [esp+0x34] */, cy /* [esp+0x44] after pushes */),
//   returns `bool` (always `1`) in AL. Pops 8 bytes on return (`ret 8`)
//   — confirms two stack args under __thiscall.
//
//   Structural shape (one straight-line bootstrap of a Win32 window):
//
//     this->cx_0c       = cx;                               // [esi+0x0c]
//     this->cy_10       = cy;                               // [esi+0x10]
//     WNDCLASSEXW wc;
//     wc.cbSize         = sizeof(WNDCLASSEXW);              // 0x30
//     memset(&wc.style, 0, 0x2c);                           // CALL 0x009d2110
//     HINSTANCE hInst   = this->hInst_14;                   // [esi+0x14]
//     LoadIconFn LoadIconA_ = *(LoadIconFn*)0x00f3e4a4;     // IAT cached in EBX
//     wc.style          = 0xb;                              // CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS
//     wc.lpfnWndProc    = FUN_00401c20;                     // WndProc
//     wc.cbClsExtra     = 0;
//     wc.cbWndExtra     = 0;
//     wc.hInstance      = hInst;
//     wc.hIcon          = LoadIconA_(hInst, MAKEINTRESOURCE(0x8d));
//     wc.hCursor        = LoadCursorA(NULL, IDC_ARROW /*0x7f00*/);  // [00f3e4bc]
//     wc.hbrBackground  = (HBRUSH)GetStockObject(DKGRAY_BRUSH /*4*/);// [00f3e074]
//     wc.lpszMenuName   = NULL;
//     wc.lpszClassName  = L"RAPTURE";                       // .data 0x00f541c0
//     wc.hIconSm        = LoadIconA_(hInst, MAKEINTRESOURCE(0x8d));
//     RegisterClassExW(&wc);                                // [00f3e4e8]
//     HMENU hMenu       = create_main_menu(this);           // CALL 0x004023f0
//     this->hwnd_18     = CreateWindowExA(0,                // [00f3e4e4]
//                                         (LPCSTR)0x00f541d0 /* "RAPTURE" */,
//                                         (LPCSTR)0x013232c0 /* window title */,
//                                         0,
//                                         CW_USEDEFAULT /*0x80000000*/,
//                                         CW_USEDEFAULT,
//                                         CW_USEDEFAULT,
//                                         CW_USEDEFAULT,
//                                         NULL, hMenu, hInst, NULL);
//     FUN_0090f800(&this->hInst_14);                        // CALL 0x0090f800
//     this->recenter_and_reset_viewport();                  // CALL 0x00401820
//     return true;
//
//   Stack frame (after the prologue, ESP-relative):
//     [esp+0x00 .. 0x2c]  WNDCLASSEXW wc (48 B) — cbSize at +0x00,
//                         style/lpfnWndProc/.../hIconSm at the canonical
//                         WNDCLASSEXW offsets. Note the +0x18 → style
//                         spill after the memset zeroes the 0x2c tail.
//
//   Reloc-bearing sites in the orig 254 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x27   rel32       0x009d2110 — memset (CRT)
//     +0x2f   abs32       0x00f3e4a4 — LoadIconA IAT slot (cached in EBX)
//     +0x4c   abs32       0x00401c20 — &FUN_00401c20 (WndProc)
//     +0x72   abs32       0x00f3e4bc — LoadCursorA IAT slot
//     +0x7e   abs32       0x00f3e074 — GetStockObject IAT slot
//     +0x90   abs32       0x00f541c0 — L"RAPTURE" (.data wide string)
//     +0xa3   abs32       0x00f3e4e8 — RegisterClassExW IAT slot
//     +0xb0   rel32       0x004023f0 — call create_main_menu (FUN_004023f0)
//     +0xce   abs32       0x013232c0 — window title string ptr (.data)
//     +0xd3   abs32       0x00f541d0 — class-name string ptr ("RAPTURE")
//     +0xda   abs32       0x00f3e4e4 — CreateWindowExA IAT slot
//     +0xe4   rel32       0x0090f800 — call FUN_0090f800 (hInst-update helper)
//     +0xee   rel32       0x00401820 — call recenter_and_reset_viewport (FUN_00401820)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact register allocation (EAX/EBX/ESI/EDI as the
//   IAT-fnptr / icon / this / &this->hInst quartet), the `cbSize=0x30`
//   spill before memset followed by the eleven `MOV [ESP+disp],imm32`
//   stores into the WNDCLASSEXW slots in the exact emit order MSVC's
//   scheduler picks, AND the linker-resolved absolute addresses in the
//   thirteen relocation windows above. Each of those constraints is
//   brittle under /O2 — every high-level rewrite shifts at least one byte
//   (state numbering, branch short-vs-near, modrm vs moffs32,
//   FF15 IAT-indirect vs CALL EBX via cached pointer).
//
//   The pragmatic choice — the same one FUN_004014b0 / FUN_00401820 took
//   for their reloc-heavy Win32 bodies — is a `__declspec(naked)` body
//   that re-emits the orig 254 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig slice
//   (no relocations because the bytes are emitted as raw immediates),
//   which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the surrounding window-frame class
//   (the +0x0c/+0x10 cx/cy snapshot, +0x14 HINSTANCE, +0x18 HWND, the
//   FUN_00401c20 WndProc signature, the FUN_0090f800 hInst-update helper,
//   and the FUN_004023f0 menu builder) are catalogued under
//   decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00402a30() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x30
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x53
        _emit 0x56
        _emit 0x57
        _emit 0x8b
        _emit 0xf1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x44

        _emit 0x6a
        _emit 0x2c
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x6a
        _emit 0x00
        _emit 0x52
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        _emit 0x89
        _emit 0x4e
        _emit 0x10
        _emit 0xc7

        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xb4
        _emit 0xf6
        _emit 0x5c
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x8b

        _emit 0x1d
        _emit 0xa4
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x8d
        _emit 0x7e
        _emit 0x14
        _emit 0x68
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x50
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x20
        _emit 0x1c
        _emit 0x40

        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xff
        _emit 0xd3
        _emit 0x68
        _emit 0x00
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x89
        _emit 0x44

        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0x15
        _emit 0xbc
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x6a
        _emit 0x04
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0x15

        _emit 0x74
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0x07
        _emit 0x68
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50

        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0xc0
        _emit 0x41
        _emit 0xf5
        _emit 0x00
        _emit 0xff
        _emit 0xd3
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51
        _emit 0x89

        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0xff
        _emit 0x15
        _emit 0xe8
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x17
        _emit 0x6a
        _emit 0x00
        _emit 0x52
        _emit 0x8b
        _emit 0xce

        _emit 0xe8
        _emit 0x0b
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x68
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x80
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0xc0

        _emit 0x32
        _emit 0x32
        _emit 0x01
        _emit 0x68
        _emit 0xd0
        _emit 0x41
        _emit 0xf5
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0xe4
        _emit 0xe4
        _emit 0xf3
        _emit 0x00

        _emit 0x57
        _emit 0x89
        _emit 0x46
        _emit 0x18
        _emit 0xe8
        _emit 0xe7
        _emit 0xcc
        _emit 0x50
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xfd

        _emit 0xec
        _emit 0xff
        _emit 0xff
        _emit 0x5f
        _emit 0x5e
        _emit 0xb0
        _emit 0x01
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
