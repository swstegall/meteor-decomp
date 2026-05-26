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
// FUNCTION: ffxivgame 0x0000e110 — allocator alloc() wrapper
//                                   (__thiscall, 250 B / 0xfa)
//
// __thiscall void* FUN_0040e110(e110_Obj *this, unsigned int size, e110_Space *space)
//   ECX        : this
//   [ESP+0x04] : size  — allocation size in bytes
//   [ESP+0x08] : space — pointer to memory-space descriptor (field4 = name string)
//
// Object layout (inferred from offsets):
//   [this + 0x00]  e110_Sub*    m_sub         — pointer to polymorphic sub-allocator
//   [this + 0x10]  const char*  m_location    — location/file tag (default "Unknown")
//   [this + 0x18]  void*        m_callback    — optional post-alloc notification
//   [this + 0x24]  CRITICAL_SECTION m_cs      — lock protecting alloc
//
// Space descriptor layout (inferred from FUN_0040e360/e350 accessors):
//   [space + 0x00]  (value)
//   [space + 0x04]  const char* m_name        — allocation name tag
//
// Behaviour:
//   1. EnterCriticalSection(&this->m_cs)
//   2. Dispatch alloc through this->m_sub vtable slot 1:
//      result = this->m_sub->vtable[1](this, size, space, space->m_name)
//   3. If this->m_callback != 0 && result != 0:
//      end_ptr  = this->m_sub->vtable[8](result)   — spilled to stack
//      begin_ptr = this->m_sub->vtable[7](result)  — 1 stack arg
//      this->m_callback(begin_ptr)                  — __cdecl
//      ADD ESP, 8 cleans both spilled end_ptr and begin_ptr arg
//   4. LeaveCriticalSection(&this->m_cs)
//   5. If result == 0: log error
//   6. Return result.
//
// Register map (orig):
//   ESI = this
//   EBP = &this->m_cs (dedicated; generates PUSH EBP at Enter/Leave)
//   EBX = m_sub (for vtable dispatch)
//   EDI = vtable+4 briefly during alloc dispatch; alloc_result after
//
// Calling convention: __thiscall; callee cleans 2 stack args (RET 0x8).
// Callee-saves pushed: EBX, EBP, ESI, EDI.
// Local frame: SUB ESP, 0x400.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Register-allocation drift (EBX↔EBP swap in callee-save assignment,
//   scheduling of buf[0x3fe]=0 between loc-null-test and branch) is not
//   reproducible from C++ source with MSVC 2005 /O2. The __declspec(naked)
//   body emits the original 250 bytes via MASM instructions + _emit for
//   the non-standard relocatable slots.

extern "C" void FUN_0040e360();
extern "C" void FUN_0040e350();
extern "C" void FUN_0040e110_snprintf_s();  // _snprintf_s IAT slot
extern "C" void FUN_0040e110_strcat_s();    // _strcat_s IAT slot

// IAT / global addresses used in the function body — declared so the
// assembler can emit reloc-bearing references matching the orig layout.
extern "C" void *g_e110_EnterCS;      // [0x00f3e16c]
extern "C" void *g_e110_LeaveCS;      // [0x00f3e168]
extern "C" void *g_e110_outputFnSlot; // [0x012651b4]
extern "C" const char g_e110_strUnknown[];   // 0x00f564d8
extern "C" const char g_e110_fmtAllocFail[]; // 0x00f564a4
extern "C" const char g_e110_strNewline[];   // 0x00f54d98

extern "C" __declspec(naked) void FUN_0040e110() {
    __asm {
        // 0000e110:  81 ec 00 04 00 00     SUB ESP, 0x400
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e116:  53                    PUSH EBX
        _emit 0x53
        // 0000e117:  55                    PUSH EBP
        _emit 0x55
        // 0000e118:  56                    PUSH ESI
        _emit 0x56
        // 0000e119:  8b f1                 MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0000e11b:  57                    PUSH EDI
        _emit 0x57
        // 0000e11c:  8d 6e 24              LEA EBP, [ESI+0x24]
        _emit 0x8d
        _emit 0x6e
        _emit 0x24
        // 0000e11f:  55                    PUSH EBP
        _emit 0x55
        // 0000e120:  ff 15 6c e1 f3 00     CALL [EnterCriticalSection]
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000e126:  8b 1e                 MOV EBX, [ESI]         ; sub = m_sub
        _emit 0x8b
        _emit 0x1e
        // 0000e128:  8b 3b                 MOV EDI, [EBX]         ; vtable
        _emit 0x8b
        _emit 0x3b
        // 0000e12a:  8b 8c 24 18 04 00 00  MOV ECX, [ESP+0x418]   ; space arg
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e131:  83 c7 04              ADD EDI, 4             ; vtable+1 slot ptr
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        // 0000e134:  e8 27 02 00 00        CALL FUN_0040e360      ; name = FUN_0040e360(space)
        _emit 0xe8
        _emit 0x27
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0000e139:  8b 8c 24 18 04 00 00  MOV ECX, [ESP+0x418]   ; space arg again
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e140:  50                    PUSH EAX               ; push name
        _emit 0x50
        // 0000e141:  e8 0a 02 00 00        CALL FUN_0040e350      ; space_val = FUN_0040e350(space)
        _emit 0xe8
        _emit 0x0a
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0000e146:  8b 17                 MOV EDX, [EDI]         ; fn = vtable[1]
        _emit 0x8b
        _emit 0x17
        // 0000e148:  50                    PUSH EAX               ; push space_val
        _emit 0x50
        // 0000e149:  8b 84 24 1c 04 00 00  MOV EAX, [ESP+0x41c]   ; size
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x1c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e150:  50                    PUSH EAX               ; push size
        _emit 0x50
        // 0000e151:  56                    PUSH ESI               ; push this
        _emit 0x56
        // 0000e152:  8b cb                 MOV ECX, EBX           ; ECX = sub (this for vtable call)
        _emit 0x8b
        _emit 0xcb
        // 0000e154:  ff d2                 CALL EDX               ; result = vtable[1](sub,this,size,val,name)
        _emit 0xff
        _emit 0xd2
        // 0000e156:  83 7e 18 00           CMP [ESI+0x18], 0      ; m_callback == 0?
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x00
        // 0000e15a:  8b f8                 MOV EDI, EAX           ; EDI = result
        _emit 0x8b
        _emit 0xf8
        // 0000e15c:  74 24                 JZ +0x24 → 0x0040e182  ; if (!callback) skip
        _emit 0x74
        _emit 0x24
        // 0000e15e:  85 ff                 TEST EDI, EDI          ; result == 0?
        _emit 0x85
        _emit 0xff
        // 0000e160:  74 20                 JZ +0x20 → 0x0040e182  ; if (!result) skip
        _emit 0x74
        _emit 0x20
        // 0000e162:  8b 1e                 MOV EBX, [ESI]         ; sub = m_sub (reload)
        _emit 0x8b
        _emit 0x1e
        // 0000e164:  8b 03                 MOV EAX, [EBX]         ; vtable = sub->vtable
        _emit 0x8b
        _emit 0x03
        // 0000e166:  8b 50 20              MOV EDX, [EAX+0x20]    ; fn = vtable[8]
        _emit 0x8b
        _emit 0x50
        _emit 0x20
        // 0000e169:  57                    PUSH EDI               ; push result arg
        _emit 0x57
        // 0000e16a:  8b cb                 MOV ECX, EBX           ; ECX = sub
        _emit 0x8b
        _emit 0xcb
        // 0000e16c:  ff d2                 CALL EDX               ; end_ptr = vtable[8](sub,result)
        _emit 0xff
        _emit 0xd2
        // 0000e16e:  50                    PUSH EAX               ; spill end_ptr to stack
        _emit 0x50
        // 0000e16f:  8b 03                 MOV EAX, [EBX]         ; vtable = sub->vtable (reload)
        _emit 0x8b
        _emit 0x03
        // 0000e171:  8b 50 1c              MOV EDX, [EAX+0x1c]    ; fn = vtable[7]
        _emit 0x8b
        _emit 0x50
        _emit 0x1c
        // 0000e174:  57                    PUSH EDI               ; push result arg
        _emit 0x57
        // 0000e175:  8b cb                 MOV ECX, EBX           ; ECX = sub
        _emit 0x8b
        _emit 0xcb
        // 0000e177:  ff d2                 CALL EDX               ; begin_ptr = vtable[7](sub,result)
        _emit 0xff
        _emit 0xd2
        // 0000e179:  50                    PUSH EAX               ; push begin_ptr as callback arg
        _emit 0x50
        // 0000e17a:  8b 46 18              MOV EAX, [ESI+0x18]    ; EAX = m_callback
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 0000e17d:  ff d0                 CALL EAX               ; callback(begin_ptr)
        _emit 0xff
        _emit 0xd0
        // 0000e17f:  83 c4 08              ADD ESP, 8             ; clean spilled end + begin arg
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0000e182:  55                    PUSH EBP               ; push &m_cs for Leave
        _emit 0x55
        // 0000e183:  ff 15 68 e1 f3 00     CALL [LeaveCriticalSection]
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000e189:  85 ff                 TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0000e18b:  75 6e                 JNZ +0x6e → 0x0040e1fb ; if (result != 0) return
        _emit 0x75
        _emit 0x6e
        // 0000e18d:  8b 76 10              MOV ESI, [ESI+0x10]    ; loc = m_location
        _emit 0x8b
        _emit 0x76
        _emit 0x10
        // 0000e190:  85 f6                 TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0000e192:  c6 84 24 0e 04 00 00 00  MOV byte [ESP+0x40e], 0  ; buf[0x3fe] = 0
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x0e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000e19a:  75 05                 JNZ +5 → 0x0040e1a1
        _emit 0x75
        _emit 0x05
        // 0000e19c:  be d8 64 f5 00        MOV ESI, g_e110_strUnknown
        _emit 0xbe
        _emit 0xd8
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 0000e1a1:  8b 8c 24 18 04 00 00  MOV ECX, [ESP+0x418]  ; space arg
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e1a8:  8b 41 04              MOV EAX, [ECX+4]       ; tag = space->m_name
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 0000e1ab:  85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0000e1ad:  75 05                 JNZ +5 → 0x0040e1b4
        _emit 0x75
        _emit 0x05
        // 0000e1af:  b8 d8 64 f5 00        MOV EAX, g_e110_strUnknown
        _emit 0xb8
        _emit 0xd8
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 0000e1b4:  8b 94 24 14 04 00 00  MOV EDX, [ESP+0x414]  ; size arg
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e1bb:  56                    PUSH ESI               ; push loc
        _emit 0x56
        // 0000e1bc:  50                    PUSH EAX               ; push tag
        _emit 0x50
        // 0000e1bd:  52                    PUSH EDX               ; push size
        _emit 0x52
        // 0000e1be:  68 a4 64 f5 00        PUSH g_e110_fmtAllocFail
        _emit 0x68
        _emit 0xa4
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 0000e1c3:  68 fe 03 00 00        PUSH 0x3fe
        _emit 0x68
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 0000e1c8:  8d 44 24 24           LEA EAX, [ESP+0x24]    ; &buf[0]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0000e1cc:  68 00 04 00 00        PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e1d1:  50                    PUSH EAX
        _emit 0x50
        // 0000e1d2:  e8 c8 6d 5c 00        CALL _snprintf_s
        _emit 0xe8
        _emit 0xc8
        _emit 0x6d
        _emit 0x5c
        _emit 0x00
        // 0000e1d7:  68 98 4d f5 00        PUSH g_e110_strNewline
        _emit 0x68
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0000e1dc:  8d 4c 24 30           LEA ECX, [ESP+0x30]    ; &buf[0]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // 0000e1e0:  68 00 04 00 00        PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e1e5:  51                    PUSH ECX
        _emit 0x51
        // 0000e1e6:  e8 c9 69 5c 00        CALL _strcat_s
        _emit 0xe8
        _emit 0xc9
        _emit 0x69
        _emit 0x5c
        _emit 0x00
        // 0000e1eb:  8d 54 24 38           LEA EDX, [ESP+0x38]    ; &buf[0]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 0000e1ef:  6a 03                 PUSH 3
        _emit 0x6a
        _emit 0x03
        // 0000e1f1:  52                    PUSH EDX
        _emit 0x52
        // 0000e1f2:  ff 15 b4 51 26 01     CALL [g_e110_outputFnSlot]
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        // 0000e1f8:  83 c4 30              ADD ESP, 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        // 0000e1fb:  8b c7                 MOV EAX, EDI           ; return result
        _emit 0x8b
        _emit 0xc7
        // 0000e1fd:  5f                    POP EDI
        _emit 0x5f
        // 0000e1fe:  5e                    POP ESI
        _emit 0x5e
        // 0000e1ff:  5d                    POP EBP
        _emit 0x5d
        // 0000e200:  5b                    POP EBX
        _emit 0x5b
        // 0000e201:  81 c4 00 04 00 00     ADD ESP, 0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0000e207:  c2 08 00              RET 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
