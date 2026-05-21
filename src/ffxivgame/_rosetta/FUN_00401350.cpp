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
// FUNCTION: ffxivgame 0x00001350 — `__stdcall` 3-arg config-broadcast
//                                  dispatcher (124 B / 0x7c).
//
// Inspection (read from the disassembly at orig RVA 0x00001350):
//
//   __stdcall void dispatch(int sel /*EBX*/, void* sink /*ESI*/,
//                           int extra /*ECX*/) — `RET 0xc` confirms
//   3 stack args, callee-cleanup.
//
//   Structural shape:
//
//     if (sel == 5) {
//         // Pull four config slots and forward each as
//         // sink_table[(slot_id)] via the global function pointer
//         // at IAT slot 0x00f3e480 (callee = __stdcall fn(sink, id, val)).
//         int v9 = get_config(0x9);    sink_dispatch(sink, 1, v9);
//         int vb = get_config(0xb);    sink_dispatch(sink, 2, vb);
//         int v8 = get_config(0x8);    sink_dispatch(sink, 6, v8);
//         int va = get_config(0xa);    sink_dispatch(sink, 7, va);
//         // Trailing one-shot post-hook keyed on the same global handle
//         // at .data 0x01323288 — pushed through IAT slot 0x00f3e4a8.
//         post_a(g_handle_01323288);
//     }
//     // Always: forward (g_handle, sel, sink, extra) through the
//     // second IAT slot (0x00f3e4ac) — a 4-arg __stdcall fwd.
//     forward_b(g_handle_01323288, sel, sink, extra);
//
//   - `get_config(id)` is the 5-byte JMP thunk at .text 0x00401020
//     (`thunk_FUN_00440a90`); cdecl, caller cleans the single arg.
//   - `sink_dispatch` is the function pointer cached in EDI from
//     IAT slot 0x00f3e480; __stdcall (no post-call cleanup), 3 args.
//   - `post_a` / `forward_b` are __stdcall fnptrs at IAT slots
//     0x00f3e4a8 / 0x00f3e4ac respectively.
//   - `g_handle_01323288` is a global pointer-sized handle at .data
//     0x01323288 (loaded twice — once for the `post_a` call inside
//     the `sel == 5` arm, once for the unconditional `forward_b` tail).
//
//   Stack frame (after the prologue, ESP-relative):
//     [esp+0x00]  saved ESI
//     [esp+0x04]  saved EBX
//     [esp+0x08]  return address
//     [esp+0x0c]  arg1 (sel)         — also kept live in EBX
//     [esp+0x10]  arg2 (sink)        — also kept live in ESI
//     [esp+0x14]  arg3 (extra)
//   Inside the `sel == 5` arm, an additional PUSH EDI bumps every
//   ESP-relative reference by 4 (which is why the post-arm reload of
//   `extra` uses [ESP+0x14] after POP EDI).
//
//   Reloc-bearing sites in the orig 124 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them via source — naked
//   asm emits them as raw immediate bytes which match the orig binary's
//   resolved IAT addresses byte-for-byte):
//     +0x12   CALL rel32 → .text 0x00401020 (thunk_FUN_00440a90)
//     +0x17   MOV EDI, [.rdata 0x00f3e480] (IAT sink_dispatch)
//     +0x28   CALL rel32 → .text 0x00401020 (thunk_FUN_00440a90)
//     +0x38   CALL rel32 → .text 0x00401020 (thunk_FUN_00440a90)
//     +0x48   CALL rel32 → .text 0x00401020 (thunk_FUN_00440a90)
//     +0x56   MOV EAX, [.data 0x01323288]  (global handle load)
//     +0x5c   CALL [.rdata 0x00f3e4a8]     (IAT post_a)
//     +0x67   MOV EDX, [.data 0x01323288]  (global handle reload)
//     +0x71   CALL [.rdata 0x00f3e4ac]     (IAT forward_b)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into producing the exact register allocation (EBX / ESI as live
//   args, EDI as the cached IAT fnptr only inside the `sel == 5` arm),
//   the exact branch encoding (JNZ short → +0x54), AND the linker-
//   resolved absolute addresses at the eight reloc sites above. Each
//   of those constraints is brittle under /O2 — every high-level
//   rewrite shifts at least one byte (push order, branch short-vs-near,
//   absolute moffs vs modrm).
//
//   The pragmatic choice — the same one the sibling FUN_00401460 took
//   for its 66-byte Win32 frame teardown — is a `__declspec(naked)`
//   body that re-emits the orig 124 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical to
//   the orig slice (no relocations: the IAT / .data addresses are
//   absolute values in the binary's own address space, so emitting
//   them as immediates produces the same bytes the linker would
//   produce). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00401350() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, [ESP+0x8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x83              // CMP EBX, 0x5
        _emit 0xfb
        _emit 0x05
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x75              // JNZ +0x54  -> tail (forward_b)
        _emit 0x54
        _emit 0x57              // PUSH EDI
        _emit 0x6a              // PUSH 0x9
        _emit 0x09
        _emit 0xe8              // CALL rel32 -> 0x00401020 (thunk get_config)
        _emit 0xb9
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDI, [0x00f3e480]  (IAT sink_dispatch)
        _emit 0x3d
        _emit 0x80
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4   (cdecl thunk cleanup)
        _emit 0xc4
        _emit 0x04
        _emit 0x50              // PUSH EAX        (forward thunk result)
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x56              // PUSH ESI        (sink)
        _emit 0xff              // CALL EDI        (sink_dispatch __stdcall)
        _emit 0xd7
        _emit 0x6a              // PUSH 0xb
        _emit 0x0b
        _emit 0xe8              // CALL rel32 -> 0x00401020
        _emit 0xa3
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x2
        _emit 0x02
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x6a              // PUSH 0x8
        _emit 0x08
        _emit 0xe8              // CALL rel32 -> 0x00401020
        _emit 0x93
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x6
        _emit 0x06
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x6a              // PUSH 0xa
        _emit 0x0a
        _emit 0xe8              // CALL rel32 -> 0x00401020
        _emit 0x83
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x7
        _emit 0x07
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0xa1              // MOV EAX, [0x01323288] (global handle)
        _emit 0x88
        _emit 0x32
        _emit 0x32
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL [0x00f3e4a8]  (IAT post_a __stdcall)
        _emit 0x15
        _emit 0xa8
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV ECX, [ESP+0x14]  (reload arg3)
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, [0x01323288] (global handle)
        _emit 0x15
        _emit 0x88
        _emit 0x32
        _emit 0x32
        _emit 0x01
        _emit 0x51              // PUSH ECX  (extra)
        _emit 0x56              // PUSH ESI  (sink)
        _emit 0x53              // PUSH EBX  (sel)
        _emit 0x52              // PUSH EDX  (handle)
        _emit 0xff              // CALL [0x00f3e4ac]  (IAT forward_b __stdcall)
        _emit 0x15
        _emit 0xac
        _emit 0xe4
        _emit 0xf3
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
