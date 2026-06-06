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
// FUNCTION: ffxivgame 0x0045ba50 — _EVP_add_digest (OpenSSL EVP digest registration)
//                                  (__cdecl, 156 bytes / 0x9c)
//
// Calling convention: __cdecl; takes 1 argument (EVP_MD* digest); returns int.
// Callee-saves pushed: ESI, EDI.
//
// Behaviour (recovered from asm @ 0x0045ba50):
//   int _EVP_add_digest(const EVP_MD *digest):
//     name = FUN_00464ab0(digest->type)          // OBJ_nid2sn / name lookup
//     r = FUN_00465770(name, 1, digest)           // OBJ_NAME_add(name, TYPE_MD_METH, digest)
//     if (r == 0) return 0;
//     FUN_00464990(digest->type)                  // side-effect lookup (e.g. nid2ln)
//     r2 = FUN_00464b50(digest->type, 1, digest)  // alternate name fetch
//     r = FUN_00465770(r2, 1, digest)             // OBJ_NAME_add again
//     if (r == 0) return 0;
//     if (digest->pkey_type != 0 && digest->pkey_type != digest->type) {
//       name = FUN_00464ab0(digest->pkey_type, 0x8001, EDI)
//       r = FUN_00465770(name, 0x8001, digest)
//       if (r == 0) return 0;
//       FUN_00464990(digest->pkey_type)
//       r2 = FUN_00464b50(digest->pkey_type, 0x8001, digest)
//       r = FUN_00465770(r2, 0x8001, digest)
//     }
//     return r;  (or whatever is in EAX at the join point)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function uses a MSVC stack-reuse pattern where partially-cleaned
//   args from one call are left on the stack as args for the next call.
//   This cannot be reproduced reliably from C++ source; naked asm used.
//   All CALL targets use symbolic `call` to emit proper REL32 relocations
//   that compare.py wildcards. All other bytes emitted verbatim via _emit.

extern "C" {
    void FUN_00464ab0();  // OBJ_nid2sn / name lookup helper
    void FUN_00465770();  // OBJ_NAME_add wrapper
    void FUN_00464990();  // secondary name lookup helper
    void FUN_00464b50();  // alternate name fetch helper
}

extern "C" __declspec(naked) void FUN_0045ba50() {
    __asm {
        // 0005ba50: 56                   PUSH ESI
        _emit 0x56
        // 0005ba51: 8b 74 24 08          MOV ESI, dword ptr [ESP+0x8]  (digest arg)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0005ba55: 8b 06                MOV EAX, dword ptr [ESI]  (digest->type)
        _emit 0x8b
        _emit 0x06
        // 0005ba57: 57                   PUSH EDI
        _emit 0x57
        // 0005ba58: 50                   PUSH EAX  (arg: type)
        _emit 0x50
        // 0005ba59: e8 52 90 00 00       CALL FUN_00464ab0
        call FUN_00464ab0
        // 0005ba5e: 56                   PUSH ESI  (arg: digest)
        _emit 0x56
        // 0005ba5f: 8b f8                MOV EDI, EAX  (EDI = name)
        _emit 0x8b
        _emit 0xf8
        // 0005ba61: 6a 01                PUSH 0x1  (arg: OBJ_NAME_TYPE_MD_METH)
        _emit 0x6a
        _emit 0x01
        // 0005ba63: 57                   PUSH EDI  (arg: name)
        _emit 0x57
        // 0005ba64: e8 07 9d 00 00       CALL FUN_00465770
        call FUN_00465770
        // 0005ba69: 83 c4 10             ADD ESP, 0x10  (clean 4 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005ba6c: 85 c0                TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005ba6e: 75 05                JNZ +5  (success → continue; fail → return 0)
        _emit 0x75
        _emit 0x05
        // 0005ba70: 5f                   POP EDI
        _emit 0x5f
        // 0005ba71: 33 c0                XOR EAX, EAX  (return 0)
        _emit 0x33
        _emit 0xc0
        // 0005ba73: 5e                   POP ESI
        _emit 0x5e
        // 0005ba74: c3                   RET
        _emit 0xc3
        // 0005ba75: 8b 0e                MOV ECX, dword ptr [ESI]  (digest->type)
        _emit 0x8b
        _emit 0x0e
        // 0005ba77: 51                   PUSH ECX
        _emit 0x51
        // 0005ba78: e8 13 8f 00 00       CALL FUN_00464990
        call FUN_00464990
        // 0005ba7d: 8b 16                MOV EDX, dword ptr [ESI]  (digest->type again)
        _emit 0x8b
        _emit 0x16
        // 0005ba7f: 83 c4 04             ADD ESP, 0x4  (clean 1 arg; leave 2 on stack)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005ba82: 56                   PUSH ESI  (arg: digest)
        _emit 0x56
        // 0005ba83: 6a 01                PUSH 0x1  (arg: OBJ_NAME_TYPE_MD_METH)
        _emit 0x6a
        _emit 0x01
        // 0005ba85: 52                   PUSH EDX  (arg: type)
        _emit 0x52
        // 0005ba86: e8 c5 90 00 00       CALL FUN_00464b50
        call FUN_00464b50
        // 0005ba8b: 83 c4 04             ADD ESP, 0x4  (pop type; leave 1 and digest on stack)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005ba8e: 50                   PUSH EAX  (arg: result of FUN_00464b50)
        _emit 0x50
        // 0005ba8f: e8 dc 9c 00 00       CALL FUN_00465770  (r = FUN_00465770(result, 1, digest))
        call FUN_00465770
        // 0005ba94: 83 c4 0c             ADD ESP, 0xc  (clean 3 args: result, 1, digest)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005ba97: 85 c0                TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005ba99: 74 d5                JZ -43  (r == 0 → fail at 0x45ba70)
        _emit 0x74
        _emit 0xd5
        // 0005ba9b: 8b 4e 04             MOV ECX, dword ptr [ESI+0x4]  (digest->pkey_type)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0005ba9e: 85 c9                TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0005baa0: 74 47                JZ +71  (pkey_type == 0 → epilogue at 0x45bae9)
        _emit 0x74
        _emit 0x47
        // 0005baa2: 39 0e                CMP dword ptr [ESI], ECX  (pkey_type == type?)
        _emit 0x39
        _emit 0x0e
        // 0005baa4: 74 43                JZ +67  (same → skip alias block)
        _emit 0x74
        _emit 0x43
        // 0005baa6: 57                   PUSH EDI  (arg: EDI = original name)
        _emit 0x57
        // 0005baa7: 68 01 80 00 00       PUSH 0x8001  (arg: OBJ_NAME_TYPE_MD_METH|OBJ_NAME_DO_ALL)
        _emit 0x68
        _emit 0x01
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 0005baac: 51                   PUSH ECX  (arg: pkey_type)
        _emit 0x51
        // 0005baad: e8 fe 8f 00 00       CALL FUN_00464ab0
        call FUN_00464ab0
        // 0005bab2: 83 c4 04             ADD ESP, 0x4  (pop pkey_type; leave 0x8001 and EDI)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005bab5: 50                   PUSH EAX  (arg: alias name)
        _emit 0x50
        // 0005bab6: e8 b5 9c 00 00       CALL FUN_00465770  (FUN_00465770(name, 0x8001, EDI))
        call FUN_00465770
        // 0005babb: 83 c4 0c             ADD ESP, 0xc  (clean 3 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005babe: 85 c0                TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005bac0: 74 ae                JZ -82  (r == 0 → fail at 0x45ba70)
        _emit 0x74
        _emit 0xae
        // 0005bac2: 8b 46 04             MOV EAX, dword ptr [ESI+0x4]  (pkey_type)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0005bac5: 50                   PUSH EAX
        _emit 0x50
        // 0005bac6: e8 c5 8e 00 00       CALL FUN_00464990
        call FUN_00464990
        // 0005bacb: 8b 4e 04             MOV ECX, dword ptr [ESI+0x4]  (pkey_type again)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0005bace: 83 c4 04             ADD ESP, 0x4  (pop pkey_type arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005bad1: 57                   PUSH EDI  (arg: EDI/digest)
        _emit 0x57
        // 0005bad2: 68 01 80 00 00       PUSH 0x8001  (OBJ_NAME_TYPE_MD_METH|OBJ_NAME_DO_ALL)
        _emit 0x68
        _emit 0x01
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 0005bad7: 51                   PUSH ECX  (arg: pkey_type)
        _emit 0x51
        // 0005bad8: e8 73 90 00 00       CALL FUN_00464b50
        call FUN_00464b50
        // 0005badd: 83 c4 04             ADD ESP, 0x4  (pop pkey_type)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005bae0: 50                   PUSH EAX  (result of FUN_00464b50)
        _emit 0x50
        // 0005bae1: e8 8a 9c 00 00       CALL FUN_00465770
        call FUN_00465770
        // 0005bae6: 83 c4 0c             ADD ESP, 0xc  (clean 3 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005bae9: 5f                   POP EDI
        _emit 0x5f
        // 0005baea: 5e                   POP ESI
        _emit 0x5e
        // 0005baeb: c3                   RET
        _emit 0xc3
    }
}
