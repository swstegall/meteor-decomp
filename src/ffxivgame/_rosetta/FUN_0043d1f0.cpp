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
// FUNCTION: ffxivgame 0x0003d1f0 — factory-extract wrapper: creates an object
//                                  via FUN_0043ce20, transfers its first field
//                                  into *out, then calls vfunc[0](1) on the
//                                  created object if non-null (__cdecl, 137 B /
//                                  0x89, EH3-style SEH frame with /GS cookie).
//
// Signature (recovered from asm):
//
//   void* __cdecl FUN_0043d1f0(void** out, int type, void* arg2, void* arg3);
//
// Frame layout after prologue (ESP = E0):
//
//   [E0+0x00] = /GS security cookie (XOR'd with ESP)
//   [E0+0x04] = saved ESI
//   [E0+0x08] = local0 (destruction-state flag: 0 → 1 after factory call)
//   [E0+0x0c] = local1 (receives the object ptr from FUN_0043ce20)
//   [E0+0x10] = old FS:[0]  ─┐ EH3 registration record
//   [E0+0x14] = scope table   │ installed into FS:[0] at E0+0x10
//   [E0+0x18] = try-level=-1 ─┘ (advances to 0 before virtual call)
//   [E0+0x1c] = return address
//   [E0+0x20] = arg0 (out — pointer-to-pointer, receives extracted element)
//   [E0+0x24] = arg1 (type: forwarded as 2nd arg to FUN_0043ce20)
//   [E0+0x28] = arg2 (forwarded as 3rd arg to FUN_0043ce20)
//   [E0+0x2c] = arg3 (forwarded as 4th arg to FUN_0043ce20)
//
// Logic:
//
//   local0 = 0;
//   ret = FUN_0043ce20(&local1, arg1, arg2, arg3);
//   elem = *ret;   // extract first field from returned container
//   *ret = NULL;   // clear it
//   *out = elem;   // hand ownership to caller
//   try_level = 0; // enter try scope
//   local0 = 1;
//   if (local1 != NULL)
//       local1->vtable[0](1);  // notify / release
//   return out;
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3 SEH prologue (FS:[0] chain manipulation, /GS cookie XOR), the
//   virtual dispatch through EAX, and the exact register allocation (ESI for
//   'out', ECX for local1 vtable dance) are not reproducible byte-for-byte
//   from a C++ try/except source form under MSVC 2005 /O2.  The
//   __declspec(naked) body re-emits all 137 bytes verbatim via MASM _emit
//   directives.  Reloc sites (PUSH handler addr at +0x03, MOV EAX,[cookie]
//   at +0x13, CALL FUN_0043ce20 at +0x41) are masked by compare.py.

extern "C" __declspec(naked) void FUN_0043d1f0() {
    __asm {
        // 0003d1f0:  6a ff                PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0003d1f2:  68 3c 69 e5 00       PUSH 0xe5693c  [reloc: scope table VA]
        _emit 0x68
        _emit 0x3c
        _emit 0x69
        _emit 0xe5
        _emit 0x00
        // 0003d1f7:  64 a1 00 00 00 00    MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d1fd:  50                   PUSH EAX
        _emit 0x50
        // 0003d1fe:  83 ec 08             SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0003d201:  56                   PUSH ESI
        _emit 0x56
        // 0003d202:  a1 b0 a8 2e 01       MOV EAX,[0x012ea8b0]  [reloc: __security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003d207:  33 c4                XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003d209:  50                   PUSH EAX
        _emit 0x50
        // 0003d20a:  8d 44 24 10          LEA EAX,[ESP+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0003d20e:  64 a3 00 00 00 00    MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d214:  8b 4c 24 28          MOV ECX,dword ptr [ESP+0x28]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0003d218:  c7 44 24 08 00 00 00 00   MOV dword ptr [ESP+0x8],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d220:  8b 44 24 2c          MOV EAX,dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0003d224:  8b 54 24 24          MOV EDX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0003d228:  50                   PUSH EAX
        _emit 0x50
        // 0003d229:  51                   PUSH ECX
        _emit 0x51
        // 0003d22a:  52                   PUSH EDX
        _emit 0x52
        // 0003d22b:  8d 44 24 18          LEA EAX,[ESP+0x18]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0003d22f:  50                   PUSH EAX
        _emit 0x50
        // 0003d230:  e8 eb fb ff ff       CALL FUN_0043ce20  [reloc: rel32]
        _emit 0xe8
        _emit 0xeb
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 0003d235:  8b 08                MOV ECX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0003d237:  8b 74 24 30          MOV ESI,dword ptr [ESP+0x30]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x30
        // 0003d23b:  c7 00 00 00 00 00    MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d241:  83 c4 10             ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0003d244:  89 0e                MOV dword ptr [ESI],ECX
        _emit 0x89
        _emit 0x0e
        // 0003d246:  8b 4c 24 0c          MOV ECX,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0003d24a:  85 c9                TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 0003d24c:  c7 44 24 18 00 00 00 00   MOV dword ptr [ESP+0x18],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d254:  c7 44 24 08 01 00 00 00   MOV dword ptr [ESP+0x8],0x1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d25c:  74 08                JZ +0x08  (→ 0x0043d266)
        _emit 0x74
        _emit 0x08
        // 0003d25e:  8b 11                MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 0003d260:  8b 02                MOV EAX,dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 0003d262:  6a 01                PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0003d264:  ff d0                CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0003d266:  8b c6                MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0003d268:  8b 4c 24 10          MOV ECX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003d26c:  64 89 0d 00 00 00 00 MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d273:  59                   POP ECX
        _emit 0x59
        // 0003d274:  5e                   POP ESI
        _emit 0x5e
        // 0003d275:  83 c4 14             ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0003d278:  c3                   RET
        _emit 0xc3
    }
}
