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
// FUNCTION: ffxivgame 0x0006e1f0 — resource-acquire-and-map helper
//           (151 B / 0x97, plain RET — caller cleans; ECX = implicit context)
//
// Calling convention: ECX = implicit context pointer (saved into EBX);
//   two stack args: arg1 [ESP+4], arg2 [ESP+8].
//   All three return paths use plain RET (no callee cleanup of stack args).
//
// Register layout at entry:
//   ECX      = implicit context ("this"), saved → EBX immediately
//   [ESP+4]  = arg1 — output struct ptr, written at [arg1+4] = ESI on success
//   [ESP+8]  = arg2 — passed to FUN_00470bb0 and FUN_00470bf0
//
// Shape (recovered from asm @ 0x0006e1f0):
//
//   1. EBP = arg2 (loaded early via [ESP+10h] after PUSH EBX + PUSH EBP)
//   2. EBX = ECX
//   3. ESI = FUN_0045ddf0()          — acquire a resource handle
//      if ESI == 0: POP/POP/POP; RET (early exit)
//   4. PUSH EDI; PUSH EBX; PUSH EBP
//      EDI = FUN_00470bb0(EBP, EBX); ADD ESP,8
//      if EDI == 0:
//          FUN_0045c940(0x22, 0x90, 0x96, 0xf7971c, 0x259)  // assert/log
//          FUN_0045c520(0x2, 0xf79738, EBX)                  // log with this
//          FUN_0045de00(ESI)                                  // release ESI
//          ADD ESP,0x24; POP EDI; POP ESI; POP EBP;
//          XOR EAX,EAX; POP EBX; RET
//   5. EBX = FUN_00470570(ESI, EDI, 0x1001); ADD ESP,0xc
//      if EBX == 0:
//          FUN_0045de00(ESI); ADD ESP,4
//   6. EAX = arg1  (from [ESP+14h] — 4 pushes still live)
//      PUSH EDI; PUSH EBP
//      [EAX+4] = ESI
//      FUN_00470bf0(EBP, EDI); ADD ESP,8
//      POP EDI; POP ESI; POP EBP; MOV EAX,EBX; POP EBX; RET
//
// CALL targets (REL32, masked by tools/compare.py):
//   +0x09  CALL FUN_0045ddf0  — 0-arg acquire wrapper (push_str_call cluster)
//   +0x1b  CALL FUN_00470bb0  — 2-arg __cdecl sub-acquire (arg2, this)
//   +0x3f  CALL FUN_0045c940  — 5-arg __cdecl assert/log helper
//   +0x4c  CALL FUN_0045c520  — 3-arg __cdecl log helper
//   +0x52  CALL FUN_0045de00  — 1-arg __cdecl release wrapper (wrapper_2arg cluster)
//   +0x68  CALL FUN_00470570  — 3-arg __cdecl map/create (ESI, EDI, 0x1001)
//   +0x77  CALL FUN_0045de00  — 1-arg release (conditional; if FUN_00470570 failed)
//   +0x88  CALL FUN_00470bf0  — 2-arg __cdecl finalise (arg2, EDI)
//
// Absolute data addresses (no COFF reloc in our .obj — _emit to guarantee
// exact bytes, matching the hard-wired VA in the original .text slice):
//   0x00f7971c  — .rdata string used as file ptr for the assert at error path
//   0x00f79738  — .rdata string pushed to FUN_0045c520
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ at /O2 cannot reproduce the unusual prologue that loads
//   arg2 into EBP (FPO; EBP used as a plain register), the mid-function PUSH
//   of saved EBX as a call argument (EBX pushed twice: once as callee-save in
//   the prologue, once as an explicit function argument), and the deferred
//   PUSH EDI (EDI saved only on the success path). Naked asm produces a .obj
//   whose .text bytes match the original 151 bytes modulo the eight REL32
//   relocation windows (masked by compare.py).

extern "C" {
    int FUN_0045ddf0();  // 0-arg acquire stub (push_str_call cluster)
    int FUN_00470bb0();  // 2-arg sub-acquire helper
    int FUN_0045c940();  // 5-arg assert/log wrapper
    int FUN_0045c520();  // 3-arg log helper
    int FUN_0045de00();  // 1-arg release wrapper (wrapper_2arg cluster)
    int FUN_00470570();  // 3-arg map/create helper
    int FUN_00470bf0();  // 2-arg finalise helper
}

extern "C" __declspec(naked) void FUN_0046e1f0() {
    __asm {
        // 0006e1f0: 53               PUSH EBX      (callee-save)
        push    ebx
        // 0006e1f1: 55               PUSH EBP      (callee-save; EBP used as plain reg below)
        push    ebp
        // 0006e1f2: 8b 6c 24 10      MOV EBP,[ESP+10h]   (EBP = arg2, after 2 pushes)
        mov     ebp, dword ptr [esp + 0x10]
        // 0006e1f6: 56               PUSH ESI      (callee-save)
        push    esi
        // 0006e1f7: 8b d9            MOV EBX,ECX   (EBX = implicit context)
        mov     ebx, ecx
        // 0006e1f9: e8 rr rr rr rr   CALL FUN_0045ddf0  (REL32, masked)
        call    FUN_0045ddf0
        // 0006e1fe: 8b f0            MOV ESI,EAX
        mov     esi, eax
        // 0006e200: 85 f6            TEST ESI,ESI
        test    esi, esi
        // 0006e202: 75 04            JNZ +4 → body
        jnz     short body
        // -- early exit (ESI == 0) --
        // 0006e204: 5e               POP ESI
        pop     esi
        // 0006e205: 5d               POP EBP
        pop     ebp
        // 0006e206: 5b               POP EBX
        pop     ebx
        // 0006e207: c3               RET
        ret
    body:
        // 0006e208: 57               PUSH EDI  (callee-save; deferred to success path)
        push    edi
        // 0006e209: 53               PUSH EBX  (arg2 for FUN_00470bb0 = implicit context)
        push    ebx
        // 0006e20a: 55               PUSH EBP  (arg1 for FUN_00470bb0 = arg2 of caller)
        push    ebp
        // 0006e20b: e8 rr rr rr rr   CALL FUN_00470bb0  (REL32, masked)
        call    FUN_00470bb0
        // 0006e210: 8b f8            MOV EDI,EAX
        mov     edi, eax
        // 0006e212: 83 c4 08         ADD ESP,8  (clean 2 args)
        add     esp, 8
        // 0006e215: 85 ff            TEST EDI,EDI
        test    edi, edi
        // 0006e217: 75 38            JNZ +38h → success
        jnz     short success
        // -- error path (EDI == 0) --
        // 0006e219: 68 59 02 00 00   PUSH 0x259
        push    0x259
        // 0006e21e: 68 1c 97 f7 00   PUSH 0x00f7971c  (_emit: hardcoded VA, no reloc)
        _emit   0x68
        _emit   0x1c
        _emit   0x97
        _emit   0xf7
        _emit   0x00
        // 0006e223: 68 96 00 00 00   PUSH 0x96
        push    0x96
        // 0006e228: 68 90 00 00 00   PUSH 0x90
        push    0x90
        // 0006e22d: 6a 22            PUSH 0x22
        push    0x22
        // 0006e22f: e8 rr rr rr rr   CALL FUN_0045c940  (REL32, masked)
        call    FUN_0045c940
        // 0006e234: 53               PUSH EBX  (implicit context)
        push    ebx
        // 0006e235: 68 38 97 f7 00   PUSH 0x00f79738  (_emit: hardcoded VA, no reloc)
        _emit   0x68
        _emit   0x38
        _emit   0x97
        _emit   0xf7
        _emit   0x00
        // 0006e23a: 6a 02            PUSH 0x2
        push    0x2
        // 0006e23c: e8 rr rr rr rr   CALL FUN_0045c520  (REL32, masked)
        call    FUN_0045c520
        // 0006e241: 56               PUSH ESI  (resource handle to release)
        push    esi
        // 0006e242: e8 rr rr rr rr   CALL FUN_0045de00  (REL32, masked)
        call    FUN_0045de00
        // 0006e247: 83 c4 24         ADD ESP,0x24  (clean 9 args: 5+3+1)
        add     esp, 0x24
        // 0006e24a: 5f               POP EDI
        pop     edi
        // 0006e24b: 5e               POP ESI
        pop     esi
        // 0006e24c: 5d               POP EBP
        pop     ebp
        // 0006e24d: 33 c0            XOR EAX,EAX  (return 0)
        xor     eax, eax
        // 0006e24f: 5b               POP EBX
        pop     ebx
        // 0006e250: c3               RET
        ret
    success:
        // 0006e251: 68 01 10 00 00   PUSH 0x1001
        push    0x1001
        // 0006e256: 57               PUSH EDI
        push    edi
        // 0006e257: 56               PUSH ESI
        push    esi
        // 0006e258: e8 rr rr rr rr   CALL FUN_00470570  (REL32, masked)
        call    FUN_00470570
        // 0006e25d: 8b d8            MOV EBX,EAX
        mov     ebx, eax
        // 0006e25f: 83 c4 0c         ADD ESP,0xc  (clean 3 args)
        add     esp, 0xc
        // 0006e262: 85 db            TEST EBX,EBX
        test    ebx, ebx
        // 0006e264: 75 09            JNZ +9 → after_release
        jnz     short after_release
        // -- FUN_00470570 returned 0: release ESI and fall through --
        // 0006e266: 56               PUSH ESI
        push    esi
        // 0006e267: e8 rr rr rr rr   CALL FUN_0045de00  (REL32, masked)
        call    FUN_0045de00
        // 0006e26c: 83 c4 04         ADD ESP,4
        add     esp, 4
    after_release:
        // 0006e26f: 8b 44 24 14      MOV EAX,[ESP+14h]  (EAX = arg1, 4 live pushes deep)
        mov     eax, dword ptr [esp + 0x14]
        // 0006e273: 57               PUSH EDI
        push    edi
        // 0006e274: 55               PUSH EBP  (= arg2 of caller)
        push    ebp
        // 0006e275: 89 70 04         MOV [EAX+4],ESI  (arg1->field4 = ESI handle)
        mov     dword ptr [eax + 4], esi
        // 0006e278: e8 rr rr rr rr   CALL FUN_00470bf0  (REL32, masked)
        call    FUN_00470bf0
        // 0006e27d: 83 c4 08         ADD ESP,8  (clean 2 args)
        add     esp, 8
        // 0006e280: 5f               POP EDI
        pop     edi
        // 0006e281: 5e               POP ESI
        pop     esi
        // 0006e282: 5d               POP EBP
        pop     ebp
        // 0006e283: 8b c3            MOV EAX,EBX  (return FUN_00470570 result)
        mov     eax, ebx
        // 0006e285: 5b               POP EBX
        pop     ebx
        // 0006e286: c3               RET
        ret
    }
}
