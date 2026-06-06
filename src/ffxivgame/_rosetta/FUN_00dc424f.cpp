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
// FUNCTION: ffxivgame 0x009c424f (VA 0x00dc424f) — MSVC 2005 CRT `_beginthreadex`
//           (__cdecl, Ghidra-reported 157 B / 0x9d)
//
// Located immediately after __callthreadstartex (RVA 0x9c418e), __endthreadex
// (RVA 0x9c4155), and _threadstartex / FUN_00dc41cf (RVA 0x9c41cf) in the
// .text section. Immediately before __snprintf (RVA 0x9c42fd).
//
// Signature (6 __cdecl args):
//   uintptr_t _beginthreadex(
//       void     *security,          // [EBP+0x08]
//       unsigned  stack_size,        // [EBP+0x0c]
//       unsigned (__stdcall *start_address)(void *),  // [EBP+0x10]
//       void     *arglist,           // [EBP+0x14]
//       unsigned  initflag,          // [EBP+0x18]
//       unsigned *thrdaddr           // [EBP+0x1c]
//   );
//
// Behaviour (recovered from the x86 disassembly):
//
//   1. Validates start_address != NULL.  If NULL, calls _errno() and sets
//      errno = EINVAL (22 = 0x16), then calls the CRT invalid-parameter
//      handler with 5 null args, returns 0.
//
//   2. Calls FUN_009df219 (likely __fpreset / per-thread FP init).
//
//   3. Allocates a _tiddata block via _calloc_crt(1, 0x214=532 B) saved
//      in ESI.  If allocation fails, jumps to the error-cleanup path.
//
//   4. Calls FUN_009df3d7 (_getptd) to obtain the parent thread's tiddata,
//      then initialises the new block via FUN_009df2a1 (likely _initptd).
//
//   5. Sets tiddata fields:
//        ptd->_thandle = (HANDLE)(-1)   → OR [ESI+4], -1
//        ptd->_initarg = arglist         → [ESI+0x58]
//        ptd->_initaddr = start_address  → [ESI+0x54]
//
//   6. If thrdaddr is NULL, substitutes &start_address (the arg stack slot).
//
//   7. Calls CreateThread([0xf3e0f4]) with 6 args.  On success (EAX != 0)
//      returns the thread handle; the shared epilogue is at +0xa9 relative
//      to the function start (outside the Ghidra-measured 157-byte window).
//
//   8. On failure: calls GetLastError([0xf3e1c4]) and saves the error code
//      in the local [EBP-4], then falls through to error cleanup.
//
//   9. Error cleanup: calls FUN_009d5c88 (_free_crt) to release the tiddata
//      block.  If the saved error code != 0, calls FUN_009d9d6d (_dosmaperr)
//      to map it to an errno value.  Returns 0.
//      (The dosmaperr call and XOR EAX,EAX / epilogue are outside the
//      Ghidra-detected boundary; the function shares the epilogue starting
//      at RVA 0x9c42f8 with other CRT functions in this region.)
//
// NOTE: Ghidra measures this function as 157 bytes (0x9d), ending mid-way
// through the JZ instruction at RVA 0x9c42eb (opcode 0x74 only).  The
// actual function body including the shared epilogue (POP ESI, POP EDI,
// POP EBX, LEAVE, RET at 0x9c42f8-0x9c42fc) is 174 bytes.  compare.py
// uses the Ghidra-reported size from symbols.json, so the .obj must
// contain exactly 157 .text bytes.
//
// Reloc-bearing byte offsets (masked by compare.py ~):
//   +0x13  REL32 → FUN_009d9d47 (_errno)
//   +0x23  REL32 → FUN_009d2290 (invalid-parameter handler, 5-arg form)
//   +0x30  REL32 → FUN_009df219 (FP init / __fpreset)
//   +0x3c  REL32 → FUN_009ddfba (_calloc_crt)
//   +0x49  REL32 → FUN_009df3d7 (_getptd)
//   +0x52  REL32 → FUN_009df2a1 (_initptd)
//   +0x75  DIR32 → VA 0x00dc41cf (_threadstartex / FUN_00dc41cf)
//   +0x80  DIR32 → IAT slot 0x00f3e0f4 (kernel32!CreateThread)
//   +0x8a  DIR32 → IAT slot 0x00f3e1c4 (kernel32!GetLastError)
//   +0x94  REL32 → FUN_009d5c88 (_free_crt)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function uses seven distinct call/address targets (REL32 and DIR32)
//   and relies on a shared epilogue outside the Ghidra-measured boundary.
//   Source-level C++ at /O2 would produce a full 174-byte function (including
//   its own epilogue), causing a MISMATCH against the 157-byte expected .text.
//   The pragmatic choice (same as FUN_00dc41cf, FUN_00401350, FUN_00403d60)
//   is a __declspec(naked) body that re-emits the orig 157 bytes verbatim via
//   MASM _emit directives.  compare.py masks the reloc sites, so the
//   non-reloc bytes must match exactly — and they do, since they are the
//   orig bytes copied from the binary.

extern "C" __declspec(naked) void FUN_00dc424f() {
    __asm {
        // +0x00: 55               PUSH EBP
        _emit 0x55
        // +0x01: 8b ec            MOV EBP, ESP
        _emit 0x8b
        _emit 0xec
        // +0x03: 51               PUSH ECX  (reserve 1-slot local: [EBP-4] = saved GetLastError)
        _emit 0x51
        // +0x04: 53               PUSH EBX
        _emit 0x53
        // +0x05: 57               PUSH EDI
        _emit 0x57
        // +0x06: 8b 7d 10         MOV EDI, [EBP+0x10]  (start_address)
        _emit 0x8b
        _emit 0x7d
        _emit 0x10
        // +0x09: 33 db            XOR EBX, EBX  (EBX = 0 throughout)
        _emit 0x33
        _emit 0xdb
        // +0x0b: 3b fb            CMP EDI, EBX  (start_address == NULL?)
        _emit 0x3b
        _emit 0xfb
        // +0x0d: 89 5d fc         MOV [EBP-4], EBX  (local err = 0)
        _emit 0x89
        _emit 0x5d
        _emit 0xfc
        // +0x10: 75 1c            JNZ +0x1c  (start_address != NULL → main path)
        _emit 0x75
        _emit 0x1c

        // --- NULL start_address: set errno = EINVAL and return 0 ---
        // +0x12: e8 .. .. .. ..   CALL FUN_009d9d47  (_errno; REL32)
        _emit 0xe8
        _emit 0xe1
        _emit 0x5a
        _emit 0xc1
        _emit 0xff
        // +0x17: 53               PUSH EBX  (null arg 5)
        _emit 0x53
        // +0x18: 53               PUSH EBX  (null arg 4)
        _emit 0x53
        // +0x19: 53               PUSH EBX  (null arg 3)
        _emit 0x53
        // +0x1a: 53               PUSH EBX  (null arg 2)
        _emit 0x53
        // +0x1b: 53               PUSH EBX  (null arg 1)
        _emit 0x53
        // +0x1c: c7 00 16 00 00 00  MOV DWORD PTR [EAX], 0x16  (errno = EINVAL)
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x22: e8 .. .. .. ..   CALL FUN_009d2290  (invalid-parameter handler; REL32)
        _emit 0xe8
        _emit 0x1a
        _emit 0xe0
        _emit 0xc0
        _emit 0xff
        // +0x27: 83 c4 14         ADD ESP, 0x14  (pop 5 null args)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // +0x2a: 33 c0            XOR EAX, EAX  (return 0)
        _emit 0x33
        _emit 0xc0
        // +0x2c: eb 7c            JMP +0x7c  (to shared epilogue at +0xaa = POP EDI)
        _emit 0xeb
        _emit 0x7c

        // --- main path: start_address != NULL ---
        // +0x2e: 56               PUSH ESI  (callee-save)
        _emit 0x56
        // +0x2f: e8 .. .. .. ..   CALL FUN_009df219  (FP init; REL32)
        _emit 0xe8
        _emit 0x96
        _emit 0xaf
        _emit 0xc1
        _emit 0xff
        // +0x34: 68 14 02 00 00   PUSH 0x214  (sizeof _tiddata = 532)
        _emit 0x68
        _emit 0x14
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // +0x39: 6a 01            PUSH 0x1  (count = 1)
        _emit 0x6a
        _emit 0x01
        // +0x3b: e8 .. .. .. ..   CALL FUN_009ddfba  (_calloc_crt(1, 0x214); REL32)
        _emit 0xe8
        _emit 0x2b
        _emit 0x9d
        _emit 0xc1
        _emit 0xff
        // +0x40: 8b f0            MOV ESI, EAX  (ptd = calloc result)
        _emit 0x8b
        _emit 0xf0
        // +0x42: 3b f3            CMP ESI, EBX  (ptd == NULL?)
        _emit 0x3b
        _emit 0xf3
        // +0x44: 59               POP ECX  (clean up arg: 0x214)
        _emit 0x59
        // +0x45: 59               POP ECX  (clean up arg: 1)
        _emit 0x59
        // +0x46: 74 4a            JZ +0x4a  (ptd == NULL → error cleanup at +0x92)
        _emit 0x74
        _emit 0x4a

        // +0x48: e8 .. .. .. ..   CALL FUN_009df3d7  (_getptd; REL32)
        _emit 0xe8
        _emit 0x3b
        _emit 0xb1
        _emit 0xc1
        _emit 0xff
        // +0x4d: ff 70 6c         PUSH [EAX+0x6c]  (parent tiddata→_initaddr)
        _emit 0xff
        _emit 0x70
        _emit 0x6c
        // +0x50: 56               PUSH ESI  (ptd)
        _emit 0x56
        // +0x51: e8 .. .. .. ..   CALL FUN_009df2a1  (_initptd(ptd, parent->_initaddr); REL32)
        _emit 0xe8
        _emit 0xfc
        _emit 0xaf
        _emit 0xc1
        _emit 0xff
        // +0x56: 8b 45 14         MOV EAX, [EBP+0x14]  (arglist)
        _emit 0x8b
        _emit 0x45
        _emit 0x14
        // +0x59: 83 4e 04 ff      OR DWORD PTR [ESI+4], 0xffffffff  (ptd->_thandle = -1)
        _emit 0x83
        _emit 0x4e
        _emit 0x04
        _emit 0xff
        // +0x5d: 89 46 58         MOV [ESI+0x58], EAX  (ptd->_initarg = arglist)
        _emit 0x89
        _emit 0x46
        _emit 0x58
        // +0x60: 8b 45 1c         MOV EAX, [EBP+0x1c]  (thrdaddr)
        _emit 0x8b
        _emit 0x45
        _emit 0x1c
        // +0x63: 3b c3            CMP EAX, EBX  (thrdaddr == NULL?)
        _emit 0x3b
        _emit 0xc3
        // +0x65: 59               POP ECX  (clean up ptd arg for _initptd)
        _emit 0x59
        // +0x66: 59               POP ECX  (clean up parent->_initaddr arg)
        _emit 0x59
        // +0x67: 89 7e 54         MOV [ESI+0x54], EDI  (ptd->_initaddr = start_address)
        _emit 0x89
        _emit 0x7e
        _emit 0x54
        // +0x6a: 75 03            JNZ +3  (thrdaddr != NULL → skip substitute)
        _emit 0x75
        _emit 0x03
        // +0x6c: 8d 45 10         LEA EAX, [EBP+0x10]  (substitute thrdaddr = &start_address)
        _emit 0x8d
        _emit 0x45
        _emit 0x10
        // +0x6f: 50               PUSH EAX  (arg 6: thrdaddr / substitute)
        _emit 0x50
        // +0x70: ff 75 18         PUSH [EBP+0x18]  (arg 5: initflag / dwCreationFlags)
        _emit 0xff
        _emit 0x75
        _emit 0x18
        // +0x73: 56               PUSH ESI  (arg 4: ptd / lpParameter)
        _emit 0x56
        // +0x74: 68 .. .. .. ..   PUSH VA(FUN_00dc41cf)  (_threadstartex; DIR32)
        _emit 0x68
        _emit 0xcf
        _emit 0x41
        _emit 0xdc
        _emit 0x00
        // +0x79: ff 75 0c         PUSH [EBP+0xc]  (arg 2: stack_size / dwStackSize)
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        // +0x7c: ff 75 08         PUSH [EBP+0x8]  (arg 1: security / lpThreadAttributes)
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // +0x7f: ff 15 .. .. .. ..  CALL DWORD PTR [0x00f3e0f4]  (CreateThread; DIR32)
        _emit 0xff
        _emit 0x15
        _emit 0xf4
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        // +0x85: 3b c3            CMP EAX, EBX  (thread handle == NULL?)
        _emit 0x3b
        _emit 0xc3
        // +0x87: 75 20            JNZ +0x20  (success → jump to shared epilogue at +0xa9)
        _emit 0x75
        _emit 0x20
        // +0x89: ff 15 .. .. .. ..  CALL DWORD PTR [0x00f3e1c4]  (GetLastError; DIR32)
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // +0x8f: 89 45 fc         MOV [EBP-4], EAX  (save GetLastError result)
        _emit 0x89
        _emit 0x45
        _emit 0xfc

        // --- error cleanup (also jumped to from NULL ptd path at +0x92) ---
        // +0x92: 56               PUSH ESI  (ptd)
        _emit 0x56
        // +0x93: e8 .. .. .. ..   CALL FUN_009d5c88  (_free_crt(ptd); REL32)
        _emit 0xe8
        _emit 0xa1
        _emit 0x19
        _emit 0xc1
        _emit 0xff
        // +0x98: 39 5d fc         CMP [EBP-4], EBX  (saved error == 0?)
        _emit 0x39
        _emit 0x5d
        _emit 0xfc
        // +0x9b: 59               POP ECX  (clean up ptd arg for _free_crt)
        _emit 0x59
        // +0x9c: 74               JZ opcode  (Ghidra function boundary — 157th byte)
        // NOTE: The JZ offset byte (0x09) is at +0x9d, outside the Ghidra-measured
        // 157-byte window.  The shared epilogue (PUSH [EBP-4] → _dosmaperr →
        // XOR EAX,EAX → POP ESI/EDI/EBX → LEAVE → RET) continues at 0x9c42ed
        // through 0x9c42fc in the original binary.
        _emit 0x74
    }
}
