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
// MSVC 2005 CRT helpers — `exit`, `_exit`, `atexit`. All statically
// linked into ffxivgame.exe (and the other 4 binaries) via `/MT`.
// Recovered 2026-05-02 from byte signatures vs canonical CRT source
// in <crt>/src/crt0msg.c, exit.c, atonexit.c.

extern "C" void __cdecl doexit(int code, int quick, int retcaller);
typedef void (__cdecl *_PVFV)(void);
extern "C" int __cdecl _onexit(_PVFV func);

// Size priority — orig CRT is built with `/Os` so MSVC picks the
// shorter `PUSH [mem]` over `MOV reg, [mem]; PUSH reg` (saves 1 B
// per arg). Same trick used for `_invalid_parameter_noinfo`.
#pragma optimize("s", on)

// FUNCTION: ffxivgame 0x005d910e — exit (17 B)
//   6a 00           PUSH 0          ; quick=0
//   6a 00           PUSH 0          ; retcaller=0
//   ff 74 24 0c     PUSH [ESP+0xc]  ; code
//   e8 ?? ?? ?? ??  CALL doexit
//   83 c4 0c        ADD ESP, 0xc
//   c3              RET
extern "C" void __cdecl exit(int code) {
    doexit(code, 0, 0);
}

// FUNCTION: ffxivgame 0x005d911f — _exit (17 B)
//   Same as exit but PUSH 1 instead of PUSH 0 for the `quick` arg.
extern "C" void __cdecl _exit(int code) {
    doexit(code, 1, 0);
}

// FUNCTION: ffxivgame 0x005d25c2 — atexit (18 B)
//   ff 74 24 04     PUSH [ESP+4]
//   e8 ?? ?? ?? ??  CALL _onexit
//   f7 d8           NEG EAX
//   1b c0           SBB EAX, EAX
//   f7 d8           NEG EAX
//   59              POP ECX
//   48              DEC EAX
//   c3              RET
//
// Pattern returns 0 if _onexit returned non-zero, -1 if 0.
extern "C" int __cdecl atexit(_PVFV func) {
    return _onexit(func) == 0 ? -1 : 0;
}

#pragma optimize("", on)
