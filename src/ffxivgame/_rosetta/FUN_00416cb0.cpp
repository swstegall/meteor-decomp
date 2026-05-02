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
// FUNCTION: ffxivgame 0x00016cb0 — "is_nonzero" bool getter (canonical
// XOR/CMP/SETNZ idiom). Second easy-wins queue match candidate.
//
// Behavior (read from the asm @ 0x00016cb0, 9 bytes):
//   33 c0            XOR EAX, EAX
//   39 41 04         CMP dword ptr [ECX + 4], EAX
//   0f 95 c0         SETNZ AL
//   c3               RET
//
// Standard `__thiscall` `bool field != 0` getter. The XOR EAX, EAX
// serves double duty: clears the high bytes of EAX (so SETNZ AL
// produces a clean 0/1 bool in EAX), AND provides the zero comparand
// for the CMP. Calling convention: __thiscall, no stack args
// (RET, not RET N).

class IsNonZeroGetter {
public:
    bool is_nonzero() const;
private:
    int padding;       // [this+0]
    int value;         // [this+4] — the 4-byte int compared against 0
};

bool IsNonZeroGetter::is_nonzero() const {
    return value != 0;
}
