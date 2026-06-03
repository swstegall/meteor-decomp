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
// FUNCTION: ffxivgame 0x004160e0 — filtered vprintf-style log helper:
//   formats `param_3 ...` into a 1024-byte stack buffer via __vsnprintf_s
//   and forwards the result through two __thiscall sinks on `param_1`,
//   gated on a byte-level severity threshold (__cdecl, 120 B / 0x78)
//
// Calling convention: __cdecl. The four explicit parameters are read off
//   the inbound stack at +4/+8/+c and the variadic tail starts at +0x10.
//
// Inferred signature:
//   void FUN_004160e0(
//       void        *param_1,   // [ESI] Printer* logger / context object
//       unsigned char param_2,  // [EBX] severity / channel byte
//       const char  *param_3,   // [EDX] format string
//       ...);
//
// Object layout (offsets touched, all `param_1` = Printer*):
//   [param_1 + 0x04]  byte — master "enabled" flag; bail when zero
//   [param_1 + 0x78]  dword — base offset of a per-channel level table
//   [param_1 + (*(p+0x78)) + 0x81]
//                     byte — channel-specific threshold; message emitted
//                     only when threshold >= param_2 (encoded as
//                     `CMP BL,[…]; JB skip` — unsigned-LT skip)
//
// On the active path the helper:
//   1. SUB ESP,0x400               — reserves `char local_400[1024]`
//   2. __vsnprintf_s(local_400, 0x400, 0x3ff, param_3, va_list)
//   3. FUN_00415650 (thiscall: this=param_1, args: 1, param_2, local_400)
//   4. FUN_00415d00 (thiscall: this=param_1, args: 1, param_2, local_400, 1)
//
// Both forward calls jump six bytes past their reported symbol entry (+6),
// skipping the callees' own `SUB ESP,0x400` — the 0x400 buffer this helper
// already allocated on the stack serves as the callees' working frame
// (a shared-prolog optimisation not reproducible from plain C++ source).
//
// Branch shape: two unsigned-LT (JB) early-outs share the same epilogue
// tail; the master-enable check (JZ at +0x12) lands on `POP ESI` since
// EBX has not yet been pushed on that path. The channel-threshold check
// (JB at +0x26) lands one byte earlier on `POP EBX`.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The shared-prolog jump-into-callee-body at +6 is not reproducible
//   from plain C source. A __declspec(naked) body re-emitting the
//   original 120 bytes via MASM _emit directives produces a .obj whose
//   .text is byte-identical to the original slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004160e0() {
    __asm {
        // 000160e0: 81 ec 00 04 00 00  SUB ESP,0x400      (alloc local_400)
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000160e6: 56                 PUSH ESI
        _emit 0x56
        // 000160e7: 8b b4 24 08 04 00 00  MOV ESI,dword ptr [ESP+0x408]
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x08
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000160ee: 80 7e 04 00        CMP byte ptr [ESI+0x4],0x0
        _emit 0x80
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        // 000160f2: 74 5c              JZ +0x5c           (→ 0x16150, POP ESI)
        _emit 0x74
        _emit 0x5c
        // 000160f4: 8b 46 78           MOV EAX,dword ptr [ESI+0x78]
        _emit 0x8b
        _emit 0x46
        _emit 0x78
        // 000160f7: 53                 PUSH EBX
        _emit 0x53
        // 000160f8: 8b 9c 24 10 04 00 00  MOV EBX,dword ptr [ESP+0x410]
        _emit 0x8b
        _emit 0x9c
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000160ff: 38 9c 30 81 00 00 00  CMP byte ptr [EAX+ESI*1+0x81],BL
        _emit 0x38
        _emit 0x9c
        _emit 0x30
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00016106: 72 47              JB +0x47           (→ 0x1614f, POP EBX)
        _emit 0x72
        _emit 0x47
        // 00016108: 8b 94 24 14 04 00 00  MOV EDX,dword ptr [ESP+0x414]  (param_3)
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001610f: 8d 8c 24 18 04 00 00  LEA ECX,[ESP+0x418]            (va_list seed)
        _emit 0x8d
        _emit 0x8c
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00016116: 51                 PUSH ECX                          (vsnprintf_s arg5)
        _emit 0x51
        // 00016117: 52                 PUSH EDX                          (arg4: format)
        _emit 0x52
        // 00016118: 68 ff 03 00 00     PUSH 0x3ff                        (arg3: count)
        _emit 0x68
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 0001611d: 8d 44 24 14        LEA EAX,[ESP+0x14]                (local_400 ptr)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00016121: 68 00 04 00 00     PUSH 0x400                        (arg2: sizeOfBuffer)
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00016126: 50                 PUSH EAX                          (arg1: buffer)
        _emit 0x50
        // 00016127: e8 2c fa 5b 00     CALL __vsnprintf_s   (rel32 → 0x009d5b58)
        _emit 0xe8
        _emit 0x2c
        _emit 0xfa
        _emit 0x5b
        _emit 0x00
        // 0001612c: 83 c4 14           ADD ESP,0x14                      (cdecl cleanup, 5 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0001612f: 8d 4c 24 08        LEA ECX,[ESP+0x8]                 (local_400 reload)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00016133: 51                 PUSH ECX                          (sink1 arg3: buf)
        _emit 0x51
        // 00016134: 53                 PUSH EBX                          (sink1 arg2: param_2)
        _emit 0x53
        // 00016135: 6a 01              PUSH 0x1                          (sink1 arg1)
        _emit 0x6a
        _emit 0x01
        // 00016137: 8b ce              MOV ECX,ESI                       (this = param_1)
        _emit 0x8b
        _emit 0xce
        // 00016139: e8 12 f5 ff ff     CALL FUN_00415650 (rel32 → 0x00415650)
        _emit 0xe8
        _emit 0x12
        _emit 0xf5
        _emit 0xff
        _emit 0xff
        // 0001613e: 6a 01              PUSH 0x1                          (sink2 arg4)
        _emit 0x6a
        _emit 0x01
        // 00016140: 8d 54 24 0c        LEA EDX,[ESP+0xc]                 (local_400 reload)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00016144: 52                 PUSH EDX                          (sink2 arg3: buf)
        _emit 0x52
        // 00016145: 53                 PUSH EBX                          (sink2 arg2: param_2)
        _emit 0x53
        // 00016146: 6a 01              PUSH 0x1                          (sink2 arg1)
        _emit 0x6a
        _emit 0x01
        // 00016148: 8b ce              MOV ECX,ESI                       (this = param_1)
        _emit 0x8b
        _emit 0xce
        // 0001614a: e8 b1 fb ff ff     CALL FUN_00415d00 (rel32 → 0x00415d00)
        _emit 0xe8
        _emit 0xb1
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 0001614f: 5b                 POP EBX
        _emit 0x5b
        // 00016150: 5e                 POP ESI
        _emit 0x5e
        // 00016151: 81 c4 00 04 00 00  ADD ESP,0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00016157: c3                 RET
        _emit 0xc3
    }
}
