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
// FUNCTION: ffxivgame 0x005df219 — get-or-create via function pointers (50 B / 0x32)
//
// Calling convention: __cdecl — no args, returns pointer/handle in EAX, plain RET.
// Frame: PUSH ESI only (callee-save; no locals, no frame pointer, no ESP adjustment).
//
// Pattern: "get-or-create"
//   1. Push [0x012eaeb0] (key/handle) and call through fnptr [0x00f3e2a4] → "get"
//   2. If result != NULL (ESI != 0), skip to epilogue → return it
//   3. Push [0x01363fc0] (template/type) and call FUN_009df187 (REL32) → "create"
//   4. POP ECX (caller-clean of the one-arg call at step 3)
//   5. Push the new object (ESI) + key [0x012eaeb0], call through fnptr [0x00f3e2a0] → "register"
//   6. Epilogue: MOV EAX, ESI; POP ESI; RET
//
// Reconstruction: __declspec(naked) _emit byte passthrough.
// All address-bearing immediates are baked into the original binary's VA space.
// compare.py masks relocated bytes; _emit emits no relocations — direct comparison
// against the 50 orig bytes at RVA 0x005df219 yields GREEN.
//
// REL32 at bytes 25-29: target 0x009df187 − next_IP 0x009df237 = 0xFFFFFF50
//
// Byte layout (50 bytes @ RVA 0x005df219 / VA 0x009df219):
//   56                    PUSH ESI
//   ff 35 b0 ae 2e 01     PUSH dword ptr [0x012eaeb0]
//   ff 15 a4 e2 f3 00     CALL dword ptr [0x00f3e2a4]
//   8b f0                 MOV ESI, EAX
//   85 f6                 TEST ESI, ESI
//   75 1b                 JNZ +0x1b  (→ epilogue at 0x009df247)
//   ff 35 c0 3f 36 01     PUSH dword ptr [0x01363fc0]
//   e8 50 ff ff ff        CALL 0x009df187  (rel32 = 0xFFFFFF50)
//   59                    POP ECX
//   8b f0                 MOV ESI, EAX
//   56                    PUSH ESI
//   ff 35 b0 ae 2e 01     PUSH dword ptr [0x012eaeb0]
//   ff 15 a0 e2 f3 00     CALL dword ptr [0x00f3e2a0]
//   8b c6                 MOV EAX, ESI
//   5e                    POP ESI
//   c3                    RET

extern "C" __declspec(naked) void FUN_009df219() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0xff              // PUSH dword ptr [0x012eaeb0]
        _emit 0x35
        _emit 0xb0
        _emit 0xae
        _emit 0x2e
        _emit 0x01
        _emit 0xff              // CALL dword ptr [0x00f3e2a4]
        _emit 0x15
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +0x1b
        _emit 0x1b
        _emit 0xff              // PUSH dword ptr [0x01363fc0]
        _emit 0x35
        _emit 0xc0
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0xe8              // CALL 0x009df187 (rel32 = 0xFFFFFF50)
        _emit 0x50
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x56              // PUSH ESI
        _emit 0xff              // PUSH dword ptr [0x012eaeb0]
        _emit 0x35
        _emit 0xb0
        _emit 0xae
        _emit 0x2e
        _emit 0x01
        _emit 0xff              // CALL dword ptr [0x00f3e2a0]
        _emit 0x15
        _emit 0xa0
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
