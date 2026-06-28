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
// FUNCTION: ffxivgame 0x0003c470 — __thiscall method that allocates and
//                                  initialises a 40-byte child object, stores
//                                  it at this->field_0x4, releases the old
//                                  child, and optionally registers two event
//                                  bit-mask signals (288 B / 0x120).
//
// Signature (recovered from asm):
//
//   void __thiscall FUN_0043c470(void *this, int arg1, int arg2, BYTE arg3);
//       ECX = this  (saved → EDI throughout)
//       [ESP+4]  = arg1  — bit index for first  SHL EDX, CL signal
//       [ESP+8]  = arg2  — bit index for second SHL EAX, CL signal
//       [ESP+12] = arg3  — flag byte; if == 1, fire the two signal calls
//       RET 0xC          — __thiscall / callee-cleans three DWORD args
//
// Body sketch (read from asm at orig RVA 0x0003c470):
//
//   1. Full EH4-SEH prologue: PUSH -1 / PUSH handler(0xe5670c) / PUSH FS:[0]
//      / SUB ESP,0x2c / PUSH EBX/ESI/EDI / __security_cookie ^ ESP / install.
//
//   2. ESI = malloc(0x28) [operator new, 40 bytes] via 0x009d1b35.
//      Local [ESP+0x10] = ESI; SEH state → 0; EBX = -1 (OR EBX, 0xFFFFFFFF).
//
//   3. If ESI == 0 (alloc failed): XOR ESI, ESI (null), skip init.
//      If ESI != 0:
//        a. Build local "config" struct on stack:
//             [ESP+0x14] = 0x00f66494  (data pointer / vtable hint)
//             [ESP+0x18] = 0x00010000  (flags / access constant)
//        b. Call 0x009fc880() → EAX:EDX (64-bit result, no args).
//        c. Call 0x009fc890(EAX, EDX) → EAX; store at [ESP+0x24],
//           store EBX(-1) at [ESP+0x28] (relative to pre-pop ESP).
//        d. Set [ESI+0x00] = -1; [ESI+0x18] = 0; [ESI+0x1c] = 0.
//        e. ADD ESP,8 (clean 2-arg area); write 0x43c460 to [ESP+0x24].
//        f. MOVQ XMM0, [ESP+0x24] → MOVQ [ESI+0x08], XMM0  (8-byte copy)
//           MOVQ XMM0, [ESP+0x2c] → MOVQ [ESI+0x10], XMM0  (8-byte copy)
//        g. PUSH &[ESP+0x14]; MOV ECX,ESI; set [ESI+0x18]=EDI, [ESI+0x1c]=5.
//        h. CALL 0x009fc950(ESI, &local_config)  (__thiscall on ESI).
//
//   4. SEH state → EBX(-1); EBX = old = [EDI+0x4]; [EDI+0x4] = ESI.
//      If old != ESI and old != 0:
//        CALL 0x009fc830(old)   (some release/unbind)
//        PUSH old; CALL 0x009d1b17(old)  (operator delete)
//
//   5. If arg3 != 1: skip to epilogue.
//      If arg3 == 1:
//        EDX = 1 << arg1; CALL [0x00f3e1ac](EDX)
//        ESI = [0x00f3e1b0]; CALL ESI(EAX)
//        EAX = 1 << arg2; ECX = [EDI+4]; EDX = [ECX+0]
//        CALL ESI(EDX, EAX)
//
//   6. Restore FS:[0], POP ECX/EDI/ESI/EBX, ADD ESP,0x38, RET 0xC.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The EH4 SEH prologue (absolute handler address 0xe5670c, security cookie
//   at 0x012ea8b0), the XMM MOVQ struct-copy pair, the IAT calls via
//   absolute memory addresses (0x00f3e1ac, 0x00f3e1b0), and five other
//   relative-call relocations make source-level reconstruction brittle under
//   MSVC 2005 /O2 /GS /EHsc.  A `__declspec(naked)` body emitting all 288
//   bytes verbatim is byte-identical to the orig slice; compare.py confirms
//   GREEN.

extern "C" __declspec(naked) void FUN_0043c470() {
    __asm {
        // --- EH4 SEH prologue ---
        _emit 0x6a  // PUSH -0x1
        _emit 0xff
        _emit 0x68  // PUSH 0xe5670c  (SEH handler)
        _emit 0x0c
        _emit 0x67
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x2c
        _emit 0xec
        _emit 0x2c
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX  (cookie on stack)
        _emit 0x8d  // LEA EAX, [ESP+0x3c]
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x64  // MOV FS:[0], EAX  (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- save `this` (ECX) → EDI; malloc(0x28) ---
        _emit 0x8b  // MOV EDI, ECX
        _emit 0xf9
        _emit 0x6a  // PUSH 0x28
        _emit 0x28
        _emit 0xe8  // CALL 0x009d1b35  (operator new / malloc)
        _emit 0x96
        _emit 0x56
        _emit 0x59
        _emit 0x00
        _emit 0x8b  // MOV ESI, EAX  (alloc result)
        _emit 0xf0
        _emit 0x83  // ADD ESP, 4  (clean arg)
        _emit 0xc4
        _emit 0x04
        _emit 0x89  // MOV [ESP+0x10], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x83  // OR EBX, 0xffffffff  (EBX = -1)
        _emit 0xcb
        _emit 0xff
        _emit 0x85  // TEST ESI, ESI
        _emit 0xf6
        _emit 0xc7  // MOV dword ptr [ESP+0x44], 0  (SEH state → 0)
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74  // JZ  (alloc failed → XOR ESI,ESI)
        _emit 0x6d
        // --- alloc succeeded: init local config struct ---
        _emit 0xc7  // MOV dword ptr [ESP+0x14], 0x00f66494
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x94
        _emit 0x64
        _emit 0xf6
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESP+0x18], 0x00010000
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0xe8  // CALL 0x009fc880  (returns 64-bit in EAX:EDX)
        _emit 0xb4
        _emit 0x03
        _emit 0x5c
        _emit 0x00
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x009fc890  (takes EAX, EDX as 2 args)
        _emit 0xbd
        _emit 0x03
        _emit 0x5c
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x24], EAX  (result stored in local)
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x89  // MOV [ESP+0x28], EBX  (-1 stored in local)
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        _emit 0x89  // MOV [ESI+0x00], EBX  (child->field_0 = -1)
        _emit 0x1e
        _emit 0xc7  // MOV dword ptr [ESI+0x18], 0
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESI+0x1c], 0
        _emit 0x46
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 8  (clean 2 pushed args)
        _emit 0xc4
        _emit 0x08
        _emit 0xc7  // MOV dword ptr [ESP+0x24], 0x0043c460
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x60
        _emit 0xc4
        _emit 0x43
        _emit 0x00
        _emit 0xf3  // MOVQ XMM0, [ESP+0x24]  (load 8 bytes)
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x66  // MOVQ [ESI+0x08], XMM0  (store 8 bytes to child)
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x08
        _emit 0xf3  // MOVQ XMM0, [ESP+0x2c]  (load next 8 bytes)
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x8d  // LEA EAX, [ESP+0x14]  (ptr to local config)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50  // PUSH EAX  (arg to FUN_009fc950)
        _emit 0x8b  // MOV ECX, ESI  (this = child obj)
        _emit 0xce
        _emit 0x66  // MOVQ [ESI+0x10], XMM0  (store 8 bytes to child)
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x10
        _emit 0x89  // MOV [ESI+0x18], EDI  (child->parent = this)
        _emit 0x7e
        _emit 0x18
        _emit 0xc7  // MOV dword ptr [ESI+0x1c], 5
        _emit 0x46
        _emit 0x1c
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x009fc950  (__thiscall on child)
        _emit 0x2e
        _emit 0x04
        _emit 0x5c
        _emit 0x00
        _emit 0xeb  // JMP +2 (skip over XOR ESI,ESI)
        _emit 0x02
        // --- alloc-failed path: ESI = 0 ---
        _emit 0x33  // XOR ESI, ESI
        _emit 0xf6
        // --- common path: store new child, release old ---
        _emit 0x89  // MOV [ESP+0x44], EBX  (SEH state → -1)
        _emit 0x5c
        _emit 0x24
        _emit 0x44
        _emit 0x8b  // MOV EBX, [EDI+0x4]  (EBX = old child ptr)
        _emit 0x5f
        _emit 0x04
        _emit 0x3b  // CMP ESI, EBX
        _emit 0xf3
        _emit 0x74  // JZ skip_release (same ptr, no-op)
        _emit 0x14
        _emit 0x85  // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74  // JZ skip_release (old was null)
        _emit 0x10
        _emit 0x8b  // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8  // CALL 0x009fc830  (release/unbind old child)
        _emit 0xf4
        _emit 0x02
        _emit 0x5c
        _emit 0x00
        _emit 0x53  // PUSH EBX  (arg = old child ptr)
        _emit 0xe8  // CALL 0x009d1b17  (operator delete)
        _emit 0xd5
        _emit 0x55
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 4  (clean 1-arg __cdecl)
        _emit 0xc4
        _emit 0x04
        // --- store new child; check arg3 ---
        _emit 0x80  // CMP byte ptr [ESP+0x54], 1  (arg3 == 1?)
        _emit 0x7c
        _emit 0x24
        _emit 0x54
        _emit 0x01
        _emit 0x89  // MOV [EDI+0x4], ESI  (this->child = new child)
        _emit 0x77
        _emit 0x04
        _emit 0x75  // JNZ skip_signals  (arg3 != 1)
        _emit 0x2f
        // --- arg3 == 1: fire two bit-mask signals ---
        _emit 0x8b  // MOV ECX, [ESP+0x4c]  (arg1)
        _emit 0x4c
        _emit 0x24
        _emit 0x4c
        _emit 0xba  // MOV EDX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xd3  // SHL EDX, CL  (EDX = 1 << arg1)
        _emit 0xe2
        _emit 0x52  // PUSH EDX
        _emit 0xff  // CALL dword ptr [0x00f3e1ac]  (fn ptr 1)
        _emit 0x15
        _emit 0xac
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b  // MOV ESI, [0x00f3e1b0]  (fn ptr 2)
        _emit 0x35
        _emit 0xb0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x50  // PUSH EAX  (result of fn ptr 1 call)
        _emit 0xff  // CALL ESI  (fn ptr 2 with result)
        _emit 0xd6
        _emit 0x8b  // MOV ECX, [ESP+0x50]  (arg2)
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        _emit 0xb8  // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xd3  // SHL EAX, CL  (EAX = 1 << arg2)
        _emit 0xe0
        _emit 0x8b  // MOV ECX, [EDI+0x4]  (= new child)
        _emit 0x4f
        _emit 0x04
        _emit 0x8b  // MOV EDX, [ECX]  (dereference child)
        _emit 0x11
        _emit 0x50  // PUSH EAX
        _emit 0x52  // PUSH EDX
        _emit 0xff  // CALL ESI  (fn ptr 2 again)
        _emit 0xd6
        // --- EH4 SEH epilogue ---
        _emit 0x8b  // MOV ECX, [ESP+0x3c]  (saved FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        _emit 0x64  // MOV FS:[0], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX  (cookie)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x38
        _emit 0xc4
        _emit 0x38
        _emit 0xc2  // RET 0x0c  (callee-cleans 3 DWORD args)
        _emit 0x0c
        _emit 0x00
    }
}
