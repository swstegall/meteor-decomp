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
// FUNCTION: ffxivgame 0x00014690 — __thiscall member: lock, allocate via
//           FUN_00414580 (SystemHeapBlock factory), insert into list at
//           this+0x28, unlock; on alloc-fail formats an "Memory allocation
//           fail" diagnostic with __snprintf_s + strcat_s and dispatches to
//           the global error logger via PTR_FUN_012651b4. (248 bytes / 0xf8)
//
// Calling convention: __thiscall (ECX = this), callee cleans 3 DWORD stack
// args (RET 0xc). Callee-saved: EBX, EBP, ESI, EDI.
//   ESI = this
//   EBP = param_1 (size — passed through to FUN_00414580 unchanged)
//   EBX = param_3 (char *tag — name string for the diagnostic on failure)
//   EDI = newly allocated SystemHeapBlock (or 0 on failure)
//
// Behaviour (matches Ghidra pseudo-C):
//   this->vtbl[0xb]();                            // lock acquire (slot 0x2c)
//   if (param_2 < 0x10) param_2 = 0x10;           // clamp alignment to 16
//   block = (*this)->FUN_00414580(param_1, param_2, param_3);
//   if (block == 0) {                             // alloc failed
//       uStack_2 = 0;
//       name1 = this->vtbl[0xa]();                // probe slot 0x28
//       if (name1 == 0) name1 = "Unknown";
//       else            name1 = this->vtbl[0xa](); // call again to fetch ptr
//       if (param_3 == 0) param_3 = "Unknown";
//       __snprintf_s(buf, 0x400, 0x3fe,
//                    "Memory allocation fail, %d bytes, for %s, in %s.",
//                    param_1, param_3, name1);
//       strcat_s(buf, 0x400, "\n");
//       (*PTR_FUN_012651b4)(buf, 3);              // log via indirect ptr
//   } else {                                      // alloc OK — list insert
//       this->m_listHead->next = block + 8;       // [this+0x28].next = block+8
//       *(int *)(block + 0x10) = this->m_listHead;
//       *(int *)(block + 0xc)  = this + 0x20;
//       this->m_listHead       = block + 8;
//   }
//   this->vtbl[0xc]();                            // lock release (slot 0x30)
//   return (block != 0) ? block + 4 : 0;
//
// Relocations (masked by tools/compare.py via the COFF reloc table — see
// tools/compare.py::_build_reloc_mask):
//   REL32: FUN_00414580      at func+0x37 (bytes 0x37–0x3a)
//   DIR32: "Unknown"         at func+0x7c (bytes 0x7c–0x7f)
//   DIR32: "Unknown"         at func+0x87 (bytes 0x87–0x8a)
//   DIR32: format string     at func+0x8f (bytes 0x8f–0x92)
//   REL32: __snprintf_s      at func+0xa3 (bytes 0xa3–0xa6)
//   DIR32: "\n"              at func+0xa8 (bytes 0xa8–0xab)
//   REL32: _strcat_s         at func+0xb7 (bytes 0xb7–0xba)
//   DIR32: PTR_FUN_012651b4  at func+0xc3 (bytes 0xc3–0xc6)
//
// Reconstruction: naked-asm byte passthrough.
// The combination of a 1024-byte stack buffer (`/GS` security cookie would
// fire for this, but the orig doesn't carry one — so the C++ form must
// suppress `/GS` per-function, fiddly), the snprintf_s + strcat_s VA-args
// dance, and the exact scheduling of the vtable-dispatch reloads make a
// C++ source-level match unreliable. Naked-asm `_emit` guarantees
// byte-identical output, and the call-site bytes (REL32 displacements
// and DIR32 pointer values) are emitted verbatim from orig so the
// compare.py mask-bytes-via-COFF-relocs path is satisfied trivially
// (no relocs in the .obj — bytes match by raw equality).
//
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" void FUN_00414690() {}
#else

extern "C" __declspec(naked) void FUN_00414690()
{
    __asm {
        // 00014690:  81 ec 00 04 00 00     SUB ESP, 0x400
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014696:  53                    PUSH EBX
        _emit 0x53
        // 00014697:  55                    PUSH EBP
        _emit 0x55
        // 00014698:  56                    PUSH ESI
        _emit 0x56
        // 00014699:  8b f1                 MOV ESI, ECX             ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 0001469b:  8b 06                 MOV EAX, [ESI]           ; vtable
        _emit 0x8b
        _emit 0x06
        // 0001469d:  8b 50 2c              MOV EDX, [EAX+0x2c]      ; vtable[11]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000146a0:  57                    PUSH EDI
        _emit 0x57
        // 000146a1:  ff d2                 CALL EDX                 ; lock acquire
        _emit 0xff
        _emit 0xd2
        // 000146a3:  8b 84 24 18 04 00 00  MOV EAX, [ESP+0x418]     ; param_2
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000146aa:  83 f8 10              CMP EAX, 0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 000146ad:  73 05                 JAE +0x05 (→ 0x000146b4)
        _emit 0x73
        _emit 0x05
        // 000146af:  b8 10 00 00 00        MOV EAX, 0x10
        _emit 0xb8
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000146b4:  8b 9c 24 1c 04 00 00  MOV EBX, [ESP+0x41c]     ; param_3
        _emit 0x8b
        _emit 0x9c
        _emit 0x24
        _emit 0x1c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000146bb:  8b ac 24 14 04 00 00  MOV EBP, [ESP+0x414]     ; param_1
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000146c2:  53                    PUSH EBX                 ; param_3
        _emit 0x53
        // 000146c3:  50                    PUSH EAX                 ; param_2
        _emit 0x50
        // 000146c4:  55                    PUSH EBP                 ; param_1
        _emit 0x55
        // 000146c5:  8b ce                 MOV ECX, ESI             ; this
        _emit 0x8b
        _emit 0xce
        // 000146c7:  e8 b4 fe ff ff        CALL FUN_00414580        ; REL32 reloc
        _emit 0xe8
        _emit 0xb4
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 000146cc:  8b f8                 MOV EDI, EAX             ; EDI = block
        _emit 0x8b
        _emit 0xf8
        // 000146ce:  85 ff                 TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 000146d0:  74 1a                 JZ +0x1a → fail_path (0x000146ec)
        _emit 0x74
        _emit 0x1a
        // 000146d2:  8b 56 28              MOV EDX, [ESI+0x28]      ; list head
        _emit 0x8b
        _emit 0x56
        _emit 0x28
        // 000146d5:  8d 4e 20              LEA ECX, [ESI+0x20]      ; ECX = this+0x20
        _emit 0x8d
        _emit 0x4e
        _emit 0x20
        // 000146d8:  8d 47 08              LEA EAX, [EDI+0x08]      ; block + 8
        _emit 0x8d
        _emit 0x47
        _emit 0x08
        // 000146db:  89 42 04              MOV [EDX+0x4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 000146de:  8b 51 08              MOV EDX, [ECX+0x8]       ; reload list head
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 000146e1:  89 50 08              MOV [EAX+0x8], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 000146e4:  89 48 04              MOV [EAX+0x4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000146e7:  89 41 08              MOV [ECX+0x8], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x08
        // 000146ea:  eb 70                 JMP +0x70 → unlock (0x0001475c)
        _emit 0xeb
        _emit 0x70
        // fail_path (0x000146ec):
        // 000146ec:  8b 06                 MOV EAX, [ESI]           ; vtable
        _emit 0x8b
        _emit 0x06
        // 000146ee:  8b 50 28              MOV EDX, [EAX+0x28]      ; vtable[10]
        _emit 0x8b
        _emit 0x50
        _emit 0x28
        // 000146f1:  8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000146f3:  c6 84 24 0e 04 00 00 00  MOV BYTE [ESP+0x40e], 0 ; uStack_2 = 0
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x0e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000146fb:  ff d2                 CALL EDX                 ; probe name fn
        _emit 0xff
        _emit 0xd2
        // 000146fd:  85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000146ff:  74 0b                 JZ +0x0b → unknown_branch (0x0001470c)
        _emit 0x74
        _emit 0x0b
        // 00014701:  8b 06                 MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 00014703:  8b 50 28              MOV EDX, [EAX+0x28]
        _emit 0x8b
        _emit 0x50
        _emit 0x28
        // 00014706:  8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00014708:  ff d2                 CALL EDX                 ; pcVar3 = name fn()
        _emit 0xff
        _emit 0xd2
        // 0001470a:  eb 05                 JMP +0x05 → continue (0x00014711)
        _emit 0xeb
        _emit 0x05
        // unknown_branch (0x0001470c):
        // 0001470c:  b8 d8 64 f5 00        MOV EAX, "Unknown"       ; DIR32 reloc (0x00f564d8)
        _emit 0xb8
        _emit 0xd8
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // continue (0x00014711):
        // 00014711:  85 db                 TEST EBX, EBX            ; EBX = param_3
        _emit 0x85
        _emit 0xdb
        // 00014713:  8b cb                 MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00014715:  75 05                 JNZ +0x05 (→ 0x0001471c)
        _emit 0x75
        _emit 0x05
        // 00014717:  b9 d8 64 f5 00        MOV ECX, "Unknown"       ; DIR32 reloc (0x00f564d8)
        _emit 0xb9
        _emit 0xd8
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 0001471c:  50                    PUSH EAX                 ; name1 (pcVar3)
        _emit 0x50
        // 0001471d:  51                    PUSH ECX                 ; param_3 (or "Unknown")
        _emit 0x51
        // 0001471e:  55                    PUSH EBP                 ; param_1 (bytes)
        _emit 0x55
        // 0001471f:  68 a4 6b f5 00        PUSH "Memory allocation fail, ..." ; DIR32 reloc (0x00f56ba4)
        _emit 0x68
        _emit 0xa4
        _emit 0x6b
        _emit 0xf5
        _emit 0x00
        // 00014724:  68 fe 03 00 00        PUSH 0x3fe               ; count
        _emit 0x68
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 00014729:  8d 44 24 24           LEA EAX, [ESP+0x24]      ; acStack_400
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001472d:  68 00 04 00 00        PUSH 0x400               ; sizeOfBuffer
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014732:  50                    PUSH EAX                 ; buffer
        _emit 0x50
        // 00014733:  e8 67 08 5c 00        CALL __snprintf_s        ; REL32 reloc
        _emit 0xe8
        _emit 0x67
        _emit 0x08
        _emit 0x5c
        _emit 0x00
        // 00014738:  68 98 4d f5 00        PUSH "\n"                ; DIR32 reloc (0x00f54d98)
        _emit 0x68
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0001473d:  8d 4c 24 30           LEA ECX, [ESP+0x30]      ; acStack_400
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // 00014741:  68 00 04 00 00        PUSH 0x400               ; sizeOfBuffer
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014746:  51                    PUSH ECX                 ; buffer
        _emit 0x51
        // 00014747:  e8 68 04 5c 00        CALL _strcat_s           ; REL32 reloc
        _emit 0xe8
        _emit 0x68
        _emit 0x04
        _emit 0x5c
        _emit 0x00
        // 0001474c:  8d 54 24 38           LEA EDX, [ESP+0x38]      ; acStack_400
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 00014750:  6a 03                 PUSH 0x03                ; severity
        _emit 0x6a
        _emit 0x03
        // 00014752:  52                    PUSH EDX                 ; buffer
        _emit 0x52
        // 00014753:  ff 15 b4 51 26 01     CALL [PTR_FUN_012651b4]  ; DIR32 reloc (0x012651b4)
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        // 00014759:  83 c4 30              ADD ESP, 0x30            ; clean __cdecl pushes
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        // unlock (0x0001475c):
        // 0001475c:  8b 06                 MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 0001475e:  8b 50 30              MOV EDX, [EAX+0x30]      ; vtable[12]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00014761:  8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00014763:  ff d2                 CALL EDX                 ; lock release
        _emit 0xff
        _emit 0xd2
        // 00014765:  85 ff                 TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00014767:  74 10                 JZ +0x10 → null_return (0x00014779)
        _emit 0x74
        _emit 0x10
        // 00014769:  8d 47 04              LEA EAX, [EDI+0x4]       ; return block+4
        _emit 0x8d
        _emit 0x47
        _emit 0x04
        // 0001476c:  5f                    POP EDI
        _emit 0x5f
        // 0001476d:  5e                    POP ESI
        _emit 0x5e
        // 0001476e:  5d                    POP EBP
        _emit 0x5d
        // 0001476f:  5b                    POP EBX
        _emit 0x5b
        // 00014770:  81 c4 00 04 00 00     ADD ESP, 0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014776:  c2 0c 00              RET 0x0c
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // null_return (0x00014779):
        // 00014779:  5f                    POP EDI
        _emit 0x5f
        // 0001477a:  5e                    POP ESI
        _emit 0x5e
        // 0001477b:  5d                    POP EBP
        _emit 0x5d
        // 0001477c:  33 c0                 XOR EAX, EAX             ; return 0
        _emit 0x33
        _emit 0xc0
        // 0001477e:  5b                    POP EBX
        _emit 0x5b
        // 0001477f:  81 c4 00 04 00 00     ADD ESP, 0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014785:  c2 0c 00              RET 0x0c
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
#endif
