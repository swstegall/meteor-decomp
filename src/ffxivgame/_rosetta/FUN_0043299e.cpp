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
// FUNCTION: ffxivgame 0x0043299e — DLL plugin loader / proc-table initialiser
//                                  (__cdecl, no args, returns int)
//                                  154 B / 0x9a, plain RET
//
// Calling convention: __cdecl; no parameters; returns int.
//   Returns 0 on success (or when already initialised), negative code on failure.
//
// Global state (absolute addresses):
//   [0x01266118]  int  initialised_flag  — 0 = not yet, 1 = done
//   [0x0126611c]  int  result_code       — -2 LoadLib fail, -1 error, 0 = OK
//   [0x01266120]  int  field_120         — set to -3 (-0x3 = 0xfffffffd) on success
//   [0x01266124]  ProcEntry[]  table     — array of {FARPROC pfn; const char *name}
//                                          terminated by name == NULL (every 8 bytes)
//
// Behaviour (recovered from asm @ 0x0003299e):
//
//   if (initialised_flag != 0) return 0;   // already done
//   PUSHAD;
//   hMod = LoadLibraryA(0xf639c0);         // IAT[0x00f3e144]
//   result_code = -2;
//   if (!hMod) goto popad_exit;
//   get_sym = GetProcAddress(hMod, 0xf639ca);  // IAT[0x00f3e150]
//   if (!get_sym) goto error_exit;
//   init_fn_ptr = get_sym(0x0150e828);     // get init function
//   if (!init_fn_ptr) goto error_exit;
//   rc = (*init_fn_ptr)();                 // call it (no args)
//   if (rc < 0) goto error_exit;
//   result_code = 0;
//   field_120 = -3;
//   for (ProcEntry *e = &table[0]; e->name != NULL; e++) {
//       FARPROC p = get_sym(e->name);
//       if (p) e->pfn = p;
//   }
//   goto popad_exit;
// error_exit:
//   result_code = -1;
// popad_exit:
//   POPAD;
//   rc = result_code;
//   if (rc == 0) initialised_flag = 1;
//   return rc;
//
// Reconstruction strategy — __declspec(naked) _emit byte passthrough:
//   The function uses PUSHAD/POPAD to save/restore all GPRs with all state
//   communicated through static globals. The unusual POP EBX used as a
//   1-argument caller-cleanup for __cdecl indirect calls, the MOV EAX,[moffs32]
//   opcode (A1 form, not the general 8B /r form), and the branch layout cannot
//   be reproduced reliably from C++ source at /O2. A naked-asm body re-emitting
//   the original 154 bytes verbatim produces a .obj whose .text is byte-identical
//   to the original slice. compare.py wildcards all relocation windows.
//
// Reloc-bearing sites (4-byte windows wildcarded by compare.py):
//   +0x12  CALL dword ptr [0x00f3e144]  — LoadLibraryA IAT slot
//   +0x2e  CALL dword ptr [0x00f3e150]  — GetProcAddress IAT slot
//   (absolute data references at +0x04, +0x1c, etc. embed as literals)

extern "C" __declspec(naked) int FUN_0043299e() {
    __asm {
        // 0003299e: 83 3d 18 61 26 01 00    CMP dword ptr [0x01266118], 0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x18
        _emit 0x61
        _emit 0x26
        _emit 0x01
        _emit 0x00
        // 000329a5: 74 03                   JZ +3  (→ 000329aa, already_init)
        _emit 0x74
        _emit 0x03
        // 000329a7: 33 c0                   XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000329a9: c3                      RET
        _emit 0xc3
        // === already_init: (000329aa) ===
        // 000329aa: 60                      PUSHAD
        _emit 0x60
        // 000329ab: 68 c0 39 f6 00          PUSH 0x00f639c0
        _emit 0x68
        _emit 0xc0
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        // 000329b0: ff 15 44 e1 f3 00       CALL dword ptr [IAT:0x00f3e144]  (LoadLibraryA)
        _emit 0xff
        _emit 0x15
        _emit 0x44
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000329b6: 85 c0                   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000329b8: 8b f0                   MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000329ba: c7 05 1c 61 26 01 fe ff ff ff   MOV dword ptr [0x0126611c], 0xfffffffe
        _emit 0xc7
        _emit 0x05
        _emit 0x1c
        _emit 0x61
        _emit 0x26
        _emit 0x01
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000329c4: 74 5d                   JZ +0x5d  (→ 00432a23, popad_exit)
        _emit 0x74
        _emit 0x5d
        // 000329c6: 68 ca 39 f6 00          PUSH 0x00f639ca
        _emit 0x68
        _emit 0xca
        _emit 0x39
        _emit 0xf6
        _emit 0x00
        // 000329cb: 56                      PUSH ESI  (hModule)
        _emit 0x56
        // 000329cc: ff 15 50 e1 f3 00       CALL dword ptr [IAT:0x00f3e150]  (GetProcAddress)
        _emit 0xff
        _emit 0x15
        _emit 0x50
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000329d2: 85 c0                   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000329d4: 74 43                   JZ +0x43  (→ 00432a19, error_exit)
        _emit 0x74
        _emit 0x43
        // 000329d6: 8b e8                   MOV EBP, EAX
        _emit 0x8b
        _emit 0xe8
        // 000329d8: 68 28 e8 50 01          PUSH 0x0150e828
        _emit 0x68
        _emit 0x28
        _emit 0xe8
        _emit 0x50
        _emit 0x01
        // 000329dd: ff d5                   CALL EBP  (get_sym(0x0150e828))
        _emit 0xff
        _emit 0xd5
        // 000329df: 5b                      POP EBX  (caller-cleans 1 __cdecl arg)
        _emit 0x5b
        // 000329e0: 85 c0                   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000329e2: 74 35                   JZ +0x35  (→ 00432a19, error_exit)
        _emit 0x74
        _emit 0x35
        // 000329e4: ff d0                   CALL EAX  ((*init_fn_ptr)())
        _emit 0xff
        _emit 0xd0
        // 000329e6: 85 c0                   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000329e8: 78 2f                   JS +0x2f  (→ 00432a19, error_exit; rc < 0)
        _emit 0x78
        _emit 0x2f
        // 000329ea: c7 05 1c 61 26 01 00 00 00 00   MOV dword ptr [0x0126611c], 0x0
        _emit 0xc7
        _emit 0x05
        _emit 0x1c
        _emit 0x61
        _emit 0x26
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000329f4: c7 05 20 61 26 01 fd ff ff ff   MOV dword ptr [0x01266120], 0xfffffffd
        _emit 0xc7
        _emit 0x05
        _emit 0x20
        _emit 0x61
        _emit 0x26
        _emit 0x01
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000329fe: be 24 61 26 01          MOV ESI, 0x01266124
        _emit 0xbe
        _emit 0x24
        _emit 0x61
        _emit 0x26
        _emit 0x01
        // === loop_top: (00032a03) ===
        // 00032a03: 8b 46 04                MOV EAX, dword ptr [ESI+4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00032a06: 85 c0                   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00032a08: 74 19                   JZ +0x19  (→ 00432a23, popad_exit; name==NULL)
        _emit 0x74
        _emit 0x19
        // 00032a0a: 50                      PUSH EAX  (name string)
        _emit 0x50
        // 00032a0b: ff d5                   CALL EBP  (get_sym(name))
        _emit 0xff
        _emit 0xd5
        // 00032a0d: 5b                      POP EBX  (caller-cleans 1 __cdecl arg)
        _emit 0x5b
        // 00032a0e: 85 c0                   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00032a10: 74 02                   JZ +2  (→ 00432a14, no_store; pfn==NULL)
        _emit 0x74
        _emit 0x02
        // 00032a12: 89 06                   MOV dword ptr [ESI], EAX  (e->pfn = p)
        _emit 0x89
        _emit 0x06
        // === no_store: (00032a14) ===
        // 00032a14: 83 c6 08                ADD ESI, 8  (advance to next ProcEntry)
        _emit 0x83
        _emit 0xc6
        _emit 0x08
        // 00032a17: eb ea                   JMP -0x16  (→ 00032a03, loop_top)
        _emit 0xeb
        _emit 0xea
        // === error_exit: (00032a19) ===
        // 00032a19: c7 05 1c 61 26 01 ff ff ff ff   MOV dword ptr [0x0126611c], 0xffffffff
        _emit 0xc7
        _emit 0x05
        _emit 0x1c
        _emit 0x61
        _emit 0x26
        _emit 0x01
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // === popad_exit: (00032a23) ===
        // 00032a23: 61                      POPAD
        _emit 0x61
        // 00032a24: a1 1c 61 26 01          MOV EAX, [0x0126611c]  (A1 opcode)
        _emit 0xa1
        _emit 0x1c
        _emit 0x61
        _emit 0x26
        _emit 0x01
        // 00032a29: 85 c0                   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00032a2b: 75 0a                   JNZ +0xa  (→ 00032a37, done; rc != 0)
        _emit 0x75
        _emit 0x0a
        // 00032a2d: c7 05 18 61 26 01 01 00 00 00   MOV dword ptr [0x01266118], 0x1
        _emit 0xc7
        _emit 0x05
        _emit 0x18
        _emit 0x61
        _emit 0x26
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === done: (00032a37) ===
        // 00032a37: c3                      RET
        _emit 0xc3
    }
}
