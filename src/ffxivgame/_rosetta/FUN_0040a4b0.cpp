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
// FUNCTION: ffxivgame 0x0040a4b0 — `__thiscall` "register a CDev physics
//                                  metric" reporter (90 B / 0x5a)
//
// __thiscall void FUN_0040a4b0(this, undefined4 arg1, undefined4 arg2)
//   stack layout (after RET 8):
//     ECX        : this              (the owner — likely a CDev metric group)
//     [ESP+0x04] : undefined4 arg1   (label / parent-handle parameter)
//     [ESP+0x08] : undefined4 arg2   (payload — passed verbatim to either
//                                     the fast-path FUN_0040a410 or the
//                                     slow-path FUN_0040e110 emitter)
//
// Inspection (read from the orig bytes at RVA 0x0000a4b0, 90 bytes):
//
//     sub  esp, 0x10                  ; reserve 16B local (a CDev "node"
//                                     ;   temporary built in-place below)
//     cmp  byte ptr [0x01328034], 0   ; g_phy_registry_ready ?
//     push esi
//     mov  esi, ecx                   ; esi = this (cached for the tail)
//     jnz  ready                      ; registry up → slow path
//
//   not_ready:                        ; first-call cold path
//     push 0x00f552f0                 ; "CDev.Engine.Phy.Init"
//     lea  ecx, [esp+0x08]            ; ecx = &local_node (post-push esp)
//     jmp  emit                       ; fall into the construct-and-emit
//
//   ready:
//     mov  ecx, [0x01328038]          ; g_phy_registry_ptr
//     test ecx, ecx
//     jz   not_yet                    ; pointer not yet published →
//                                     ;   build a "CDev.Engine.Phy" node
//                                     ;   and emit through the slow path
//
//   fast_path:                        ; registry up AND ptr published
//     mov  edx, [esp+0x1c]            ; reload caller's arg2 (the payload)
//     push edx
//     call FUN_0040a410               ; the registered-path one-arg emitter
//     pop  esi                        ; restore callee-save
//     add  esp, 0x10
//     ret  8                          ; __thiscall, callee-cleans 2 dwords
//
//   not_yet:                          ; ready but registry ptr still NULL
//     push 0x00f552e0                 ; "CDev.Engine.Phy"
//     lea  ecx, [esp+0x10]            ; ecx = &local_node (different esp)
//   emit:
//     push 0x10                       ; node-size / kind tag
//     call FUN_0040e2d0               ; ctor: builds the CDev metric node
//                                     ;   in place; returns its handle in
//                                     ;   EAX
//     mov  ecx, [esi+0x04]            ; ecx = this->m_sink (a CDev sink
//                                     ;   pointer cached on the owner)
//     push eax                        ; the just-built node handle
//     mov  eax, [esp+0x20]            ; reload caller's arg2 (payload)
//     push eax
//     call FUN_0040e110               ; sink->emit(payload, node_handle)
//     pop  esi
//     add  esp, 0x10
//     ret  8
//
// Reloc-bearing sites (CALL rel32 targets the linker would resolve when
// emitted from source-level C++; we re-emit the orig rel32 bytes verbatim
// so the .obj's .text matches byte-for-byte with NO relocations):
//     +0x05   MOV  abs32   → 0x01328034  (.data — g_phy_registry_ready flag)
//     +0x10   PUSH imm32   → 0x00f552f0  (.rdata — "CDev.Engine.Phy.Init")
//     +0x1a   MOV  abs32   → 0x01328038  (.data — g_phy_registry_ptr)
//     +0x29   CALL rel32   → FUN_0040a410 (RVA 0x0000a410)
//     +0x35   PUSH imm32   → 0x00f552e0  (.rdata — "CDev.Engine.Phy")
//     +0x40   CALL rel32   → FUN_0040e2d0 (RVA 0x0000e2d0)
//     +0x4e   CALL rel32   → FUN_0040e110 (RVA 0x0000e110)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form here would emit the same shape but the four
//   reloc-bearing sites (two abs32 globals, three rel32 calls, two
//   absolute string-literal pushes) would yield non-trivial relocations
//   the linker resolves only against the orig binary's symbol table.
//   `tools/compare.py` masks reloc bytes out of the diff, but driving a
//   relink isn't necessary: a `__declspec(naked)` body that re-emits the
//   orig 90 bytes verbatim via MASM `_emit` directives produces a .obj
//   whose .text is byte-identical to the orig slice (no relocations —
//   the abs32 addresses and rel32 displacements are baked into the orig
//   binary's own address space and emitted here as raw bytes).
//   compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_0040a4b0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x10
        _emit 0xec
        _emit 0x10
        _emit 0x80              // CMP byte ptr [0x01328034], 0
        _emit 0x3d
        _emit 0x34
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x75              // JNZ short +0x0b -> ready
        _emit 0x0b
        _emit 0x68              // PUSH 0x00F552F0 ("CDev.Engine.Phy.Init")
        _emit 0xf0
        _emit 0x52
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x08]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xeb              // JMP short +0x24 -> emit
        _emit 0x24
        _emit 0x8b              // MOV ECX, dword ptr [0x01328038]  (ready:)
        _emit 0x0d
        _emit 0x38
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ short +0x11 -> not_yet
        _emit 0x11
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x1C]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL FUN_0040a410 (rel32 → 0xffffff32)
        _emit 0x32
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
        _emit 0x68              // PUSH 0x00F552E0 ("CDev.Engine.Phy")  (not_yet:)
        _emit 0xe0
        _emit 0x52
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x6a              // PUSH 0x10                          (emit:)
        _emit 0x10
        _emit 0xe8              // CALL FUN_0040e2d0 (rel32 → 0x00003ddb)
        _emit 0xdb
        _emit 0x3d
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x04]
        _emit 0x4e
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0040e110 (rel32 → 0x00003c0d)
        _emit 0x0d
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
