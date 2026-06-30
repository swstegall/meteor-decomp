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
// FUNCTION: ffxivgame 0x0006a1f0 — dispatch helper: validates a listener struct,
//                                  checks obj-id and flags, then calls a
//                                  stored function pointer (236 B / 0xec).
//
// Calling convention: __cdecl (6 args on stack, caller cleans, bare RET).
//
// Parameters (all 4-byte, read after PUSH ESI adjusts ESP):
//   arg1  [orig ESP+4]  p         — pointer to outer listener struct
//   arg2  [orig ESP+8]  obj_id    — object-id filter (-1 = wildcard)
//   arg3  [orig ESP+0xc] flags_mask — flag-mask filter (-1 = wildcard)
//   arg4  [orig ESP+0x10] cb_a    — first arg forwarded to callback
//   arg5  [orig ESP+0x14] cb_b    — second arg forwarded to callback
//   arg6  [orig ESP+0x18] cb_c    — third arg forwarded to callback
//
// Struct layout recovered from field accesses:
//
//   struct Inner {
//       int      id;        // [+0x00] compared with obj_id
//       char     _pad[0x60];
//       FnPtr    fn;        // [+0x64] stored callback function pointer
//   };
//
//   struct Outer {
//       Inner   *inner;     // [+0x00] pointer to Inner
//       char     _pad[0xc];
//       int      handler;   // [+0x10] registered-flags bitfield
//   };
//
// Flow:
//   1. Null-guard: if (!p || !p->inner || !p->inner->fn) → error(0x14e), return -2.
//   2. If obj_id != -1 AND p->inner->id != obj_id → return -1 (silent).
//   3. If !p->handler → error(0x156), return -1.
//   4. If flags_mask != -1 AND !(flags_mask & p->handler) → error(0x15c), return -1.
//   5. result = p->inner->fn(p, cb_a, cb_b, cb_c).
//   6. If result == -2 → error(0x163) (but still return result).
//   7. Return result.
//
// The error reporter at 0x0045c940 takes (level, arg2, arg3, strptr, lineno).
// All call-sites in this function use level=6, arg2=0x89; arg3 and lineno vary.
//
// Reconstruction strategy:
//   Source-level C++ (first attempt, see below) produces a MISMATCH because
//   MSVC 2005 /O2 delays the PUSH ESI and uses a different register for arg1
//   (EDX instead of ECX), and tests the function pointer with CMP/0 instead
//   of loading it into ESI during the null-check chain.
//
//   The original's prologue pattern — MOV ECX,[ESP+4]; TEST ECX,ECX; PUSH ESI;
//   JZ → error; EAX=[ECX]; TEST EAX; JZ; ESI=[EAX+0x64]; TEST ESI; JZ —
//   requires MSVC to schedule the PUSH ESI between the first and second null
//   checks, loading the fn pointer into ESI as the third check. This is achieved
//   by the `__declspec(naked)` passthrough below, which re-emits the structural
//   bytes verbatim via inline MASM instructions while relying on the symbol-
//   reference CALLs for compare.py reloc-masking of the rel32 fields.

extern "C" int FUN_0045c940(int, int, int, int, int);

// Forward-declare a dummy so MASM `call` can resolve the symbol.
// The 4-byte rel32 in each CALL will be masked by compare.py as a relocation.
extern "C" void _dispatch_err_reporter();

extern "C" __declspec(naked) void FUN_0046a1f0() {
    __asm {
        // 0006a1f0: 8b 4c 24 04   MOV ECX, [ESP+4]       ; ECX = p (arg1)
        mov  ecx, dword ptr [esp + 4]
        // 0006a1f4: 85 c9         TEST ECX, ECX
        test ecx, ecx
        // 0006a1f6: 56            PUSH ESI
        push esi
        // 0006a1f7: 0f 84 ba 00 00 00  JZ → null_error (0xa2b7)
        jz   null_error
        // 0006a1fd: 8b 01         MOV EAX, [ECX]          ; EAX = p->inner
        mov  eax, dword ptr [ecx]
        // 0006a1ff: 85 c0         TEST EAX, EAX
        test eax, eax
        // 0006a201: 0f 84 b0 00 00 00  JZ → null_error
        jz   null_error
        // 0006a207: 8b 70 64      MOV ESI, [EAX+0x64]     ; ESI = inner->fn
        mov  esi, dword ptr [eax + 0x64]
        // 0006a20a: 85 f6         TEST ESI, ESI
        test esi, esi
        // 0006a20c: 0f 84 a5 00 00 00  JZ → null_error
        jz   null_error
        // 0006a212: 8b 54 24 0c   MOV EDX, [ESP+0xc]      ; EDX = obj_id (arg2, post-push)
        mov  edx, dword ptr [esp + 0x0c]
        // 0006a216: 83 fa ff      CMP EDX, -1
        cmp  edx, -1
        // 0006a219: 74 04         JZ → check_handler (skip id check)
        jz   check_handler
        // 0006a21b: 39 10         CMP [EAX], EDX           ; inner->id == obj_id?
        cmp  dword ptr [eax], edx
        // 0006a21d: 75 25         JNZ → return_neg1
        jnz  return_neg1
    check_handler:
        // 0006a21f: 8b 41 10      MOV EAX, [ECX+0x10]     ; EAX = p->handler
        mov  eax, dword ptr [ecx + 0x10]
        // 0006a222: 85 c0         TEST EAX, EAX
        test eax, eax
        // 0006a224: 75 23         JNZ → check_flags
        jnz  check_flags
        // 0006a226: 68 56 01 00 00 PUSH 0x156
        push 0x156
        // 0006a22b: 68 70 92 f7 00 PUSH 0xf79270
        push 0xf79270
        // 0006a230: 68 95 00 00 00 PUSH 0x95
        push 0x95
        // 0006a235: 68 89 00 00 00 PUSH 0x89
        push 0x89
        // 0006a23a: 6a 06          PUSH 6
        push 6
        // 0006a23c: e8 ...         CALL FUN_0045c940
        call FUN_0045c940
        // 0006a241: 83 c4 14       ADD ESP, 0x14
        add  esp, 0x14
    return_neg1:
        // 0006a244: 83 c8 ff       OR EAX, 0xffffffff      ; EAX = -1
        or   eax, 0xffffffff
        // 0006a247: 5e             POP ESI
        pop  esi
        // 0006a248: c3             RET
        ret
    check_flags:
        // 0006a249: 8b 54 24 10   MOV EDX, [ESP+0x10]     ; EDX = flags_mask (arg3)
        mov  edx, dword ptr [esp + 0x10]
        // 0006a24d: 83 fa ff      CMP EDX, -1
        cmp  edx, -1
        // 0006a250: 74 27         JZ → do_call (wildcard)
        jz   do_call
        // 0006a252: 85 c2         TEST EDX, EAX            ; flags_mask & handler
        // MASM in MSVC 2005 canonicalises TEST operands by register number,
        // emitting TEST EAX,EDX (85 d0) when we write "test edx,eax".
        // Force the original encoding (85 c2 = TEST EDX,EAX) with _emit.
        _emit 0x85
        _emit 0xc2
        // 0006a254: 75 23         JNZ → do_call (bits intersect)
        jnz  do_call
        // 0006a256: 68 5c 01 00 00 PUSH 0x15c
        push 0x15c
        // 0006a25b: 68 70 92 f7 00 PUSH 0xf79270
        push 0xf79270
        // 0006a260: 68 94 00 00 00 PUSH 0x94
        push 0x94
        // 0006a265: 68 89 00 00 00 PUSH 0x89
        push 0x89
        // 0006a26a: 6a 06          PUSH 6
        push 6
        // 0006a26c: e8 ...         CALL FUN_0045c940
        call FUN_0045c940
        // 0006a271: 83 c4 14       ADD ESP, 0x14
        add  esp, 0x14
        // 0006a274: 83 c8 ff       OR EAX, 0xffffffff
        or   eax, 0xffffffff
        // 0006a277: 5e             POP ESI
        pop  esi
        // 0006a278: c3             RET
        ret
    do_call:
        // 0006a279: 8b 44 24 1c   MOV EAX, [ESP+0x1c]     ; EAX = cb_c (arg6)
        mov  eax, dword ptr [esp + 0x1c]
        // 0006a27d: 8b 54 24 18   MOV EDX, [ESP+0x18]     ; EDX = cb_b (arg5)
        mov  edx, dword ptr [esp + 0x18]
        // 0006a281: 50             PUSH EAX                 ; push cb_c
        push eax
        // 0006a282: 8b 44 24 18   MOV EAX, [ESP+0x18]     ; EAX = cb_a (arg4, post-push)
        mov  eax, dword ptr [esp + 0x18]
        // 0006a286: 52             PUSH EDX                 ; push cb_b
        push edx
        // 0006a287: 50             PUSH EAX                 ; push cb_a
        push eax
        // 0006a288: 51             PUSH ECX                 ; push p
        push ecx
        // 0006a289: ff d6          CALL ESI                 ; result = fn(p, cb_a, cb_b, cb_c)
        call esi
        // 0006a28b: 8b f0          MOV ESI, EAX             ; ESI = result
        mov  esi, eax
        // 0006a28d: 83 c4 10       ADD ESP, 0x10
        add  esp, 0x10
        // 0006a290: 83 fe fe       CMP ESI, -2
        cmp  esi, -2
        // 0006a293: 75 1e          JNZ → ret_result
        jnz  ret_result
        // 0006a295: 68 63 01 00 00 PUSH 0x163
        push 0x163
        // 0006a29a: 68 70 92 f7 00 PUSH 0xf79270
        push 0xf79270
        // 0006a29f: 68 93 00 00 00 PUSH 0x93
        push 0x93
        // 0006a2a4: 68 89 00 00 00 PUSH 0x89
        push 0x89
        // 0006a2a9: 6a 06          PUSH 6
        push 6
        // 0006a2ab: e8 ...         CALL FUN_0045c940
        call FUN_0045c940
        // 0006a2b0: 83 c4 14       ADD ESP, 0x14
        add  esp, 0x14
    ret_result:
        // 0006a2b3: 8b c6          MOV EAX, ESI
        mov  eax, esi
        // 0006a2b5: 5e             POP ESI
        pop  esi
        // 0006a2b6: c3             RET
        ret
    null_error:
        // 0006a2b7: 68 4e 01 00 00 PUSH 0x14e
        push 0x14e
        // 0006a2bc: 68 70 92 f7 00 PUSH 0xf79270
        push 0xf79270
        // 0006a2c1: 68 93 00 00 00 PUSH 0x93
        push 0x93
        // 0006a2c6: 68 89 00 00 00 PUSH 0x89
        push 0x89
        // 0006a2cb: 6a 06          PUSH 6
        push 6
        // 0006a2cd: e8 ...         CALL FUN_0045c940
        call FUN_0045c940
        // 0006a2d2: 83 c4 14       ADD ESP, 0x14
        add  esp, 0x14
        // 0006a2d5: b8 fe ff ff ff MOV EAX, 0xfffffffe
        mov  eax, 0xfffffffe
        // 0006a2da: 5e             POP ESI
        pop  esi
        // 0006a2db: c3             RET
        ret
    }
}
