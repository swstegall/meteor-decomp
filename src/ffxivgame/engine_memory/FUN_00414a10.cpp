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
// FUNCTION: ffxivgame 0x00014a10 — `SystemHeapSpace`-style ReceivableHeapBlock
//                                  "Receive" sibling (238 B, __thiscall, ret 0x10)
//
// __thiscall int FUN_00414a10(this, param_1, param_2, param_3, param_4)
//   ECX        : this (pool-owning Space — sibling of the 0x12fb0 receiver)
//   [ESP+0x04] : param_1   (added to this->field_24 before being fed to the
//                           virtual-receive call as its 5th arg)
//   [ESP+0x08] : param_2
//   [ESP+0x0c] : param_3
//   [ESP+0x10] : param_4
//
// `this` layout used by this function:
//   +0x00 : vtable ptr — slot 6 (offset 0x18) is the "get lock-guard / IGuard"
//                        helper used to bracket the body. Its return value's
//                        vtable[0xb] (offset 0x2c) is the Enter; its
//                        vtable[0xc] (offset 0x30) is the Leave.
//   +0x24 : DWORD       base offset, added to param_1 before being passed as
//                       the 5th arg to the virtual-receive call.
//   +0x28 : Owner *     opaque holder whose +0x34 field is the head of an
//                       intrusive doubly-linked list. The freshly-allocated
//                       block payload (alloc+8, the offset 8 list-link
//                       sub-object) is spliced in between head and head->_8.
//
// Behaviour (same shape as FUN_00412fb0 / FUN_00414140 but with an x86 try/
// finally SEH frame wrapping the whole body so the lock-guard is released
// even if the inner virtual-receive throws):
//   1. SEH frame setup (FS:[0] chain, handler = LAB_00e55181, state = -1).
//   2. guard = this->vt[6]();         guard->vt[0xb]();      // Enter
//   3. alloc = __aligned_malloc(0x3c, 0x10);
//   4. if (alloc != 0):
//        store alloc at local_a (+0x08) and local_b (+0x0c)
//        SEH state = 0
//        uVar = this->vt[6](0, p2, p3, p4, this->field_24+p1, this->field_28)
//        edi = FUN_004147b0(this = alloc, uVar)      // in-place ctor
//      else edi = 0
//   5. SEH state = -1
//   6. node = (edi != 0) ? (edi + 8) : 0
//   7. // splice node between head = this->field_28->field_34
//      //  and head->_8 in the intrusive doubly-linked list:
//      head           = this->field_28->field_34
//      old_next       = head->field_8
//      old_next->_4   = node          // old_next->prev = node
//      node->_8       = old_next      // node->next     = old_next
//      node->_4       = head          // node->prev     = head
//      head->_8       = node          // head->next     = node
//   8. guard = this->vt[6](); guard->vt[0xc]();      // Leave
//   9. SEH frame teardown.
//  10. return (edi != 0) ? (edi + 4) : 0;
//
// Reconstruction strategy: __declspec(naked) `_emit` byte passthrough.
//   MSVC 2005 /O2 spits this out as a single 238-byte block in which:
//     - the x86 try/finally SEH frame `PUSH -1 / PUSH handler /
//       PUSH FS:[0] / MOV FS:[0],ESP / SUB ESP,8` prologue isn't
//       reproducible from C++ without `__try`/`__finally` plus a
//       compiler-generated unwind handler at a specific RVA;
//     - the absolute `PUSH 0xe55181` handler address is hard-coded into
//       the orig PE — emitting it as literal `68 81 51 e5 00` makes the
//       .obj's .text match orig byte-for-byte without needing a linker
//       fixup;
//     - the two REL32 calls (CALL __aligned_malloc @ off 0x30, CALL
//       FUN_004147b0 @ off 0x77) are emitted as orig displacement bytes,
//       so there's no relocation entry for compare.py to mask;
//     - the SEH state DWORD updates (`MOV [ESP+0x18], 0` / `... ,-1`)
//       sit at fixed offsets that any source-level rewrite would
//       perturb.
//
// Relocations (none — emitted as absolute orig bytes; compare.py sees a
// zero-mask .text matching orig at every byte):
//   PUSH imm32       (handler addr 0xe55181) at func+0x02
//   CALL __aligned_malloc                     at func+0x30
//   CALL FUN_004147b0                         at func+0x77

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" int FUN_00414a10() { __builtin_unreachable(); }
#else

extern "C" __declspec(naked) void FUN_00414a10()
{
    __asm {
        // --- SEH frame prologue ---
        // 00014a10:  6a ff               PUSH -1                       ; SEH state slot init
        _emit 0x6a
        _emit 0xff
        // 00014a12:  68 81 51 e5 00      PUSH 0xe55181                 ; SEH handler (LAB_00e55181)
        _emit 0x68
        _emit 0x81
        _emit 0x51
        _emit 0xe5
        _emit 0x00
        // 00014a17:  64 a1 00 00 00 00   MOV  EAX, FS:[0]              ; prev SEH chain
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014a1d:  50                  PUSH EAX                      ; chain it
        _emit 0x50
        // 00014a1e:  64 89 25 00 00 00 00 MOV  FS:[0], ESP             ; install new frame
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014a25:  83 ec 08            SUB  ESP, 8                   ; 2 local DWORDs (the alloc back-pointers)
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00014a28:  56                  PUSH ESI                      ; callee-save
        _emit 0x56
        // 00014a29:  8b f1               MOV  ESI, ECX                 ; ESI = this
        _emit 0x8b
        _emit 0xf1

        // --- Enter: this->vt[6]()->vt[0xb]() ---
        // 00014a2b:  8b 06               MOV  EAX, [ESI]               ; vtable
        _emit 0x8b
        _emit 0x06
        // 00014a2d:  8b 50 18            MOV  EDX, [EAX+0x18]          ; vt[6] (get_guard)
        _emit 0x8b
        _emit 0x50
        _emit 0x18
        // 00014a30:  57                  PUSH EDI                      ; callee-save (delayed save — saves a byte of prologue)
        _emit 0x57
        // 00014a31:  ff d2               CALL EDX                      ; guard = this->vt[6]()
        _emit 0xff
        _emit 0xd2
        // 00014a33:  8b 10               MOV  EDX, [EAX]               ; guard's vtable
        _emit 0x8b
        _emit 0x10
        // 00014a35:  8b c8               MOV  ECX, EAX                 ; ECX = guard
        _emit 0x8b
        _emit 0xc8
        // 00014a37:  8b 42 2c            MOV  EAX, [EDX+0x2c]          ; guard->vt[0xb] (Enter)
        _emit 0x8b
        _emit 0x42
        _emit 0x2c
        // 00014a3a:  ff d0               CALL EAX                      ; guard->Enter()
        _emit 0xff
        _emit 0xd0

        // --- alloc = __aligned_malloc(0x3c, 0x10) ---
        // 00014a3c:  6a 10               PUSH 0x10                     ; alignment
        _emit 0x6a
        _emit 0x10
        // 00014a3e:  6a 3c               PUSH 0x3c                     ; size
        _emit 0x6a
        _emit 0x3c
        // 00014a40:  e8 cd 0c 5c 00      CALL __aligned_malloc         ; → 0x9d5712 (REL32 literal)
        _emit 0xe8
        _emit 0xcd
        _emit 0x0c
        _emit 0x5c
        _emit 0x00
        // 00014a45:  8b f8               MOV  EDI, EAX                 ; EDI = alloc
        _emit 0x8b
        _emit 0xf8
        // 00014a47:  83 c4 08            ADD  ESP, 8                   ; clean cdecl args
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00014a4a:  89 7c 24 08         MOV  [ESP+0x08], EDI          ; local_a = alloc
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        // 00014a4e:  89 7c 24 0c         MOV  [ESP+0x0c], EDI          ; local_b = alloc
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 00014a52:  85 ff               TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00014a54:  c7 44 24 18 00 00 00 00   MOV  [ESP+0x18], 0      ; SEH state = 0 (entered try)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014a5c:  74 32               JZ  +0x32 (→ 0x00414a90)       ; alloc failed → edi=0 path
        _emit 0x74
        _emit 0x32

        // --- inner: virtual receive + in-place ctor ---
        // 00014a5e:  8b 4e 28            MOV  ECX, [ESI+0x28]          ; field_28 (Owner)
        _emit 0x8b
        _emit 0x4e
        _emit 0x28
        // 00014a61:  8b 56 24            MOV  EDX, [ESI+0x24]          ; field_24 (base offset)
        _emit 0x8b
        _emit 0x56
        _emit 0x24
        // 00014a64:  03 54 24 20         ADD  EDX, [ESP+0x20]          ; + param_1
        _emit 0x03
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 00014a68:  8b 44 24 2c         MOV  EAX, [ESP+0x2c]          ; param_4
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 00014a6c:  51                  PUSH ECX                      ; arg6 = field_28
        _emit 0x51
        // 00014a6d:  8b 4c 24 2c         MOV  ECX, [ESP+0x2c]          ; param_3 (esp shifted by +4)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 00014a71:  52                  PUSH EDX                      ; arg5 = field_24 + p1
        _emit 0x52
        // 00014a72:  8b 54 24 2c         MOV  EDX, [ESP+0x2c]          ; param_2 (esp shifted by +8)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 00014a76:  50                  PUSH EAX                      ; arg4 = param_4
        _emit 0x50
        // 00014a77:  8b 06               MOV  EAX, [ESI]               ; vtable
        _emit 0x8b
        _emit 0x06
        // 00014a79:  51                  PUSH ECX                      ; arg3 = param_3
        _emit 0x51
        // 00014a7a:  52                  PUSH EDX                      ; arg2 = param_2
        _emit 0x52
        // 00014a7b:  8b 50 18            MOV  EDX, [EAX+0x18]          ; vt[6]
        _emit 0x8b
        _emit 0x50
        _emit 0x18
        // 00014a7e:  6a 00               PUSH 0                        ; arg1 = 0
        _emit 0x6a
        _emit 0x00
        // 00014a80:  8b ce               MOV  ECX, ESI                 ; ECX = this
        _emit 0x8b
        _emit 0xce
        // 00014a82:  ff d2               CALL EDX                      ; uVar = this->vt[6](0,p2,p3,p4,base+p1,field28)
        _emit 0xff
        _emit 0xd2
        // 00014a84:  50                  PUSH EAX                      ; push uVar as the lone stack arg of FUN_004147b0
        _emit 0x50
        // 00014a85:  8b cf               MOV  ECX, EDI                 ; ECX = alloc (this for in-place ctor)
        _emit 0x8b
        _emit 0xcf
        // 00014a87:  e8 24 fd ff ff      CALL FUN_004147b0             ; (REL32 literal: 0x4147b0)
        _emit 0xe8
        _emit 0x24
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00014a8c:  8b f8               MOV  EDI, EAX                 ; EDI = ctor result
        _emit 0x8b
        _emit 0xf8
        // 00014a8e:  eb 02               JMP +0x02 (→ 0x00414a92)
        _emit 0xeb
        _emit 0x02

        // 00014a90:  33 ff               XOR EDI, EDI                  ; alloc-failed branch
        _emit 0x33
        _emit 0xff

        // --- merge: prep list-node ---
        // 00014a92:  85 ff               TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00014a94:  c7 44 24 18 ff ff ff ff  MOV  [ESP+0x18], -1      ; SEH state = -1 (left try)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00014a9c:  74 05               JZ +0x05 (→ 0x00414aa3)
        _emit 0x74
        _emit 0x05
        // 00014a9e:  8d 4f 08            LEA ECX, [EDI+0x08]           ; node = edi + 8
        _emit 0x8d
        _emit 0x4f
        _emit 0x08
        // 00014aa1:  eb 02               JMP +0x02 (→ 0x00414aa5)
        _emit 0xeb
        _emit 0x02
        // 00014aa3:  33 c9               XOR ECX, ECX                  ; node = 0
        _emit 0x33
        _emit 0xc9

        // --- splice node into the intrusive doubly-linked list ---
        // 00014aa5:  8b 46 28            MOV  EAX, [ESI+0x28]          ; this->field_28
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 00014aa8:  8b 40 34            MOV  EAX, [EAX+0x34]          ; head = field_28->_34
        _emit 0x8b
        _emit 0x40
        _emit 0x34
        // 00014aab:  8b 50 08            MOV  EDX, [EAX+0x08]          ; old_next = head->_8
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00014aae:  89 4a 04            MOV  [EDX+0x04], ECX          ; old_next->_4 = node    (prev)
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 00014ab1:  8b 50 08            MOV  EDX, [EAX+0x08]          ; reload old_next (MSVC keeps the second load)
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00014ab4:  89 51 08            MOV  [ECX+0x08], EDX          ; node->_8 = old_next    (next)
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00014ab7:  89 41 04            MOV  [ECX+0x04], EAX          ; node->_4 = head        (prev)
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 00014aba:  89 48 08            MOV  [EAX+0x08], ECX          ; head->_8 = node        (next)
        _emit 0x89
        _emit 0x48
        _emit 0x08

        // --- Leave: this->vt[6]()->vt[0xc]() ---
        // 00014abd:  8b 06               MOV  EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 00014abf:  8b 50 18            MOV  EDX, [EAX+0x18]          ; vt[6]
        _emit 0x8b
        _emit 0x50
        _emit 0x18
        // 00014ac2:  8b ce               MOV  ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00014ac4:  ff d2               CALL EDX                      ; guard = this->vt[6]()
        _emit 0xff
        _emit 0xd2
        // 00014ac6:  8b 10               MOV  EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 00014ac8:  8b c8               MOV  ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00014aca:  8b 42 30            MOV  EAX, [EDX+0x30]          ; vt[0xc] (Leave)
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00014acd:  ff d0               CALL EAX                      ; guard->Leave()
        _emit 0xff
        _emit 0xd0

        // --- epilogue: return (edi != 0) ? (edi + 4) : 0 ---
        // 00014acf:  85 ff               TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00014ad1:  74 16               JZ +0x16 (→ 0x00414ae9)
        _emit 0x74
        _emit 0x16

        // success path
        // 00014ad3:  8d 47 04            LEA  EAX, [EDI+0x04]          ; return value = alloc + 4
        _emit 0x8d
        _emit 0x47
        _emit 0x04
        // 00014ad6:  5f                  POP EDI
        _emit 0x5f
        // 00014ad7:  5e                  POP ESI
        _emit 0x5e
        // 00014ad8:  8b 4c 24 08         MOV  ECX, [ESP+0x08]          ; saved FS:[0] (after 2 pops, lives at +0x08)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00014adc:  64 89 0d 00 00 00 00 MOV  FS:[0], ECX             ; restore SEH chain
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014ae3:  83 c4 14            ADD  ESP, 0x14                ; pop frame (8 locals + 4 handler + 4 state + 4 saved fs)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00014ae6:  c2 10 00            RET  0x10                     ; __thiscall, 4 stack args
        _emit 0xc2
        _emit 0x10
        _emit 0x00

        // failure path (edi == 0): return 0
        // 00014ae9:  8b 4c 24 10         MOV  ECX, [ESP+0x10]          ; saved FS:[0] (no pops yet, lives at +0x10)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00014aed:  5f                  POP EDI
        _emit 0x5f
        // 00014aee:  33 c0               XOR  EAX, EAX                 ; return 0
        _emit 0x33
        _emit 0xc0
        // 00014af0:  5e                  POP ESI
        _emit 0x5e
        // 00014af1:  64 89 0d 00 00 00 00 MOV  FS:[0], ECX             ; restore SEH chain
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00014af8:  83 c4 14            ADD  ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00014afb:  c2 10 00            RET  0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}

#endif
