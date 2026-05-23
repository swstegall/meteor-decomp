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
// FUNCTION: ffxivgame 0x4105b0 — Return `this` — MOV EAX,ECX; RET.
// __thiscall member returning `this` with no stack arguments.
//
// Asm: 8b c1 c3

class C { public: C *identity(); };
C *C::identity() { return this; }
