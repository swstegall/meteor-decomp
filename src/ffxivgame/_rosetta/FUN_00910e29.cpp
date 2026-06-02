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
// FUNCTION: ffxivgame 0x00910e29 — drain-and-destruct loop over an
//                                  embedded pointer-vector
//                                  (159 bytes / 0x9f)
//
// Calling convention: non-standard — EBP holds a context/object pointer
//   set by the caller; the function uses it as a general-purpose base
//   register. Returns bool (AL=1) via non-thiscall epilogue:
//     POP EBP (restore caller's EBP) / ADD ESP,8 (clean 2 args) / RET.
//
// Behaviour:
//   while (true) {
//       void *obj  = *(EBP + 0x1c);          // context object
//       void **vec = (void**)obj + 0x20e8/4;  // embedded vector struct
//       void  *begin = vec[1];               // [vec+0x4]
//       if (begin == NULL) break;            // empty
//       ptrdiff_t n = (vec[2] - begin) >> 2; // [vec+0x8] - begin
//       if (n == 0) break;                   // no elements
//
//       void **end_ptr = (void**)vec[2];      // end = one past last
//       assert(begin <= end_ptr);            // _invalid_parameter_noinfo
//       void **last   = end_ptr - 1;         // pointer to last element
//       assert(last < end_ptr);             // same assert
//       assert(last >= (void**)begin);       // same assert
//
//       void *elem = *last;                  // load last element
//
//       // memmove(last, last+1, count*sizeof(ptr)) — count typically 0
//       void *cap_end = *(obj + 0x20f0);     // re-read end pointer
//       ptrdiff_t count = (cap_end - (last+1)) >> 2;
//       if (count > 0)
//           memmove_s(last, count*4, last+1, count*4);
//
//       vec[2] -= 4;                         // shrink vector by 1
//       (*(void(**)(void*))(*elem + 0x2c))(elem); // vtable[0x2c/4]()
//       // loop
//   }
//   return true;
//
// Object layout (obj = *(EBP+0x1c)):
//   obj + 0x20e8   embedded vector (MSVC std::vector-like triple):
//     + 0x00  (unused first word by this function)
//     + 0x04  begin pointer     ([ESI+4])
//     + 0x08  end pointer       ([ESI+8] = also obj+0x20f0)
//   obj + 0x20f0 = obj+0x20e8+8 = end pointer (same field)
//
// CALL targets:
//   CALL 0x009d22b4 (3×) — __invalid_parameter_noinfo (bounds-check panic)
//   CALL 0x009d186e (1×) — _memmove_s
//
// Stack:
//   At entry (before PUSH EBX/ESI/EDI):
//     [ESP+0]  return address
//     [ESP+4]  arg0
//     [ESP+8]  arg1   (← ADD ESP,8 cleans these in epilogue)
//   EBP        caller-set context pointer (NOT this function's frame ptr)
//
//   [ESP+0x14] = arg1 position, used mid-loop to snapshot original EDI
//   (the stored value is not consumed after the snapshot)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   EBP is used without a PUSH EBP prologue — the push is in the calling
//   frame (EBP is a callee-saved general-purpose register here, set up
//   by the outer call chain). The corresponding POP EBP + ADD ESP,8
//   cleans both EBP and the two caller args in one shot. This asymmetry
//   cannot be expressed in standard C++. The 4-byte alignment NOP
//   (LEA ESP,[ESP+0]) before the loop header and the specific register
//   allocation across three CALL sites also preclude a source-level match.
//   Per the project convention for ffxivgame, use a __declspec(naked) body
//   re-emitting all 159 bytes verbatim.

extern "C" __declspec(naked) void FUN_00910e29() {
    __asm {
        // 00510e29: 53                PUSH EBX
        _emit 0x53
        // 00510e2a: 56                PUSH ESI
        _emit 0x56
        // 00510e2b: 57                PUSH EDI
        _emit 0x57
        // 00510e2c: 8d 64 24 00       LEA ESP,[ESP+0]  (4-byte alignment NOP)
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // === loop top (0x00910e30) ===
        // 00510e30: 8b 85 1c 00 00 00 MOV EAX,[EBP+0x1c]
        _emit 0x8b
        _emit 0x85
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00510e36: 8d b0 e8 20 00 00 LEA ESI,[EAX+0x20e8]
        _emit 0x8d
        _emit 0xb0
        _emit 0xe8
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // 00510e3c: 8b 46 04          MOV EAX,[ESI+4]  (begin ptr)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00510e3f: 85 c0             TEST EAX,EAX     (null check)
        _emit 0x85
        _emit 0xc0
        // 00510e41: 74 7b             JZ 0x00910ebe    (begin==NULL → exit)
        _emit 0x74
        _emit 0x7b
        // 00510e43: 8b 4e 08          MOV ECX,[ESI+8]  (end ptr)
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 00510e46: 2b c8             SUB ECX,EAX      (end - begin)
        _emit 0x2b
        _emit 0xc8
        // 00510e48: c1 f9 02          SAR ECX,2        (>> 2)
        _emit 0xc1
        _emit 0xf9
        _emit 0x02
        // 00510e4b: 74 71             JZ 0x00910ebe    (count==0 → exit)
        _emit 0x74
        _emit 0x71
        // 00510e4d: 8b 7e 08          MOV EDI,[ESI+8]  (EDI = end ptr)
        _emit 0x8b
        _emit 0x7e
        _emit 0x08
        // 00510e50: 3b c7             CMP EAX,EDI      (begin vs end)
        _emit 0x3b
        _emit 0xc7
        // 00510e52: 76 05             JBE +5           (begin<=end → ok)
        _emit 0x76
        _emit 0x05
        // 00510e54: e8 5b 14 0c 00    CALL 0x009d22b4  (_invalid_parameter_noinfo)
        _emit 0xe8
        _emit 0x5b
        _emit 0x14
        _emit 0x0c
        _emit 0x00
        // 00510e59: 8d 47 fc          LEA EAX,[EDI-4]  (EAX = end-4 = last element)
        _emit 0x8d
        _emit 0x47
        _emit 0xfc
        // 00510e5c: 3b 46 08          CMP EAX,[ESI+8]  (last vs end)
        _emit 0x3b
        _emit 0x46
        _emit 0x08
        // 00510e5f: 89 7c 24 14       MOV [ESP+0x14],EDI  (snapshot original end)
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 00510e63: 77 05             JA +5            (last>end → error)
        _emit 0x77
        _emit 0x05
        // 00510e65: 3b 46 04          CMP EAX,[ESI+4]  (last vs begin)
        _emit 0x3b
        _emit 0x46
        _emit 0x04
        // 00510e68: 73 05             JNC +5           (last>=begin → ok)
        _emit 0x73
        _emit 0x05
        // 00510e6a: e8 45 14 0c 00    CALL 0x009d22b4  (_invalid_parameter_noinfo)
        _emit 0xe8
        _emit 0x45
        _emit 0x14
        _emit 0x0c
        _emit 0x00
        // 00510e6f: 83 c7 fc          ADD EDI,-4       (EDI = last element ptr)
        _emit 0x83
        _emit 0xc7
        _emit 0xfc
        // 00510e72: 3b 7e 08          CMP EDI,[ESI+8]  (last < end?)
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 00510e75: 72 05             JC +5            (last<end → ok)
        _emit 0x72
        _emit 0x05
        // 00510e77: e8 38 14 0c 00    CALL 0x009d22b4  (_invalid_parameter_noinfo)
        _emit 0xe8
        _emit 0x38
        _emit 0x14
        _emit 0x0c
        _emit 0x00
        // 00510e7c: 8b b5 1c 00 00 00 MOV ESI,[EBP+0x1c]  (reload obj ptr)
        _emit 0x8b
        _emit 0xb5
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00510e82: 8b 86 f0 20 00 00 MOV EAX,[ESI+0x20f0]  (end ptr again)
        _emit 0x8b
        _emit 0x86
        _emit 0xf0
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // 00510e88: 8b 1f             MOV EBX,[EDI]    (EBX = *last = element)
        _emit 0x8b
        _emit 0x1f
        // 00510e8a: 81 c6 e8 20 00 00 ADD ESI,0x20e8   (ESI = obj+0x20e8 = vec)
        _emit 0x81
        _emit 0xc6
        _emit 0xe8
        _emit 0x20
        _emit 0x00
        _emit 0x00
        // 00510e90: 8d 4f 04          LEA ECX,[EDI+4]  (ECX = last+1 = src for memmove)
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        // 00510e93: 2b c1             SUB EAX,ECX      (end - (last+1))
        _emit 0x2b
        _emit 0xc1
        // 00510e95: c1 f8 02          SAR EAX,2        (count of ptrs to move)
        _emit 0xc1
        _emit 0xf8
        _emit 0x02
        // 00510e98: 85 c0             TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00510e9a: 7e 10             JLE +0x10        (count<=0 → skip memmove)
        _emit 0x7e
        _emit 0x10
        // 00510e9c: 03 c0             ADD EAX,EAX      (count*2)
        _emit 0x03
        _emit 0xc0
        // 00510e9e: 03 c0             ADD EAX,EAX      (count*4 = byte count)
        _emit 0x03
        _emit 0xc0
        // 00510ea0: 50                PUSH EAX         (arg4: count bytes)
        _emit 0x50
        // 00510ea1: 51                PUSH ECX         (arg3: src = last+1)
        _emit 0x51
        // 00510ea2: 50                PUSH EAX         (arg2: destSize)
        _emit 0x50
        // 00510ea3: 57                PUSH EDI         (arg1: dest = last)
        _emit 0x57
        // 00510ea4: e8 c5 09 0c 00    CALL 0x009d186e  (_memmove_s)
        _emit 0xe8
        _emit 0xc5
        _emit 0x09
        _emit 0x0c
        _emit 0x00
        // 00510ea9: 83 c4 10          ADD ESP,0x10     (clean 4 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // === skip_memmove (0x00910eac) ===
        // 00510eac: 83 46 08 fc       ADD [ESI+8],-4   (pop from vector: end -= 4)
        _emit 0x83
        _emit 0x46
        _emit 0x08
        _emit 0xfc
        // 00510eb0: 8b 13             MOV EDX,[EBX]    (EDX = elem vtable)
        _emit 0x8b
        _emit 0x13
        // 00510eb2: 8b 42 2c          MOV EAX,[EDX+0x2c] (vtable[0x2c/4])
        _emit 0x8b
        _emit 0x42
        _emit 0x2c
        // 00510eb5: 8b cb             MOV ECX,EBX      (ECX = elem)
        _emit 0x8b
        _emit 0xcb
        // 00510eb7: ff d0             CALL EAX         (vtable[11](elem))
        _emit 0xff
        _emit 0xd0
        // 00510eb9: e9 72 ff ff ff    JMP 0x00910e30   (loop back to top)
        _emit 0xe9
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // === epilogue / exit (0x00910ebe) ===
        // 00510ebe: 5f                POP EDI
        _emit 0x5f
        // 00510ebf: 5e                POP ESI
        _emit 0x5e
        // 00510ec0: 5b                POP EBX
        _emit 0x5b
        // 00510ec1: b0 01             MOV AL,1         (return true)
        _emit 0xb0
        _emit 0x01
        // 00510ec3: 5d                POP EBP          (restore caller's EBP)
        _emit 0x5d
        // 00510ec4: 83 c4 08          ADD ESP,8        (clean 2 caller args)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00510ec7: c3                RET
        _emit 0xc3
    }
}
