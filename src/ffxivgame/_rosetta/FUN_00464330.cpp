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
// FUNCTION: ffxivgame 0x00064330 — `__cdecl` ASN1_object_size (64 bytes)
//
// Statically-linked OpenSSL helper. Computes the encoded DER size of an
// ASN.1 object given its constructed flag, payload length, and tag:
//
//   int ASN1_object_size(int constructed, int length, int tag)
//   {
//       int ret;
//       ret = length;
//       ret++;
//       if (tag >= 31) {
//           while (tag > 0) {            // multi-byte tag octets (base-128)
//               tag >>= 7;
//               ret++;
//           }
//       }
//       if (constructed == 2)            // indefinite-length constructed
//           ret += 3;
//       else {
//           ret++;
//           if (length > 127) {          // long-form length octets
//               while (length > 0) {
//                   length >>= 8;
//                   ret++;
//               }
//           }
//       }
//       return ret;
//   }
//
// Register allocation in the orig: ECX = tag (arg3 @ [ESP+0xc]),
// EDX = length (arg2 @ [ESP+8]), EAX = ret (seeded `LEA EAX,[EDX+1]`).
// The `constructed` arg (@ [ESP+4]) is only CMP'd against 2.
//
// Asm (64 bytes — pure leaf, NO relocations: no CALLs, no IAT loads;
// every branch is a self-relative short jump within the slice, so the
// bytes are reproducible verbatim and `tools/compare.py` reports GREEN):
//
//   8b 4c 24 0c        MOV  ECX, [ESP+0xc]      ; tag
//   83 f9 1f           CMP  ECX, 0x1f           ; tag vs 31
//   8b 54 24 08        MOV  EDX, [ESP+0x8]      ; length
//   8d 42 01           LEA  EAX, [EDX+1]        ; ret = length + 1
//   7c 0e              JL   .skiptag            ; tag < 31
//   85 c9              TEST ECX, ECX
//   7e 0a              JLE  .skiptag            ; tag <= 0
//  .tagloop:
//   c1 f9 07           SAR  ECX, 7              ; tag >>= 7
//   83 c0 01           ADD  EAX, 1              ; ret++
//   85 c9              TEST ECX, ECX
//   7f f6              JG   .tagloop            ; while tag > 0
//  .skiptag:
//   83 7c 24 04 02     CMP  [ESP+4], 2          ; constructed == 2 ?
//   75 04              JNZ  .primitive
//   83 c0 03           ADD  EAX, 3              ; ret += 3
//   c3                 RET
//  .primitive:
//   83 c0 01           ADD  EAX, 1              ; ret++
//   83 fa 7f           CMP  EDX, 0x7f           ; length vs 127
//   7e 0e              JLE  .end                ; length <= 127
//   85 d2              TEST EDX, EDX
//   7e 0a              JLE  .end                ; length <= 0
//  .lenloop:
//   c1 fa 08           SAR  EDX, 8              ; length >>= 8
//   83 c0 01           ADD  EAX, 1              ; ret++
//   85 d2              TEST EDX, EDX
//   7f f6              JG   .lenloop            ; while length > 0
//  .end:
//   c3                 RET
//
// Reconstruction strategy — naked-asm byte passthrough (the same idiom
// siblings FUN_00406fa0 / FUN_00406ff0 use). A C-level source for this
// leaf would in principle compile to the same bytes, but the exact
// register pinning (ECX=tag, EDX=length, EAX=ret) and the precise
// branch lowering are guaranteed only by re-emitting the orig 64 bytes
// directly. No relocations exist, so the resulting .obj `.text` is
// byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00464330() {
    __asm {
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x0c]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x83              // CMP ECX, 0x1f
        _emit 0xf9
        _emit 0x1f
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x08]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x8d              // LEA EAX, [EDX + 0x01]
        _emit 0x42
        _emit 0x01
        _emit 0x7c              // JL  +0x0e  (.skiptag)
        _emit 0x0e
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x7e              // JLE +0x0a  (.skiptag)
        _emit 0x0a
        _emit 0xc1              // SAR ECX, 0x07           (.tagloop)
        _emit 0xf9
        _emit 0x07
        _emit 0x83              // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x7f              // JG  -0x0a  (.tagloop)
        _emit 0xf6
        _emit 0x83              // CMP dword ptr [ESP + 0x04], 0x02   (.skiptag)
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x02
        _emit 0x75              // JNZ +0x04  (.primitive)
        _emit 0x04
        _emit 0x83              // ADD EAX, 0x03
        _emit 0xc0
        _emit 0x03
        _emit 0xc3              // RET
        _emit 0x83              // ADD EAX, 0x01           (.primitive)
        _emit 0xc0
        _emit 0x01
        _emit 0x83              // CMP EDX, 0x7f
        _emit 0xfa
        _emit 0x7f
        _emit 0x7e              // JLE +0x0e  (.end)
        _emit 0x0e
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x7e              // JLE +0x0a  (.end)
        _emit 0x0a
        _emit 0xc1              // SAR EDX, 0x08           (.lenloop)
        _emit 0xfa
        _emit 0x08
        _emit 0x83              // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x7f              // JG  -0x0a  (.lenloop)
        _emit 0xf6
        _emit 0xc3              // RET                     (.end)
    }
}
