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
// FUNCTION: ffxivgame 0x0000f3d0 — __thiscall, 1 stack arg
//                                  (SQEX::CDev::Engine::Memory::ComplexLink::CheckConsistency)
//                                  (354 B / 0x162)
//
// __thiscall void FUN_0040f3d0(ComplexLink *this, MemoryBlock *block)
//   ECX        : this  — pointer to the ComplexLink object
//   [ESP+0x04] : block — pointer to a memory block descriptor
//
// Behaviour:
//   Iterates over the free-list of memory blocks anchored in `block`,
//   checking three consistency invariants for each block:
//     1. (line 0x8e) prev pointer of current block matches expected prev
//     2. (line 0x91) next block's back-pointer points to current block
//     3. (line 0x94) prev block's forward size matches current block's position
//   If any invariant is violated, calls the registered assert handler via
//   the one-time-init global function pointer at 0x0132390c (flag at 0x01323910).
//   After the free-list walk, traverses two intrusive linked lists in `this`
//   (forward via +0x8 and +0xc chains).
//
// Reloc-bearing sites (absolute data addresses; not reproducible via
// standalone .obj compilation):
//   0x01323910  — one-time-init flag
//   0x0132390c  — assert handler function pointer
//   0x00f566bc  — "SQEX::CDev::Engine::Memory::ComplexLink::CheckConsistency"
//   0x00f5665c  — ".\\modules\\ComplexLink.cpp"
//   0x00f54d48  — assert expression string
//   0x00f566ac  — "left == right"
//   0x0040f2d0  — FUN_0040f2d0 (assert handler initialiser)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The register allocation across the three-check loop body, the LEA NOP
//   alignment at +0x2a, and the absolute data addresses make this
//   non-reproducible from C++ source with MSVC 2005 /O2.
//   A __declspec(naked) body emits the original 354 bytes verbatim.

extern "C" __declspec(naked) void FUN_0040f3d0()
{
    __asm {
        // 0000f3d0:  53                   PUSH EBX
        _emit 0x53
        // 0000f3d1:  56                   PUSH ESI
        _emit 0x56
        // 0000f3d2:  57                   PUSH EDI
        _emit 0x57
        // 0000f3d3:  8b 7c 24 10          MOV EDI,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0000f3d7:  8b 47 04             MOV EAX,dword ptr [EDI + 0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 0000f3da:  8b 57 08             MOV EDX,dword ptr [EDI + 0x8]
        _emit 0x8b
        _emit 0x57
        _emit 0x08
        // 0000f3dd:  8d 74 38 10          LEA ESI,[EAX + EDI*0x1 + 0x10]
        _emit 0x8d
        _emit 0x74
        _emit 0x38
        _emit 0x10
        // 0000f3e1:  03 d0                ADD EDX,EAX
        _emit 0x03
        _emit 0xd0
        // 0000f3e3:  8d 44 3a f0          LEA EAX,[EDX + EDI*0x1 + -0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x3a
        _emit 0xf0
        // 0000f3e7:  3b f0                CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 0000f3e9:  8b d9                MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 0000f3eb:  8d 4e f0             LEA ECX,[ESI + -0x10]
        _emit 0x8d
        _emit 0x4e
        _emit 0xf0
        // 0000f3ee:  0f 84 19 01 00 00    JZ 0x0040f50d
        _emit 0x0f
        _emit 0x84
        _emit 0x19
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0000f3f4:  55                   PUSH EBP
        _emit 0x55
        // 0000f3f5:  bd d0 f2 40 00       MOV EBP,0x40f2d0
        _emit 0xbd
        _emit 0xd0
        _emit 0xf2
        _emit 0x40
        _emit 0x00
        // 0000f3fa:  8d 9b 00 00 00 00    LEA EBX,[EBX]
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f400:  8b 56 04             MOV EDX,dword ptr [ESI + 0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0000f403:  81 e2 ff ff ff 7f    AND EDX,0x7fffffff
        _emit 0x81
        _emit 0xe2
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 0000f409:  8b c6                MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0000f40b:  2b c2                SUB EAX,EDX
        _emit 0x2b
        _emit 0xc2
        // 0000f40d:  83 e8 10             SUB EAX,0x10
        _emit 0x83
        _emit 0xe8
        _emit 0x10
        // 0000f410:  3b c8                CMP ECX,EAX
        _emit 0x3b
        _emit 0xc8
        // 0000f412:  74 38                JZ 0x0040f44c
        _emit 0x74
        _emit 0x38
        // 0000f414:  f6 05 10 39 32 01 01 TEST byte ptr [0x01323910],0x1
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0000f41b:  75 0d                JNZ 0x0040f42a
        _emit 0x75
        _emit 0x0d
        // 0000f41d:  83 0d 10 39 32 01 01 OR dword ptr [0x01323910],0x1
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0000f424:  89 2d 0c 39 32 01    MOV dword ptr [0x0132390c],EBP
        _emit 0x89
        _emit 0x2d
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000f42a:  68 bc 66 f5 00       PUSH 0xf566bc
        _emit 0x68
        _emit 0xbc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f42f:  68 8e 00 00 00       PUSH 0x8e
        _emit 0x68
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f434:  68 5c 66 f5 00       PUSH 0xf5665c
        _emit 0x68
        _emit 0x5c
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f439:  68 48 4d f5 00       PUSH 0xf54d48
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0000f43e:  68 ac 66 f5 00       PUSH 0xf566ac
        _emit 0x68
        _emit 0xac
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f443:  ff 15 0c 39 32 01    CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000f449:  83 c4 14             ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0000f44c:  8b 0e                MOV ECX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0000f44e:  8b 54 31 14          MOV EDX,dword ptr [ECX + ESI*0x1 + 0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x31
        _emit 0x14
        // 0000f452:  8d 44 31 10          LEA EAX,[ECX + ESI*0x1 + 0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x31
        _emit 0x10
        // 0000f456:  81 e2 ff ff ff 7f    AND EDX,0x7fffffff
        _emit 0x81
        _emit 0xe2
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 0000f45c:  2b c2                SUB EAX,EDX
        _emit 0x2b
        _emit 0xc2
        // 0000f45e:  83 e8 10             SUB EAX,0x10
        _emit 0x83
        _emit 0xe8
        _emit 0x10
        // 0000f461:  3b f0                CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 0000f463:  74 38                JZ 0x0040f49d
        _emit 0x74
        _emit 0x38
        // 0000f465:  f6 05 10 39 32 01 01 TEST byte ptr [0x01323910],0x1
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0000f46c:  75 0d                JNZ 0x0040f47b
        _emit 0x75
        _emit 0x0d
        // 0000f46e:  83 0d 10 39 32 01 01 OR dword ptr [0x01323910],0x1
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0000f475:  89 2d 0c 39 32 01    MOV dword ptr [0x0132390c],EBP
        _emit 0x89
        _emit 0x2d
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000f47b:  68 bc 66 f5 00       PUSH 0xf566bc
        _emit 0x68
        _emit 0xbc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f480:  68 91 00 00 00       PUSH 0x91
        _emit 0x68
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f485:  68 5c 66 f5 00       PUSH 0xf5665c
        _emit 0x68
        _emit 0x5c
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f48a:  68 48 4d f5 00       PUSH 0xf54d48
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0000f48f:  68 ac 66 f5 00       PUSH 0xf566ac
        _emit 0x68
        _emit 0xac
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f494:  ff 15 0c 39 32 01    CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000f49a:  83 c4 14             ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0000f49d:  8b 4e 04             MOV ECX,dword ptr [ESI + 0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 0000f4a0:  81 e1 ff ff ff 7f    AND ECX,0x7fffffff
        _emit 0x81
        _emit 0xe1
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 0000f4a6:  8b c6                MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0000f4a8:  2b c1                SUB EAX,ECX
        _emit 0x2b
        _emit 0xc1
        // 0000f4aa:  8b 50 f0             MOV EDX,dword ptr [EAX + -0x10]
        _emit 0x8b
        _emit 0x50
        _emit 0xf0
        // 0000f4ad:  83 e8 10             SUB EAX,0x10
        _emit 0x83
        _emit 0xe8
        _emit 0x10
        // 0000f4b0:  8d 44 02 10          LEA EAX,[EDX + EAX*0x1 + 0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x02
        _emit 0x10
        // 0000f4b4:  3b f0                CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 0000f4b6:  74 38                JZ 0x0040f4f0
        _emit 0x74
        _emit 0x38
        // 0000f4b8:  f6 05 10 39 32 01 01 TEST byte ptr [0x01323910],0x1
        _emit 0xf6
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0000f4bf:  75 0d                JNZ 0x0040f4ce
        _emit 0x75
        _emit 0x0d
        // 0000f4c1:  83 0d 10 39 32 01 01 OR dword ptr [0x01323910],0x1
        _emit 0x83
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0000f4c8:  89 2d 0c 39 32 01    MOV dword ptr [0x0132390c],EBP
        _emit 0x89
        _emit 0x2d
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000f4ce:  68 bc 66 f5 00       PUSH 0xf566bc
        _emit 0x68
        _emit 0xbc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f4d3:  68 94 00 00 00       PUSH 0x94
        _emit 0x68
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f4d8:  68 5c 66 f5 00       PUSH 0xf5665c
        _emit 0x68
        _emit 0x5c
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f4dd:  68 48 4d f5 00       PUSH 0xf54d48
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0000f4e2:  68 ac 66 f5 00       PUSH 0xf566ac
        _emit 0x68
        _emit 0xac
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f4e7:  ff 15 0c 39 32 01    CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0000f4ed:  83 c4 14             ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0000f4f0:  8b 16                MOV EDX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 0000f4f2:  8b 47 08             MOV EAX,dword ptr [EDI + 0x8]
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // 0000f4f5:  8b ce                MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0000f4f7:  8d 74 16 10          LEA ESI,[ESI + EDX*0x1 + 0x10]
        _emit 0x8d
        _emit 0x74
        _emit 0x16
        _emit 0x10
        // 0000f4fb:  8b 57 04             MOV EDX,dword ptr [EDI + 0x4]
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        // 0000f4fe:  03 c7                ADD EAX,EDI
        _emit 0x03
        _emit 0xc7
        // 0000f500:  8d 44 10 f0          LEA EAX,[EAX + EDX*0x1 + -0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x10
        _emit 0xf0
        // 0000f504:  3b f0                CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 0000f506:  0f 85 f4 fe ff ff    JNZ 0x0040f400
        _emit 0x0f
        _emit 0x85
        _emit 0xf4
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0000f50c:  5d                   POP EBP
        _emit 0x5d
        // 0000f50d:  8b 43 0c             MOV EAX,dword ptr [EBX + 0xc]
        _emit 0x8b
        _emit 0x43
        _emit 0x0c
        // 0000f510:  8d 4b 04             LEA ECX,[EBX + 0x4]
        _emit 0x8d
        _emit 0x4b
        _emit 0x04
        // 0000f513:  3b c1                CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0000f515:  74 07                JZ 0x0040f51e
        _emit 0x74
        _emit 0x07
        // 0000f517:  8b 40 08             MOV EAX,dword ptr [EAX + 0x8]
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 0000f51a:  3b c1                CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0000f51c:  75 f9                JNZ 0x0040f517
        _emit 0x75
        _emit 0xf9
        // 0000f51e:  8b 43 10             MOV EAX,dword ptr [EBX + 0x10]
        _emit 0x8b
        _emit 0x43
        _emit 0x10
        // 0000f521:  3b c1                CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0000f523:  5f                   POP EDI
        _emit 0x5f
        // 0000f524:  5e                   POP ESI
        _emit 0x5e
        // 0000f525:  5b                   POP EBX
        _emit 0x5b
        // 0000f526:  74 07                JZ 0x0040f52f
        _emit 0x74
        _emit 0x07
        // 0000f528:  8b 40 0c             MOV EAX,dword ptr [EAX + 0xc]
        _emit 0x8b
        _emit 0x40
        _emit 0x0c
        // 0000f52b:  3b c1                CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0000f52d:  75 f9                JNZ 0x0040f528
        _emit 0x75
        _emit 0xf9
        // 0000f52f:  c2 04 00             RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
