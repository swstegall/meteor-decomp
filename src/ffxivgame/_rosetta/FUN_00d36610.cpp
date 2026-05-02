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
// FUNCTION: ffxivgame 0x00936610 — network-singleton shutdown / WSACleanup
//
// First Win32-touching match candidate. The whole point of this one is
// to *prove* the PSDK install is wired into cl.exe end-to-end: the
// source #includes <winsock2.h> and calls WSACleanup directly, so any
// failure to resolve the WSAStartup-style declarations would surface
// at compile time. (See `tools/setup-msvc.sh` and the PSDK install at
// `vstudio2005-workspace/install-psdk.sh`.)
//
// Picked from the auto-survey of small (≤200 byte) functions whose
// asm body contains a `CALL` to a known Win32 import thunk. WSACleanup
// (kernel-import via `JMP [iat]` thunk at RVA 0x00d42d80) was the
// shortest socket-touching caller in the band.
//
// Behavior (read from the asm @ 0x00936610, 63 bytes):
//   Decrement a refcount global at 0x01378664. If it drops to zero:
//     1. Load object pointer from 0x01378660 into ECX, call method
//        at 0x00d3d670 (a __thiscall — likely shutdown / leave-room
//        on the network singleton).
//     2. Re-read the object pointer (the call may have mutated it).
//        If non-null, call __thiscall method at 0x00d3d6c0 (likely
//        the destructor) then operator delete at 0x009d1b17.
//     3. Clear the global pointer to null.
//     4. Call WSACleanup() — winsock teardown.
//   Return 0 unconditionally.
//
// Calling convention: `__cdecl` (no `RET N` — caller cleans up; in fact
// the function takes no args at all, so it's effectively void→int).
//
// Callee-save: PUSH ESI / POP ESI bracket the cleanup block. ESI is
// used as a live cache of the object pointer across the destructor
// call (which clobbers ECX), then pushed as the operator-delete arg.

// PSDK validation — `<winsock2.h>` resolves end-to-end (the original
// iteration #1 of this file compiled cleanly with the include). For the
// match itself we redeclare `WSACleanup` non-dllimport below, because
// the winsock2.h prototype carries `__declspec(dllimport)` which forces
// cl.exe to emit `CALL [iat]` (ff 15, 6 bytes) — the orig uses a direct
// `CALL near` (e8, 5 bytes) into the JMP-thunk for WSACleanup, which is
// what a non-dllimport declaration produces. The smoke compile of a
// program that genuinely #includes winsock2.h has already proven the
// header resolution path; replicating that here just moves bytes around.
extern "C" int __stdcall WSACleanup(void);

// External globals (in `.data` at fixed VAs in the binary). cl.exe
// emits relocatable references; we never link, so the addresses don't
// need to match — only the access patterns do.
// External methods called by the body. Wrap the singleton in a real
// class with non-static member functions so cl.exe uses true
// __thiscall (ECX = `this`, no shadow params). Single-arg __fastcall
// is byte-equivalent in a vacuum but sometimes loses the load-into-ECX
// register choice in the optimizer's allocator pass; __thiscall is
// the cleanest expression of "ECX is `this`."
class LobbyNetSingleton {
public:
    void shutdown();
    void destroy();
};
extern "C" void __cdecl lobby_net_singleton_free(void *p);

extern "C" int g_lobby_net_refcount;
extern LobbyNetSingleton *g_lobby_net_object;     // C++-mangled symbol (no extern "C") since the type is a class pointer

// Iteration history (most recent first; pre-MSVC-2005-RTM, /O2):
//
// #4 [PARTIAL — structurally equivalent, 7-byte register-choice variance]
//    Final form: class-wrapped LobbyNetSingleton with non-static member
//    methods (genuine __thiscall), C++ if-init form for the cleanup
//    block, non-dllimport WSACleanup decl (forces direct CALL near).
//    SUB+JNZ refcount pattern matches orig. CALL near to method1, free,
//    and WSACleanup all match. Direct-call WSACleanup confirmed
//    correct.
//
//    Stable remaining diff (7 bytes in second cleanup block):
//      orig: MOV ECX, [g_object]   (load → ECX)
//            TEST ECX, ECX
//            MOV ESI, ECX           (cache to ESI BEFORE JZ)
//            JZ skip
//            CALL method2          (ECX still set)
//            PUSH ESI               (cached)
//      ours: MOV ESI, [g_object]   (load → ESI direct)
//            TEST ESI, ESI
//            JZ skip                (no pre-cache MOV)
//            MOV ECX, ESI           (move ESI → ECX inside if)
//            CALL method2
//            PUSH ESI
//
//    Both encodings are 16 bytes for this section, functionally
//    identical (same calls with same args), and produce the same
//    machine state. cl.exe's allocator picks ESI-direct for our
//    source pattern; the orig was emitted with ECX-load + cache. Tried
//    multiple source variations (local-then-test, init-in-condition,
//    direct-global-access without local) — cl.exe consistently
//    chooses ESI-direct under our flags. May be sensitive to
//    surrounding code that's not present in this isolated TU; in a
//    full-binary rebuild the allocator state could yield the orig's
//    choice instead.
//
// #3 — class wrapper for true __thiscall. Same byte count as #2.
// #2 — single-arg __fastcall (vs 2-arg in #1) eliminated XOR EDX, EDX
//      from method calls. Non-dllimport WSACleanup decl saves 1 byte.
// #1 — initial port; included <winsock2.h> directly + 2-arg fastcall.
//      71 bytes vs orig 63 (or 66 actual). PSDK validation: include
//      resolved cleanly; cl.exe + winsock2.h emits CALL [iat] (ff 15)
//      for WSACleanup because of __declspec(dllimport) on the
//      prototype.

extern "C" int rosetta_FUN_00d36610(void) {
    if (--g_lobby_net_refcount == 0) {
        g_lobby_net_object->shutdown();

        // Init-in-condition form: declares + tests in one expression.
        // Sometimes flips cl.exe's allocator choice between
        // "load-to-ESI-direct" (the local-decl-then-test form) and
        // "load-to-ECX-cache-to-ESI" (the orig's pattern).
        if (LobbyNetSingleton *p = g_lobby_net_object) {
            p->destroy();
            lobby_net_singleton_free(p);
        }
        g_lobby_net_object = 0;

        WSACleanup();
    }
    return 0;
}

// vim: ts=4 sts=4 sw=4 et
