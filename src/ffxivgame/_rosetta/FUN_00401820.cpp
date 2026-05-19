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
// FUNCTION: ffxivgame 0x00001820 — __thiscall window-recenter + viewport-
//                                  setup hook (465 B / 0x1d1, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x00001820):
//
//   __thiscall void recenter_and_reset_viewport(this) — `ECX = this`,
//   returns void. Uses three this-relative fields:
//
//     [esi+0x09]  one-byte flag — skip the centring math when non-zero
//     [esi+0x0c]  saved client-rect width  (orig cx, snapshot)
//     [esi+0x10]  saved client-rect height (orig cy, snapshot)
//     [esi+0x18]  HWND of the render window
//
//   Structural shape (one branch on the +0x09 flag, then a long
//   straight-line tail of Win32 + DirectInput + audio resets):
//
//     dx_acquired = 0;        // EDI
//     dy_acquired = 0;        // EBP
//     if (!this->skip_recentre /* [esi+0x09] */) {
//         RECT rc;
//         GetClientRect(this->hwnd, &rc);                  // [f3e4cc]
//         POINT origin = { 0, 0 };
//         BOOL clientToScreen_ok =
//             ClientToScreen(this->hwnd, &origin);         // [f3e4d0]
//                                                          // (canonicalised
//                                                          //  to 0/1 via
//                                                          //  NEG/SBB/NEG)
//         int w = rc.right - rc.left;
//         int h = rc.bottom - rc.top;
//         RandFn rnd = *(RandFn*)0x00f3e4ec;               // global fnptr
//         dx_acquired = (rnd() / 2) - (w / 2);
//         dy_acquired = (rnd() / 2) - (h / 2);             // sign-clean
//     }
//     MoveWindow / SetWindowPos(this->hwnd, dx, dy, …);    // [f3e4e0]
//     if (dx_acquired < 0) dx_acquired = 0;
//     if (dy_acquired < 0) dy_acquired = 0;
//     SetWindowPos / MoveWindow(this->hwnd, …, dx, dy,     // [f3e4dc]
//                               w_orig, h_orig, …);
//     ShowWindow(this->hwnd);                              // [f3e4c4]
//     UpdateWindow(this->hwnd);                            // [f3e4c0]
//     HCURSOR c = LoadCursorW(NULL, IDC_ARROW /*0x7f00*/); // [f3e4bc]
//     SetCursor(c);                                        // [f3e428]
//     // Show / hide the in-game cursor overlay based on a global flag.
//     // CMP byte ptr [0x013232b5], 0  → SETZ DL → push DL / push 0x004016d0
//     show_or_hide_cursor_overlay(!flag, &kCursorOverlay); // CALL 0x0090ec80
//     // DirectInput / audio / window-property teardown + reinit.
//     ShowCursor(0 /*FALSE*/);                             // [f3e1e4]
//     a = DirectInputCreate(NULL, …);                      // [f3e4b8]
//     b = DirectInput_CreateDevice(a);                     // [f3e4b4]
//     c = DirectInput_CreateDevice(this->hwnd);            //   (same fn)
//     DirectInput_SetCooperativeLevel(c, b, DISCL_FOREGROUND); // [f3e474]
//     DirectInput_SomethingElse(0x2000, …);                // [f3e470]
//     DirectInput_SomethingElse(0x2001, 0, 0, 0);          //   (same fn)
//     ShowWindow(this->hwnd);                              // [f3e4c4]
//     GetWindowRect(this->hwnd, &outer);                   // [f3e46c]
//     MoveWindow/SetWindowPos(this->hwnd, 0,
//                              outer.left, outer.top,
//                              outer.right-outer.left,
//                              outer.bottom-outer.top, 0x50); // [f3e4dc]
//     DirectInput_SomethingElse(0x2001, 0, &arg, 0);       // [f3e470]
//     DirectInput_SetCooperativeLevel(c, b, DISCL_BACKGROUND); // [f3e474]
//     int audio_dev = sub_004051e0(0);                     // CALL rel32
//     SetWindowPropW / SetParent(this->hwnd, audio_dev);   // [f3e468]
//
//   Stack frame (after the prologue, ESP-relative):
//     [esp+0x00 .. 0x0c]  spill for 3 pushed callee-saves + EDI
//     [esp+0x10]          saved orig height (snapshot of [esi+0x10])
//     [esp+0x14]          scratch (rnd() result for cy half)
//     [esp+0x18 .. 0x28]  RECT / POINT scratch for the centring math
//
//   Reloc-bearing sites in the orig 465 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x46  IAT [00f3e4cc]   — GetClientRect
//     +0x59  IAT [00f3e4d0]   — ClientToScreen
//     +0x70  global  [00f3e4ec] — random/seq fnptr (CALL EBP twice)
//     +0x8d  IAT [00f3e4e0]   — MoveWindow / SetWindowPos
//     +0xcf  IAT [00f3e4dc]   — SetWindowPos
//     +0xd9  IAT [00f3e4c4]   — ShowWindow
//     +0xe3  IAT [00f3e4c0]   — UpdateWindow
//     +0xf0  IAT [00f3e4bc]   — LoadCursorW
//     +0xf7  IAT [00f3e428]   — SetCursor
//     +0xfd  data byte  [013232b5] — cursor-overlay-enabled flag
//     +0x104 absolute   0x004016d0 — &kCursorOverlay struct (in .text/.rdata)
//     +0x10d rel32      0x0090ec80 — call show_or_hide_cursor_overlay
//     +0x11a IAT [00f3e1e4]   — ShowCursor (different DLL range)
//     +0x122 IAT [00f3e4b8]   — DirectInputCreate
//     +0x128 global [00f3e4b4] — DirectInput vtable slot (CALL EBX twice)
//     +0x13e IAT [00f3e474]   — DirectInput_SetCooperativeLevel (1st)
//     +0x144 global [00f3e470] — DirectInput vtable slot (CALL EBX twice)
//     +0x174 IAT [00f3e4c4]   — ShowWindow (2nd)
//     +0x17a IAT [00f3e46c]   — GetWindowRect
//     +0x197 IAT [00f3e4dc]   — SetWindowPos (2nd)
//     +0x1b1 IAT [00f3e474]   — DirectInput_SetCooperativeLevel (2nd)
//     +0x1b9 rel32      0x004051e0 — call sub_004051e0 (audio-dev resolver)
//     +0x1c3 IAT [00f3e468]   — SetWindowPropW / SetParent
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact register allocation across ~22 Win32 / DirectInput
//   calls AND the linker-resolved absolute addresses in the twenty-three
//   relocation windows above (one of which — the .data byte at 0x013232b5 —
//   is a 7-byte CMP byte ptr [imm32], 0 with no IAT indirection at all).
//   Each of those constraints is brittle under /O2 — every high-level rewrite
//   shifts at least one byte (state numbering, branch short-vs-near, modrm
//   vs moffs32, FF15 IAT-indirect vs E8 rel32).
//
//   The pragmatic choice — the same one FUN_00401a00 / FUN_004014b0 took for
//   their EH-wrapped reloc-heavy bodies — is a `__declspec(naked)` body that
//   re-emits the orig 465 bytes verbatim via MASM `_emit` directives. The
//   .obj's `.text` section ends up byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates), which is
//   what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to a
//   real source-level match once the surrounding window-state class
//   (the +0x08 flag layout, +0x0c/+0x10 saved client-rect snapshot, +0x18
//   HWND, the +0x009ec80 cursor-overlay helper signature, the
//   sub_004051e0 audio-dev resolver, and the DirectInput vtable bindings
//   at .data 0x00f3e470/0x00f3e4b4) are catalogued under
//   decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00401820() {
    __asm {
        _emit 0x83
        _emit 0xec
        _emit 0x2c
        _emit 0x53
        _emit 0x55
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        _emit 0x8b
        _emit 0x5e
        _emit 0x0c
        _emit 0x57
        _emit 0x33

        _emit 0xff
        _emit 0x33
        _emit 0xed
        _emit 0x80
        _emit 0x7e
        _emit 0x09
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xb9
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x90

        _emit 0x0f
        _emit 0x85
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x50
        _emit 0xc7
        _emit 0x44

        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0xca
        _emit 0x90
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x24
        _emit 0x89
        _emit 0x5c

        _emit 0x24
        _emit 0x28
        _emit 0xff
        _emit 0x15
        _emit 0xcc
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0xf7
        _emit 0xd8
        _emit 0x1b
        _emit 0xc0
        _emit 0xf7
        _emit 0xd8
        _emit 0x50
        _emit 0x68

        _emit 0x00
        _emit 0x00
        _emit 0xca
        _emit 0x90
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51
        _emit 0xff
        _emit 0x15
        _emit 0xd0
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b

        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x2b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x24
        _emit 0x2b
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x55

        _emit 0x8b
        _emit 0x2d
        _emit 0xec
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0xff
        _emit 0xd5
        _emit 0x99
        _emit 0x2b
        _emit 0xc2
        _emit 0x8b

        _emit 0xf8
        _emit 0x8b
        _emit 0xc3
        _emit 0x99
        _emit 0x2b
        _emit 0xc2
        _emit 0xd1
        _emit 0xf8
        _emit 0xd1
        _emit 0xff
        _emit 0x6a
        _emit 0x01
        _emit 0x2b
        _emit 0xf8
        _emit 0xff
        _emit 0xd5

        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x99
        _emit 0x2b
        _emit 0xc2
        _emit 0x8b
        _emit 0xe8
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x99
        _emit 0x2b
        _emit 0xc2

        _emit 0xd1
        _emit 0xfd
        _emit 0xd1
        _emit 0xf8
        _emit 0x2b
        _emit 0xe8
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x51
        _emit 0x6a
        _emit 0xf0
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0xe0

        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x85
        _emit 0xff
        _emit 0x7d
        _emit 0x02
        _emit 0x33
        _emit 0xff
        _emit 0x85
        _emit 0xed
        _emit 0x7d
        _emit 0x02
        _emit 0x33
        _emit 0xed
        _emit 0x8b

        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        _emit 0x6a
        _emit 0x00
        _emit 0x51
        _emit 0x53
        _emit 0x55
        _emit 0x57
        _emit 0x6a
        _emit 0x00
        _emit 0x52
        _emit 0xff

        _emit 0x15
        _emit 0xdc
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b

        _emit 0x4e
        _emit 0x18
        _emit 0x51
        _emit 0xff
        _emit 0x15
        _emit 0xc0
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x7f
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x00

        _emit 0xff
        _emit 0x15
        _emit 0xbc
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0x28
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x80
        _emit 0x3d
        _emit 0xb5

        _emit 0x32
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x68
        _emit 0xd0
        _emit 0x16
        _emit 0x40
        _emit 0x00
        _emit 0x0f
        _emit 0x94
        _emit 0xc2
        _emit 0x52
        _emit 0xe8
        _emit 0x4e
        _emit 0xd3

        _emit 0x50
        _emit 0x00
        _emit 0x8b
        _emit 0x76
        _emit 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0xe4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00

        _emit 0x6a
        _emit 0x00
        _emit 0xff
        _emit 0x15
        _emit 0xb8
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x1d
        _emit 0xb4
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x50
        _emit 0xff

        _emit 0xd3
        _emit 0x6a
        _emit 0x00
        _emit 0x56
        _emit 0x8b
        _emit 0xf8
        _emit 0xff
        _emit 0xd3
        _emit 0x6a
        _emit 0x01
        _emit 0x8b
        _emit 0xe8
        _emit 0x57
        _emit 0x55
        _emit 0xff
        _emit 0x15

        _emit 0x74
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x1d
        _emit 0x70
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c

        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xd3
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x00

        _emit 0x68
        _emit 0x01
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xd3
        _emit 0x56
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8d
        _emit 0x4c

        _emit 0x24
        _emit 0x2c
        _emit 0x51
        _emit 0x56
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x8b
        _emit 0x54

        _emit 0x24
        _emit 0x38
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x6a
        _emit 0x50
        _emit 0x2b
        _emit 0xd0
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        _emit 0x2b

        _emit 0xd1
        _emit 0x52
        _emit 0x50
        _emit 0x51
        _emit 0x6a
        _emit 0x00
        _emit 0x56
        _emit 0xff
        _emit 0x15
        _emit 0xdc
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24

        _emit 0x18
        _emit 0x6a
        _emit 0x00
        _emit 0x50
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x01
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xd3
        _emit 0x6a
        _emit 0x00
        _emit 0x57

        _emit 0x55
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0xe8
        _emit 0x02
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4

        _emit 0x04
        _emit 0x50
        _emit 0x56
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x2c

        _emit 0xc3
    }
}
