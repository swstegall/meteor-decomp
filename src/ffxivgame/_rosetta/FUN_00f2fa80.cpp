// [STAMPED] from FUN_00414d10.cpp by tools/stamp_clusters.py
//           sibling at orig RVA 0x00b2fa80 (VA 0x00f2fa80)
//           same byte-shape cluster — see cluster_shapes.py output
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
// FUNCTION: ffxivgame 0x00f2fa80 — singleton-instance tail-call forwarder
// Reloc-aware cluster template — primary of a 1,838-member cluster.
// Each member loads a different global instance address into ECX
// and tail-jumps to a different `__thiscall` method, but the byte
// structure is identical mod the two relocations.
//
// Asm (10 bytes):
//   b9 ?? ?? ?? ??  MOV ECX, &g_instance    ; reloc — per-singleton address
//   e9 ?? ?? ?? ??  JMP method              ; reloc — tail call
//
// Standard tail-call optimisation: a free function that returns the
// result of `g_instance.method()` (with no other work). cl.exe under
// /O2 emits the load-this + JMP-method form instead of CALL+RET, so
// the resulting function is just 10 bytes (no prologue, no
// epilogue, no stack frame).

class CClass {
public:
    int method();
};

extern CClass g_instance;

int forwarder() {
    return g_instance.method();
}
