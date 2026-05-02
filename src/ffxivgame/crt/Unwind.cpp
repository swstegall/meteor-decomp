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
// MSVC 2005 CRT — `__global_unwind2`. Wraps `RtlUnwind` to perform
// the global SEH unwind for the current frame. Recovered 2026-05-02
// from byte signature vs lowhelpr.asm / chandler4gs.c in the CRT
// source.

extern "C" __declspec(dllimport) void __stdcall RtlUnwind(
    void *TargetFrame,
    void *TargetIp,
    void *ExceptionRecord,
    void *ReturnValue);

extern "C" {

// FUNCTION: ffxivgame 0x005f6414 — __global_unwind2 (32 B)
//
//   55              push    ebp
//   8b ec           mov     ebp, esp
//   53              push    ebx
//   56              push    esi
//   57              push    edi
//   55              push    ebp        ; ReturnValue (5th — but RtlUnwind has 4)
//   6a 00           push    0          ; ExceptionRecord = NULL
//   6a 00           push    0          ; ReturnValue = NULL
//   68 ?? ?? ?? ??  push    OFFSET return_point   ; TargetIp = the label below
//   ff 75 08        push    [ebp+8]    ; TargetFrame = arg
//   e8 ?? ?? ?? ??  call    RtlUnwind
// return_point:
//   5d              pop     ebp
//   5f              pop     edi
//   5e              pop     esi
//   5b              pop     ebx
//   8b e5           mov     esp, ebp
//   5d              pop     ebp
//   c3              ret

__declspec(naked) void __cdecl __global_unwind2(void *pRegistFrame) {
    __asm {
        push    ebp
        mov     ebp, esp
        push    ebx
        push    esi
        push    edi
        push    ebp
        push    0
        push    0
        push    OFFSET return_point
        push    dword ptr [ebp+8]
        call    RtlUnwind
    return_point:
        pop     ebp
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret
    }
}

}  // extern "C"
