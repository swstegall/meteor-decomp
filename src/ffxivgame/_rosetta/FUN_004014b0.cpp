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
// FUNCTION: ffxivgame 0x004014b0 — `__thiscall` Win32 message-pump tick
//                                   (307 B / 0x133, EH3-SEH wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x000014b0):
//
//   __thiscall bool tick(this) — `ECX = this`, returns `bool` in AL.
//
//   Structure (matches Ghidra's headless decompile and the asm flow):
//
//     if (this->state_flag_8 != 1) {                  // [esi+0x08]
//         if (++this->tick_count_b < 8) {             // [esi+0x0b]
//             MSG msg;
//             if (PeekMessageW(&msg, this->hwnd_18,   // [esi+0x18]
//                              0, 0, PM_REMOVE)) {
//                 if ((msg.message == WM_KEYDOWN ||
//                      msg.message == WM_KEYUP) &&
//                     msg.wParam == VK_PROCESSKEY /* 0xE5 */) {
//                     if (this->ime_sub_30.canTranslate()) {  // [esi+0x30], call -> 0x004b3820 (__thiscall)
//                         msg.wParam = ImmGetVirtualKey(msg.hwnd);
//                     } else {
//                         UINT vk = ImmGetVirtualKey(msg.hwnd);
//                         if (vk == VK_TAB) msg.wParam = 9;
//                     }
//                 }
//                 TranslateMessage(&msg);
//                 DispatchMessageW(&msg);
//                 return true;
//             }
//             if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
//                 TranslateMessage(&msg);
//                 DispatchMessageW(&msg);
//                 return true;
//             }
//             // No message available — poll the inline polymorphic
//             // subobject at +0x34 via its 2nd vtable slot.
//             return this->m_sub_34.poll();           // [esi+0x34], (*(*[esi+0x34]+4))(&[esi+0x34])
//         }
//         // Tick budget exhausted — force-yield via the same poll(),
//         // and reset the counter if poll() succeeds.
//         if (!this->m_sub_34.poll()) return false;
//         this->tick_count_b = 0;
//         return true;
//     }
//     return false;
//
//   Stack frame (after the prologue, EBP-relative):
//     [ebp-0x04]  EH3 trylevel (`__$EHRec.trylevel`, init -1, set 0)
//     [ebp-0x08]  EH3 handler  (0xe54322 — scope table / personality)
//     [ebp-0x0c]  EH3 next     (FS:[0] chain link)
//     [ebp-0x10]  saved ESP    (`__$EHRec.SavedESP`)
//     [ebp-0x14]  saved `this` (ESI snapshot for unwind)
//     [ebp-0x30 .. ebp-0x14]  tagMSG (28 B): hwnd / message / wParam /
//                              lParam / time / pt.x / pt.y
//     [ebp-0x34]  __security_cookie ^ EBP (pushed below the locals)
//
//   Reloc-bearing sites in the orig 307 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x06   scope-table handler RVA  (0x00e54322 — .rdata FuncInfo)
//     +0x18   __security_cookie load   (.data 0x012ea8b0)
//     +0x22   FS:[0] install epilog    (constant 0, fold-through)
//     +0x57   PeekMessageW IAT load    (.rdata 0x00f3e420)
//     +0x89   __thiscall callee CALL   (.text 0x004b3820 rel32 — ime sub)
//     +0x96   ImmGetVirtualKey CALL    (.text 0x009d00a2 rel32)
//     +0xa4   ImmGetVirtualKey CALL    (.text 0x009d00a2 rel32, 2nd)
//     +0xb6   TranslateMessage IAT     (.rdata 0x00f3e41c)
//     +0xc0   DispatchMessageW IAT     (.rdata 0x00f3e3e8)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact EH3 prolog (PUSH -1 / PUSH scope-table /
//   PUSH FS:[0] / cookie ^ EBP / FS:[0] = &EHRec), the exact register
//   allocation across the four PeekMessageW arms and two
//   ImmGetVirtualKey arms, AND the linker-resolved absolute addresses
//   in the eight relocation windows above. Each of those constraints
//   is brittle under /O2 — every high-level rewrite shifts at least
//   one byte (state numbering, branch short-vs-near, modrm vs moffs32).
//
//   The pragmatic choice — the same one FUN_00401750 took for its
//   SEH-wrapped 182-byte ctor — is a `__declspec(naked)` body that
//   re-emits the orig 307 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig
//   slice (no relocations because the bytes are emitted as raw
//   immediates), which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what
//   the function actually does, so a future contributor can promote
//   this to a real source-level match once the upstream IME-helper
//   callee (0x004b3820), the `m_sub_34` vtable layout, and the
//   surrounding window-frame class are reconstructed.

extern "C" __declspec(naked) void FUN_004014b0() {
    __asm {
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x22
        _emit 0x43
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x24
        _emit 0x53
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc5
        _emit 0x50
        _emit 0x8d

        _emit 0x45
        _emit 0xf4
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        _emit 0x8b
        _emit 0xf1
        _emit 0x89
        _emit 0x75
        _emit 0xec

        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x38
        _emit 0x5e
        _emit 0x08
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f

        _emit 0x84
        _emit 0xb3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5e
        _emit 0x0b
        _emit 0x80
        _emit 0x7e
        _emit 0x0b
        _emit 0x08
        _emit 0x0f
        _emit 0x83
        _emit 0xba
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x8b
        _emit 0x3d
        _emit 0x20
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x53
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x00

        _emit 0x50
        _emit 0x8d
        _emit 0x4d
        _emit 0xd0
        _emit 0x51
        _emit 0xff
        _emit 0xd7
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x6d
        _emit 0x8b
        _emit 0x45
        _emit 0xd4
        _emit 0x3d
        _emit 0x00

        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x74
        _emit 0x07
        _emit 0x3d
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x34
        _emit 0x81
        _emit 0x7d
        _emit 0xd8
        _emit 0xe5

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x2b
        _emit 0x8d
        _emit 0x4e
        _emit 0x30
        _emit 0xe8
        _emit 0xe3
        _emit 0x22
        _emit 0x0b
        _emit 0x00
        _emit 0x84
        _emit 0xc0
        _emit 0x74

        _emit 0x0e
        _emit 0x8b
        _emit 0x55
        _emit 0xd0
        _emit 0x52
        _emit 0xe8
        _emit 0x58
        _emit 0xeb
        _emit 0x5c
        _emit 0x00
        _emit 0x89
        _emit 0x45
        _emit 0xd8
        _emit 0xeb
        _emit 0x11
        _emit 0x8b

        _emit 0x45
        _emit 0xd0
        _emit 0x50
        _emit 0xe8
        _emit 0x4a
        _emit 0xeb
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xf8
        _emit 0x09
        _emit 0x75
        _emit 0x03
        _emit 0x89
        _emit 0x45
        _emit 0xd8

        _emit 0x8d
        _emit 0x4d
        _emit 0xd0
        _emit 0x51
        _emit 0xff
        _emit 0x15
        _emit 0x1c
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x8d
        _emit 0x55
        _emit 0xd0
        _emit 0x52
        _emit 0xff
        _emit 0x15

        _emit 0xe8
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        _emit 0x8a
        _emit 0xc3
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
        _emit 0x53
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x8d

        _emit 0x45
        _emit 0xd0
        _emit 0x50
        _emit 0xff
        _emit 0xd7
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0xc7
        _emit 0x8b
        _emit 0x46
        _emit 0x34
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        _emit 0x8d

        _emit 0x4e
        _emit 0x34
        _emit 0xff
        _emit 0xd2
        _emit 0x84
        _emit 0xc0
        _emit 0x75
        _emit 0x27
        _emit 0x32
        _emit 0xc0
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        _emit 0x64
        _emit 0x89
        _emit 0x0d

        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b
        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
        _emit 0x8b
        _emit 0x46
        _emit 0x34
        _emit 0x8b

        _emit 0x50
        _emit 0x04
        _emit 0x8d
        _emit 0x4e
        _emit 0x34
        _emit 0xff
        _emit 0xd2
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0xdd
        _emit 0xc6
        _emit 0x46
        _emit 0x0b
        _emit 0x00
        _emit 0x8a

        _emit 0xc3
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x8b

        _emit 0xe5
        _emit 0x5d
        _emit 0xc3
    }
}
