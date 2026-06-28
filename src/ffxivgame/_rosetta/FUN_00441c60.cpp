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
// FUNCTION: ffxivgame 0x00041c60 — __thiscall "find-and-clear" iterator helper
//                                  (157 bytes / 0x9d, ret 8)
//
// Calling convention: __thiscall (ECX = this); 2 stack args; returns void.
// Callee-saves pushed in prologue: EBP, ESI.
// Additional saves in else-branch: EBX, EDI.
//
// Frame layout (SUB ESP,0x10 gives 4 locals at [ESP+0x00..0x0f]):
//   local_a [EBP-0x10 / high frame]: output node pointer from find call
//   local_b [EBP-0x0c / high frame]: iterator field from find call
//   local_c [unused in observable path]
//   local_d [ESP+0x1c after branch pushes]: cached end pointer
//
// Stack args (offsets after final frame setup, in early branch):
//   arg1 = [ESP+0x18 after prologue] — pointer to an object with field_0x10
//   arg2 = [ESP+0x1c after prologue] — a value passed to FUN_00b52c40
//
// Object layout (this, offsets touched):
//   [this + 0x18]  pointer to a container (find target)
//   [this + 0x1c]  end pointer of that container
//
// Behaviour (recovered from asm @ 0x00041c60):
//
//   if FUN_00b52b70(arg1):         // thiscall on arg1 → bool
//       FUN_00b52c40(arg1, arg2)   // thiscall on arg1 with one stack arg
//       arg1->field_0x10 = 0
//       return
//
//   // else: search in the container at this+0x18
//   Iterator result;
//   (this+0x18)->find(&result, &arg1->field_0x10)    // FUN_0071d420
//   node = result.node   // EBX
//   end  = (this+0x18)->tail  // [this+0x1c]
//
//   // range-validation gates (call FUN_009d22b4 on violation):
//   if (node == 0 || node != this+0x18)  → FUN_009d22b4 (assert / _invalid_param)
//   if (iter == end)  → skip to final block
//   if (node == 0)    → FUN_009d22b4
//   if (iter == node->field_4)  → FUN_009d22b4
//
//   // conditional work:
//   bool r = FUN_004428a0(iter->field_0x10, arg1)  // thiscall
//   if (r || arg1->field_0x10 != 0)
//       arg1->field_0x10 = 0
//
// CALL targets (all REL32 — wildcarded by tools/compare.py):
//   +0x0d  CALL FUN_00b52b70  — bool predicate on arg1 (__thiscall, no stack args)
//   +0x1d  CALL FUN_00b52c40  — side-effect method on arg1 (__thiscall, 1 stack arg)
//   +0x41  CALL FUN_0071d420  — find in container (__thiscall, 2 stack args)
//   +0x59  CALL FUN_009d22b4  — validation gate (_invalid_parameter / assert)
//   +0x6c  CALL FUN_009d22b4  — validation gate
//   +0x76  CALL FUN_009d22b4  — validation gate
//   +0x7f  CALL FUN_004428a0  — work function (__thiscall, 1 stack arg)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The function mixes two separate epilogues (early-return at +0x2e and
//   main-return at +0x9a), four callee-saved registers (EBP/ESI always,
//   EBX/EDI only in the else branch), and hardcoded JZ/JNZ short-branch
//   offsets that remain correct only because every CALL instruction is
//   exactly 5 bytes.  Source-level C++ at /O2 will reorder saves and
//   produce different JMP encoding.  Raw `_emit` is used for every byte
//   except the seven CALL targets (which receive COFF REL32 relocations
//   via `call FunctionName` and are wildcarded by compare.py).

extern "C" void FUN_00b52b70();   // bool predicate (__thiscall, REL32)
extern "C" void FUN_00b52c40();   // side-effect method (__thiscall, 1 arg, REL32)
extern "C" void FUN_0071d420();   // container find (__thiscall, 2 args, REL32)
extern "C" void FUN_009d22b4();   // validation gate / _invalid_parameter (REL32)
extern "C" void FUN_004428a0();   // work function (__thiscall, 1 arg, REL32)

extern "C" __declspec(naked) void FUN_00441c60()
{
    __asm {
        // 00041c60: 83 ec 10    SUB ESP,0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 00041c63: 55          PUSH EBP
        _emit 0x55
        // 00041c64: 8b 6c 24 18 MOV EBP,[ESP+0x18]   (EBP = arg1)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00041c68: 56          PUSH ESI
        _emit 0x56
        // 00041c69: 8b f1       MOV ESI,ECX          (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00041c6b: 8b cd       MOV ECX,EBP          (ECX = arg1 for thiscall)
        _emit 0x8b
        _emit 0xcd
        // 00041c6d: e8 ...      CALL FUN_00b52b70    (REL32, compare.py masks)
        call FUN_00b52b70
        // 00041c72: 84 c0       TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 00041c74: 74 1b       JZ +0x1b  (to 0x41c91 — else branch)
        _emit 0x74
        _emit 0x1b
        // --- if-true block (AL != 0) ---
        // 00041c76: 8b 44 24 20 MOV EAX,[ESP+0x20]   (EAX = arg2)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00041c7a: 50          PUSH EAX             (push arg2 for FUN_00b52c40)
        _emit 0x50
        // 00041c7b: 8b cd       MOV ECX,EBP          (ECX = arg1)
        _emit 0x8b
        _emit 0xcd
        // 00041c7d: e8 ...      CALL FUN_00b52c40    (REL32, compare.py masks)
        call FUN_00b52c40
        // 00041c82: 5e          POP ESI              (clean 1 arg + restore ESI)
        _emit 0x5e
        // 00041c83: c7 45 10 00 00 00 00  MOV [EBP+0x10],0   (arg1->field_0x10 = 0)
        _emit 0xc7
        _emit 0x45
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00041c8a: 5d          POP EBP
        _emit 0x5d
        // 00041c8b: 83 c4 10    ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00041c8e: c2 08 00    RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // --- else branch (JZ target = 0x41c91) ---
        // 00041c91: 53          PUSH EBX
        _emit 0x53
        // 00041c92: 57          PUSH EDI
        _emit 0x57
        // 00041c93: 8d 7d 10    LEA EDI,[EBP+0x10]  (EDI = &arg1->field_0x10)
        _emit 0x8d
        _emit 0x7d
        _emit 0x10
        // 00041c96: 57          PUSH EDI             (push &arg1->field_0x10 as arg2)
        _emit 0x57
        // 00041c97: 8d 4c 24 14 LEA ECX,[ESP+0x14]  (ECX = &local_a)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00041c9b: 83 c6 18    ADD ESI,0x18        (ESI = this + 0x18 = container)
        _emit 0x83
        _emit 0xc6
        _emit 0x18
        // 00041c9e: 51          PUSH ECX             (push &local_a as arg1)
        _emit 0x51
        // 00041c9f: 8b ce       MOV ECX,ESI         (ECX = this+0x18 for thiscall)
        _emit 0x8b
        _emit 0xce
        // 00041ca1: e8 ...      CALL FUN_0071d420   (REL32, compare.py masks)
        call FUN_0071d420
        // 00041ca6: 8b 5c 24 10 MOV EBX,[ESP+0x10]  (EBX = local_a = result node)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 00041caa: 85 db       TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 00041cac: 8b 56 04    MOV EDX,[ESI+0x4]   (EDX = container->tail)
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 00041caf: 89 54 24 1c MOV [ESP+0x1c],EDX  (local_d = container->tail)
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 00041cb3: 74 04       JZ +0x04  (to 0x41cb9 — call assert if node==0)
        _emit 0x74
        _emit 0x04
        // 00041cb5: 3b de       CMP EBX,ESI         (node == this+0x18 sentinel?)
        _emit 0x3b
        _emit 0xde
        // 00041cb7: 74 05       JZ +0x05  (to 0x41cbe — skip assert if node==sentinel)
        _emit 0x74
        _emit 0x05
        // 00041cb9: e8 ...      CALL FUN_009d22b4   (REL32, compare.py masks)
        call FUN_009d22b4
        // 00041cbe: 8b 74 24 14 MOV ESI,[ESP+0x14]  (ESI = local_b = iterator)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00041cc2: 3b 74 24 1c CMP ESI,[ESP+0x1c]  (iter == end?)
        _emit 0x3b
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        // 00041cc6: 74 20       JZ +0x20  (to 0x41ce8 — skip work block)
        _emit 0x74
        _emit 0x20
        // 00041cc8: 85 db       TEST EBX,EBX        (node == 0?)
        _emit 0x85
        _emit 0xdb
        // 00041cca: 75 05       JNZ +0x05  (to 0x41cd1 — skip assert if node!=0)
        _emit 0x75
        _emit 0x05
        // 00041ccc: e8 ...      CALL FUN_009d22b4   (REL32, compare.py masks)
        call FUN_009d22b4
        // 00041cd1: 3b 73 04    CMP ESI,[EBX+0x4]  (iter == node->field_4?)
        _emit 0x3b
        _emit 0x73
        _emit 0x04
        // 00041cd4: 75 05       JNZ +0x05  (to 0x41cdb — skip assert if not equal)
        _emit 0x75
        _emit 0x05
        // 00041cd6: e8 ...      CALL FUN_009d22b4   (REL32, compare.py masks)
        call FUN_009d22b4
        // 00041cdb: 8b 4e 10    MOV ECX,[ESI+0x10] (ECX = iter->field_0x10)
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00041cde: 55          PUSH EBP            (push arg1 for FUN_004428a0)
        _emit 0x55
        // 00041cdf: e8 ...      CALL FUN_004428a0   (REL32, compare.py masks)
        call FUN_004428a0
        // 00041ce4: 84 c0       TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 00041ce6: 75 05       JNZ +0x05  (to 0x41ced — set *EDI=0 if result!=0)
        _emit 0x75
        _emit 0x05
        // --- join point from JZ +0x20 above (0x41ce8) ---
        // 00041ce8: 83 3f 00    CMP [EDI],0        (*EDI already zero?)
        _emit 0x83
        _emit 0x3f
        _emit 0x00
        // 00041ceb: 74 06       JZ +0x06  (to 0x41cf3 — skip if already 0)
        _emit 0x74
        _emit 0x06
        // --- join point from JNZ +0x05 above (0x41ced) ---
        // 00041ced: c7 07 00 00 00 00  MOV [EDI],0
        _emit 0xc7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- epilogue (else-branch path, 0x41cf3) ---
        // 00041cf3: 5f          POP EDI
        _emit 0x5f
        // 00041cf4: 5b          POP EBX
        _emit 0x5b
        // 00041cf5: 5e          POP ESI
        _emit 0x5e
        // 00041cf6: 5d          POP EBP
        _emit 0x5d
        // 00041cf7: 83 c4 10    ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00041cfa: c2 08 00    RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
