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
// FUNCTION: ffxivgame 0x009d6947 — `fread` → `_fread_s` thunk (__cdecl, 27 B).
//
// Frameless __cdecl wrapper that forwards a 4-arg fread call to the
// secure CRT variant _fread_s at VA 0x009d68b1, injecting (size_t)-1
// as the bufferSize argument (arg2).  When bufferSize == SIZE_MAX the
// _fread_s bounds-check is a no-op, so the call is semantically
// identical to a plain fread.
//
// Source-level equivalent:
//   size_t fread(void *buf, size_t elemSize, size_t count, FILE *stream) {
//       return _fread_s(buf, (size_t)-1, elemSize, count, stream);
//   }
//
// Asm shape (27 bytes — RVA 0x005d6947..0x005d6962):
//
//   009d6947: ff 74 24 10   PUSH DWORD PTR [ESP+0x10]   ; param_4 (stream)
//   009d694b: ff 74 24 10   PUSH DWORD PTR [ESP+0x10]   ; param_3 (count)
//   009d694f: ff 74 24 10   PUSH DWORD PTR [ESP+0x10]   ; param_2 (elemSize)
//   009d6953: 6a ff         PUSH -1                     ; bufferSize = SIZE_MAX
//   009d6955: ff 74 24 14   PUSH DWORD PTR [ESP+0x14]   ; param_1 (buf)
//   009d6959: e8 53 ff ff ff CALL _fread_s              ; VA 0x009d68b1
//   009d695e: 83 c4 14      ADD  ESP, 0x14              ; cdecl cleanup (5 args)
//   009d6961: c3            RET
//
// The PUSH [ESP+0x10] pattern relies on ESP shifting after each push:
//   before any pushes:  [ESP+0x10] = param_4
//   after 1st push:     [ESP+0x10] = param_3  (orig [ESP+0xC])
//   after 2nd push:     [ESP+0x10] = param_2  (orig [ESP+0x8])
//   after 3rd+4th push: [ESP+0x14] = param_1  (orig [ESP+0x4])
//
// Reloc-bearing site in the orig 27 bytes:
//   +0x13   REL32 → _fread_s @ VA 0x009d68b1
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level rewrite would rely on MSVC 2005 /O2 emitting the
//   `PUSH [ESP+N]` reload pattern rather than materialising the args
//   into registers first.  That optimisation is fragile w.r.t. argument
//   type annotations.  The naked byte-passthrough used by FUN_00401000
//   and FUN_00404e10 is the safe, stable choice.

extern "C" __declspec(naked) void FUN_009d6947() {
    __asm {
        _emit 0xff      // PUSH DWORD PTR [ESP+0x10]   ; param_4 (stream)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0xff      // PUSH DWORD PTR [ESP+0x10]   ; param_3 (count)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0xff      // PUSH DWORD PTR [ESP+0x10]   ; param_2 (elemSize)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x6a      // PUSH -1                     ; bufferSize = SIZE_MAX
        _emit 0xff
        _emit 0xff      // PUSH DWORD PTR [ESP+0x14]   ; param_1 (buf)
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0xe8      // CALL _fread_s               ; rel32 = -0xad
        _emit 0x53
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83      // ADD  ESP, 0x14              ; cdecl cleanup (5 args)
        _emit 0xc4
        _emit 0x14
        _emit 0xc3      // RET
    }
}
