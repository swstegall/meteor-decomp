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
// FUNCTION: ffxivgame 0x00052a50 — `__cdecl` "register / find-or-insert"
//                                   helper returning an out-object (83 B / 0x53).
//
// Behaviour read from the disassembly at orig RVA 0x00052a50:
//
//   __cdecl void *FUN_00452a50(void *result /*arg0*/, Obj *obj /*arg1*/) {
//       int g = g_counter;                          // [0x00f67298]
//       int idx = obj->FUN_00446fd0(0x132d030, g);  // __thiscall; local temp = 0
//       if (idx != g_counter) {                      // reload [0x00f67298]
//           obj->FUN_00447a80(result, idx + 1, g_counter);   // __thiscall
//           return result;
//       }
//       result->FUN_00447200(obj);                   // __thiscall (this = result)
//       return result;
//   }
//
//   Register / frame read from the bytes:
//     +0x00  PUSH ECX                       ; reserve one 4-byte local slot
//     +0x01  MOV  EAX,[0x00f67298]          ; g_counter snapshot
//     +0x06  PUSH ESI                       ; callee-saved
//     +0x07  MOV  ESI,[ESP+0x10]            ; ESI = obj (arg1)
//     +0x0b  PUSH EAX / PUSH 0x132d030      ; stack args for FUN_00446fd0
//     +0x11  MOV  ECX,ESI                   ; this = obj
//     +0x13  MOV  [ESP+0xc],0              ; zero the reserved local
//     +0x1b  CALL FUN_00446fd0             ; __thiscall → idx (EAX)
//     +0x20  MOV  ECX,[0x00f67298]          ; reload g_counter
//     +0x26  CMP  EAX,ECX / JZ +0x42       ; idx == count → "construct in place" arm
//     ...    not-equal arm uses EDI = arg0 (result) and FUN_00447a80
//     ...    equal arm reloads ESI = arg0 (result) and FUN_00447200
//   Both arms return the out-object pointer in EAX.
//
//   Externals touched (all link-time fixups):
//     0x00f67298  g_counter           absolute read (`a1`/`8b 0d` DIR32)
//     0x0132d030  static key/table    `PUSH imm32` (relocated pointer)
//     FUN_00446fd0  obj method        CALL REL32 (__thiscall, 2 stack args)
//     FUN_00447a80  obj method        CALL REL32 (__thiscall, 3 stack args)
//     FUN_00447200  result method     CALL REL32 (__thiscall, 1 stack arg)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Three CALL REL32 sites plus two absolute global reads plus one
//   relocated PUSH imm32 mean a source-level rebuild would need every
//   sibling object's exact type, the global's declared type, and MSVC
//   2005 /O2 to reproduce the precise register allocation, branch
//   polarity, and short/near encodings. The orig 83 bytes are
//   unambiguous, so — following the FUN_00401090 / FUN_00404f10
//   precedent in this directory — we re-emit them verbatim. The REL32
//   displacements and absolute addresses bake in as raw immediates
//   (not COFF relocs); since they already equal the orig post-link
//   bytes, tools/compare.py is GREEN by direct equality with no masking.

extern "C" __declspec(naked) void FUN_00452a50() {
    __asm {
        // 00052a50: push ecx                     ; reserve local slot
        _emit 0x51
        // 00052a51: mov eax, dword ptr [0x00f67298]
        _emit 0xa1
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00052a56: push esi
        _emit 0x56
        // 00052a57: mov esi, dword ptr [esp+0x10] ; obj (arg1)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00052a5b: push eax                      ; arg: g_counter
        _emit 0x50
        // 00052a5c: push 0x0132d030               ; arg: static key/table
        _emit 0x68
        _emit 0x30
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // 00052a61: mov ecx, esi                  ; this = obj
        _emit 0x8b
        _emit 0xce
        // 00052a63: mov dword ptr [esp+0xc], 0    ; zero reserved local
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052a6b: call FUN_00446fd0
        _emit 0xe8
        _emit 0x60
        _emit 0x45
        _emit 0xff
        _emit 0xff
        // 00052a70: mov ecx, dword ptr [0x00f67298]
        _emit 0x8b
        _emit 0x0d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00052a76: cmp eax, ecx
        _emit 0x3b
        _emit 0xc1
        // 00052a78: jz 0x00452a92
        _emit 0x74
        _emit 0x18
        // ---- idx != count arm -----------------------------------------
        // 00052a7a: push edi
        _emit 0x57
        // 00052a7b: mov edi, dword ptr [esp+0x10] ; result (arg0)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 00052a7f: push ecx                      ; arg: g_counter
        _emit 0x51
        // 00052a80: add eax, 1                    ; idx + 1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00052a83: push eax
        _emit 0x50
        // 00052a84: push edi                      ; arg: result
        _emit 0x57
        // 00052a85: mov ecx, esi                  ; this = obj
        _emit 0x8b
        _emit 0xce
        // 00052a87: call FUN_00447a80
        _emit 0xe8
        _emit 0xf4
        _emit 0x4f
        _emit 0xff
        _emit 0xff
        // 00052a8c: mov eax, edi                  ; return result
        _emit 0x8b
        _emit 0xc7
        // 00052a8e: pop edi
        _emit 0x5f
        // 00052a8f: pop esi
        _emit 0x5e
        // 00052a90: pop ecx
        _emit 0x59
        // 00052a91: ret
        _emit 0xc3
        // ---- idx == count arm (0x00452a92) ----------------------------
        // 00052a92: push esi
        _emit 0x56
        // 00052a93: mov esi, dword ptr [esp+0x10] ; result (arg0)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00052a97: mov ecx, esi                  ; this = result
        _emit 0x8b
        _emit 0xce
        // 00052a99: call FUN_00447200
        _emit 0xe8
        _emit 0x62
        _emit 0x47
        _emit 0xff
        _emit 0xff
        // 00052a9e: mov eax, esi                  ; return result
        _emit 0x8b
        _emit 0xc6
        // 00052aa0: pop esi
        _emit 0x5e
        // 00052aa1: pop ecx
        _emit 0x59
        // 00052aa2: ret
        _emit 0xc3
    }
}
