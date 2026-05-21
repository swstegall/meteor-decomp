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
// FUNCTION: ffxivgame 0x00403e07 — tail fragment of a std::basic_string-style
// _Grow / reallocate-and-copy helper. NOT an independently callable function:
// FUN_00403d60 sets up the EBP frame, /GS+SEH cookie, callee-save spill of
// EBX/ESI/EDI, allocates the new buffer (whose pointer it stores back into
// the slot at [EBP+0x8] — repurposing what was the size arg), computes the
// new capacity into ESI, then falls through with `JMP 0x00403e07` at RVA
// 0x00403ddb. The matching Catch_All@0x00403ddd re-enters via SEH dispatch
// with state == 2 after an alloc-time exception in the parent.
//
// Liveness on entry (caller-established):
//
//   EBP        = parent stack frame (already set up)
//   EDI        = this  (the string-like object, layout below)
//   ESI        = new capacity (newRes)
//   [EBP+0x8]  = pointer to freshly allocated buffer (was: caller arg #1)
//   [EBP+0xc]  = count = number of source chars (caller arg #2)
//   [EBP-0xc]  = saved FS:[0] chain pointer (will be restored on exit)
//   EBX/ESI/EDI/ECX already spilled by parent's PUSH chain
//
// Object layout (MSVC 2005 std::basic_string with non-EBO'd allocator
// occupying the 4 bytes at offset 0):
//
//   offset 0x00  _Alval (allocator footprint, 4 B)
//   offset 0x04  _Bx union { char* _Ptr; char _Buf[16]; }   — 16 B
//   offset 0x14  _Mysize (current length)                    — 4 B
//   offset 0x18  _Myres  (current capacity)                  — 4 B
//
// Logical body (with EDI=this, ESI=newRes, [EBP+8]=newBuf, [EBP+c]=count):
//
//   if (count > 0) {
//       char *old = (this->_Myres >= 16) ? this->_Bx._Ptr
//                                        : &this->_Bx._Buf[0];
//       char_traits<char>::_Copy_s(newBuf, newRes + 1, old, count);
//   }
//   if (this->_Myres >= 16) {
//       operator delete(this->_Bx._Ptr);
//   }
//   // Tidy SSO byte (dead-store; immediately overwritten by the ptr
//   // store below — but MSVC emits both writes as part of the
//   // "deactivate SSO, activate heap pointer" union-flip idiom).
//   *(char*)&this->_Bx = '\0';
//   this->_Bx._Ptr     = newBuf;
//   this->_Myres       = newRes;
//   this->_Mysize      = count;
//   // CMP ESI,0x10 at the top of this group sets CF; the JC below uses
//   // that *preserved* CF (only register-to-register MOV and dword-store
//   // touched flags would be unaffected) to pick the buffer to NUL-term.
//   char *term = (newRes < 16) ? newBuf : <heap ptr — never taken when
//                                          we just allocated for >=16>;
//   term[count] = '\0';
//   // tear down SEH chain, restore callee-saves, ret 8 (parent's frame).
//
// External calls (each leaves a 4-byte rel32 reloc that compare.py masks):
//   CALL 0x009d17f3 — char_traits<char>::_Copy_s (memcpy_s-style)
//   CALL 0x009d1b17 — operator delete(void*)     (cdecl, 1 arg)
//
// Function size: config/ffxivgame.yaml gives 0x66 (102 B, ending at
// POP EBP @0x00403e6c), but config/ffxivgame.size_overrides.json
// corrects this to 105 B by re-including the trailing `RET 0x8`
// (c2 08 00) at 0x00403e6d — Ghidra's flow-analysis under-counted the
// 3-byte `ret imm16` opcode because the asm walker stopped before it.
// compare.py honours size_overrides, so the naked __asm below ends
// with `ret 8` and emits all 105 bytes.
//
// Why naked asm: this is structurally not a function (no prolog, lives
// in the middle of a parent's frame, jumped to via fall-through and SEH
// resume). There is no C++ source shape that round-trips back to these
// 102 bytes — every attempt to express it at source level would force
// MSVC to emit a fresh prolog/epilog. Naked __asm lets us write the
// bytes literally and lean on compare.py's reloc-masking to ignore the
// 2 × 4-byte CALL offsets that the linker would fill at final-link time.

extern "C" int FUN_009d17f3();   // _Copy_s (memcpy_s-style copier)
extern "C" int FUN_009d1b17();   // operator delete (cdecl, 1 arg)

extern "C" __declspec(naked) void FUN_00403e07() {
    __asm {
        // ---- @0x00403e07: if (count > 0) copy old → new --------------
        mov     ebx, [ebp + 0x0c]                  // 8b 5d 0c
        test    ebx, ebx                           // 85 db
        jbe     after_copy                         // 76 20  → 0x403e2e
        cmp     dword ptr [edi + 0x18], 0x10       // 83 7f 18 10
        jb      use_sso                            // 72 05  → 0x403e19
        mov     eax, [edi + 0x04]                  // 8b 47 04   (heap ptr)
        jmp     do_copy                            // eb 03  → 0x403e1c
    use_sso:                                       //          @0x403e19
        lea     eax, [edi + 0x04]                  // 8d 47 04   (&_Bx[0])
    do_copy:                                       //          @0x403e1c
        push    ebx                                // 53           count
        push    eax                                // 50           old
        mov     eax, [ebp + 0x08]                  // 8b 45 08     newBuf
        lea     edx, [esi + 0x01]                  // 8d 56 01     newRes+1
        push    edx                                // 52
        push    eax                                // 50
        call    FUN_009d17f3                       // e8 ?? ?? ?? ??  _Copy_s
        add     esp, 0x10                          // 83 c4 10
    after_copy:                                    //          @0x403e2e
        // ---- @0x00403e2e: free old heap buf if it was on the heap ---
        cmp     dword ptr [edi + 0x18], 0x10       // 83 7f 18 10
        jb      after_delete                       // 72 0c  → 0x403e40
        mov     ecx, [edi + 0x04]                  // 8b 4f 04
        push    ecx                                // 51
        call    FUN_009d1b17                       // e8 ?? ?? ?? ??  delete
        add     esp, 0x04                          // 83 c4 04
    after_delete:                                  //          @0x403e40
        // ---- @0x00403e40: install new buf + size/cap, NUL-terminate -
        cmp     esi, 0x10                          // 83 fe 10   sets CF for the JB below
        mov     ecx, [ebp + 0x08]                  // 8b 4d 08
        lea     eax, [edi + 0x04]                  // 8d 47 04   &this->_Bx
        mov     byte ptr [eax], 0                  // c6 00 00   tidy SSO byte
        mov     [eax], ecx                         // 89 08      install newBuf
        mov     [edi + 0x18], esi                  // 89 77 18   _Myres = newRes
        mov     [edi + 0x14], ebx                  // 89 5f 14   _Mysize = count
        jb      finalize_sso                       // 72 02  → 0x403e58
        mov     eax, ecx                           // 8b c1      term = newBuf
    finalize_sso:                                  //          @0x403e58
        mov     byte ptr [eax + ebx * 1], 0        // c6 04 18 00  term[count]='\0'
        // ---- @0x00403e5c: SEH/callee-save teardown ------------------
        mov     ecx, [ebp - 0x0c]                  // 8b 4d f4   saved FS[0]
        mov     dword ptr fs:[0], ecx              // 64 89 0d 00 00 00 00
        pop     ecx                                // 59
        pop     edi                                // 5f
        pop     esi                                // 5e
        pop     ebx                                // 5b
        mov     esp, ebp                           // 8b e5
        pop     ebp                                // 5d
        ret     8                                  // c2 08 00
    }
}
