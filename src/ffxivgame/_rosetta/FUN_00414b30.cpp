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
// FUNCTION: ffxivgame 0x00414b30 — two-field __thiscall setter
// Loads both stack args into EAX/EDX, stores to [this+0x2c] and [this+0x30].
// No frame. Callee cleans 8 bytes (RET 8).

struct C {
    char _pad[0x2c];
    int field_0x2c;
    int field_0x30;
    void set_fields(int param_1, int param_2);
};

void C::set_fields(int param_1, int param_2) {
    field_0x2c = param_1;
    field_0x30 = param_2;
}
