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
// Helpers internal to Component::Install::InstallUnpacker. Each is a
// non-virtual member function called from the slot-2 unpack loop
// (FUN_00cc6700, see docs/install-unpacker.md). Match attempts here.

extern "C" __declspec(dllimport) long __stdcall InterlockedExchange(long *target, long value);
extern "C" __declspec(dllimport) int __stdcall SwitchToThread();

// FUNCTION: ffxivgame 0x00cc6620 — wait-for-resource-ready spin (71 B)
//
// Recovered from Ghidra GUI 2026-05-02:
//   void FUN_00cc6620(int param_1) {
//     InterlockedExchange((LONG *)(param_1 + 4), 1);
//     cVar1 = FUN_008edbf0(&param_1);
//     while (cVar1 == '\0') {
//       SwitchToThread();
//       cVar1 = FUN_008edbf0(&param_1);
//     }
//     return;
//   }
//
// In the binary this is `__thiscall` (callee gets `this` in ECX) — Ghidra
// elided that. The function arg is the caller's "resource handle"
// (an int passed as a value, but the same address `&param_1` is forwarded
// to the predicate).
//
// FUN_008edbf0 (the predicate at RVA 0x004edbf0) is invoked on the parent's
// `this + 0x38` member with `&param_1` as its single arg. It returns
// `char` — non-zero meaning "resource ready, exit loop".
//
// The spin pattern:
//   1. Atomically write 1 to (arg+4) — likely "I'm waiting on this"
//      handshake bit so producers can wake us.
//   2. First-try the predicate.
//   3. On failure, loop calling SwitchToThread() and re-check.
//
// Original bytes (71 B):
//   008c6620: 8b 44 24 04 56 6a 01 83 c0 04 50 8b f1 ff 15 ?? ?? ?? ?? 8d 4c 24 08 83 c6 38 51 8b ce e8 ?? ?? ?? ?? 84 c0 75 1d 57 8b 3d ?? ?? ?? ?? 8d 49 00 ff d7 8d 54 24 0c 52 8b ce e8 ?? ?? ?? ?? 84 c0 74 ee 5f 5e c2 04 00
//
// Decoded:
//   MOV  EAX, [ESP+4]          ; arg1
//   PUSH ESI
//   PUSH 1
//   ADD  EAX, 4
//   PUSH EAX
//   MOV  ESI, ECX              ; ESI = this  (saved across all calls)
//   CALL [InterlockedExchange] ; (arg1+4, 1)
//   ; --- first predicate try ---
//   LEA  ECX, [ESP+8]          ; ECX = &arg1 (back to caller's arg slot)
//   ADD  ESI, 0x38             ; ESI = this + 0x38
//   PUSH ECX                    ; arg
//   MOV  ECX, ESI              ; this for thiscall
//   CALL FUN_008edbf0
//   TEST AL, AL
//   JNZ  exit                   ; if returned true, return
//   ; --- spin loop ---
//   PUSH EDI
//   MOV  EDI, [SwitchToThread]
//   LEA  ECX, [ECX+0]           ; 3-byte NOP for branch alignment
//   loop:
//   CALL EDI                    ; SwitchToThread()
//   LEA  EDX, [ESP+0xc]         ; &arg1 (offset shifted by EDI push)
//   PUSH EDX
//   MOV  ECX, ESI               ; this+0x38
//   CALL FUN_008edbf0
//   TEST AL, AL
//   JZ   loop                   ; loop while predicate returns false
//   POP  EDI
//   exit:
//   POP  ESI
//   RET  4
//
// Iteration #1 attempt: requires modeling the parent class's +0x38
// sub-object. For now use a stand-in class shape with the predicate as
// an external function.

class WaitablePredicate {
public:
    char TryReady(int *p);   // = FUN_008edbf0 @ 0x004edbf0
};

class InstallUnpacker {
public:
    void WaitForReady(int handle);
private:
    char m_pad_00[0x38];
    WaitablePredicate m_resource;   // +0x38
};

void InstallUnpacker::WaitForReady(int handle) {
    InterlockedExchange((long *)(handle + 4), 1);
    if (!m_resource.TryReady(&handle)) {
        do {
            SwitchToThread();
        } while (!m_resource.TryReady(&handle));
    }
}
