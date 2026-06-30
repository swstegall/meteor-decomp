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
// FUNCTION: ffxivgame 0x0004f740 — string-wrapper compare shim (39 B)
//
// int FUN_0044f740(StringWrapper* a, StringWrapper* b, int len, int flags)
//
// Extracts the raw char* from each wrapper via FUN_00445210 (a 3-byte
// __thiscall that returns *(char**)this) then forwards to the full string
// compare function FUN_0044f5e0.
//
// Argument evaluation is right-to-left: flags and len are preloaded into
// EAX/ECX before any pushes so their stack offsets don't shift, then
// b->FUN_00445210() is computed (ECX=b after 2 pushes), its result pushed,
// then a->FUN_00445210() (ECX=a pre-loaded before that push), and finally
// FUN_0044f5e0 is called.  ADD ESP,0x10 batch-cleans all 4 accumulated
// pushes; plain RET leaves the caller to clean FUN_0044f740's own 4 args.
//
// Calling convention: __cdecl — 4 args, int return.
// Frame: none (/Oy).

struct FUN_00445210_t {
    const char* FUN_00445210();
};

extern "C" int FUN_0044f5e0(const char*, const char*, int, int);

int __cdecl FUN_0044f740(FUN_00445210_t* param1, FUN_00445210_t* param2,
                         int param3, int param4)
{
    return FUN_0044f5e0(param1->FUN_00445210(), param2->FUN_00445210(),
                        param3, param4);
}
