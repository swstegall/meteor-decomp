# Imported rich struct layouts — `ffxivgame` (no ctor RVA)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


These classes carry field layouts but no recovered ctor RVA, so they
aren't keyed to a function you'd match directly. Anchored by vtable RVA.
Full machine catalog: `config/ffxivgame.legacy_structs.json`.

---

# `Sqex::Socket::SocketBase` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x1a4 (420 bytes)
- **VTable**: 0x011132dc (RVA 0x00d132dc, 18 vfuncs)
- **RTTI**: `.?AVSocketBase@Socket@Sqex@@`
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/SQEX/Socket/SocketTypes.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `nint` | `VTable` | 0x011132DC (18 vfuncs) |
| `0x0004` | `int` | `SocketHandle` | init -1 (INVALID_SOCKET) |
| `0x0008` | `nint` | `CallbackContext` |  |
| `0x000c` | `nint` | `MutexVTable` | Sqex::Thread::Mutex, vt=0x00F67878 |
| `0x0011` | `byte` | `Initialized` | init 1 |
| `0x002c` | `uint` | `StatBytesSent` |  |
| `0x0030` | `uint` | `StatBytesReceived` |  |
| `0x0034` | `uint` | `StatPacketsSent` |  |
| `0x0038` | `uint` | `StatPacketsReceived` |  |
| `0x003c` | `uint` | `StatRetransmits` |  |
| `0x0040` | `uint` | `StatDropped` |  |
| `0x0044` | `uint` | `StatErrors` |  |
| `0x0048` | `uint` | `StatTimeouts` |  |
| `0x004c` | `uint` | `StatQueueDepth` |  |
| `0x0050` | `uint` | `MaxSegmentSize` | init 0x78 = 120 bytes (RUDP2 MSS) |
| `0x0054` | `uint` | `MaxWindowSize` | init 0x708 = 1800 bytes (RUDP2 window) |
| `0x0058` | `uint` | `SendQueueSize` |  |
| `0x005c` | `uint` | `RecvQueueSize` |  |
| `0x0060` | `uint` | `PendingAcks` |  |
| `0x0064` | `uint` | `RetransmitCount` |  |
| `0x0068` | `uint` | `DuplicateCount` |  |
| `0x006c` | `uint` | `OutOfOrderCount` |  |
| `0x0070` | `uint` | `WindowFull` |  |
| `0x0074` | `uint` | `FlowControl` |  |
| `0x0078` | `uint` | `TimestampA` |  |
| `0x007c` | `uint` | `TimestampB` |  |
| `0x0080` | `uint` | `TimestampC` |  |
| `0x0084` | `uint` | `TimestampD` |  |
| `0x0088` | `uint` | `ConnectionFlags` |  |
| `0x008c` | `uint` | `ConnectionState` | PM: connected/disconnected/handshaking |
| `0x0090` | `nint` | `BufferPtr` |  |
| `0x0094` | `uint` | `SequenceNumber` | init 0 — RUDP2 segment sequence tracking |
| `0x009c` | `nint` | `AllocatorPtr` |  |
| `0x00a0` | `nint` | `PollerRef` |  |
| `0x00b4` | `uint` | `TimerA` |  |
| `0x00b8` | `uint` | `TimerB` |  |
| `0x00bc` | `uint` | `TimerC` |  |
| `0x00c0` | `uint` | `TimerD` |  |
| `0x00c8` | `uint` | `RingBufferHead` |  |
| `0x00cc` | `uint` | `RingBufferTail` |  |
| `0x00d0` | `uint` | `RingBufferCapacity` |  |
| `0x00d4` | `uint` | `RingBufferCount` |  |
| `0x00e0` | `nint` | `SendBuffer` |  |
| `0x00e8` | `nint` | `RecvBuffer` |  |
| `0x00ec` | `uint` | `RecvBufferState` | init 0 — PM: lastPartialSize |
| `0x00f0` | `nint` | `BufferAllocA` |  |
| `0x00f4` | `nint` | `BufferAllocB` |  |
| `0x00f8` | `nint` | `BufferSlot0` |  |
| `0x0120` | `nint` | `BufferSlot1` |  |
| `0x0148` | `nint` | `BufferSlot2` |  |
| `0x0170` | `nint` | `BufferSlot3` |  |
| `0x0180` | `nint` | `EndpointInfo` |  |
| `0x0188` | `nint` | `EndpointAddr` |  |
| `0x018c` | `uint` | `EndpointState` |  |
| `0x0190` | `nint` | `EndpointExtra` |  |
| `0x0194` | `nint` | `EndpointExtra2` |  |
| `0x0198` | `nint` | `ContextA` |  |
| `0x019c` | `nint` | `ContextB` |  |
| `0x01a0` | `uint` | `ContextFlags` |  |

---

# `Application::Scene::Actor::System::GameManagerActor::PostEffectController::PostFilterBase` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x80 (128 bytes)
- **VTable**: 0x00fb7c04 (RVA 0x00bb7c04, 1 vfuncs)
- **RTTI**: `.?AVPostFilterBase@PostEffectController@GameManagerActor@System@Actor@Scene@Application@@`
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/FFXIV/Application/Scene/Actor/System/GameManagerActor.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `nint` | `VTable` | 0x00FB7C04 (1 vfunc) |
| `0x0004` | `uint` | `ControllerPtr` | ecx — back-pointer to PostEffectController |
| `0x000c` | `uint` | `ParamA` | ecx |
| `0x0010` | `uint` | `ParamB` | ecx |
| `0x0015` | `byte` | `Enabled` | init 0x00 |
| `0x0018` | `uint` | `ParamC` | ecx |
| `0x0024` | `uint` | `FilterDataA` | edx |
| `0x0028` | `uint` | `FilterDataB` | edx |
| `0x002c` | `ushort` | `FilterDataC` | dx |
| `0x002e` | `ushort` | `FilterDataD` | dx |
| `0x0030` | `uint` | `FilterDataE` | ecx |
| `0x0034` | `uint` | `FilterDataF` | ecx |
| `0x0074` | `float` | `Intensity` | xmm0 init |
| `0x0078` | `uint` | `StateA` | eax |
| `0x007c` | `uint` | `StateB` | eax |

---

# `Application::Misc::RaptureTextureManager` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x2bf (703 bytes)
- **VTable**: 0x00fb6d94 (RVA 0x00bb6d94, 2 vfuncs)
- **RTTI**: `.?AVRaptureTextureManager@Misc@Application@@`
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/FFXIV/Application/Scene/Actor/Chara/CharaVisual.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `nint` | `VTable` | 0x00FB6D94 (2 vfuncs) |
| `0x0004` | `nint` | `OwnerRef` |  |
| `0x0008` | `nint` | `TextureArrayPtr` |  |
| `0x000c` | `nint` | `SecondaryVTable` | 0x00FB6E10 |
| `0x0030` | `uint` | `NullMarker` | ASCII "null" = 0x6E756C6C |
| `0x0083` | `byte` | `TextureLoadState` |  |
| `0x0154` | `float` | `TextureScaleU` |  |
| `0x0158` | `float` | `TextureScaleV` |  |
| `0x015c` | `float` | `TextureOffsetU` |  |
| `0x0210` | `uint` | `TextureFlags` | init 1 |
| `0x0240` | `byte` | `TextureReady` | init 1 |

---

# `Application::Scene::Actor::Chara::CharaCutVisualCtrl` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x50 (80 bytes)
- **VTable**: 0x0104447c (RVA 0x00c4447c, 1 vfuncs)
- **RTTI**: `.?AVCharaCutVisualCtrl@Chara@Actor@Scene@Application@@`
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/FFXIV/Application/Scene/Actor/Chara/CharaVisual.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `nint` | `VTable` | 0x0104447C (1 vfunc) |
| `0x0004` | `nint` | `OwnerActorPtr` |  |
| `0x0008` | `nint` | `QueryHandle_VTable` | QueryHandle (vt=0x00FBC4F8) |
| `0x000c` | `uint` | `State` |  |
| `0x0038` | `uint` | `CutVisualState` |  |
| `0x003c` | `nint` | `CutVisualDataPtr` |  |
| `0x0040` | `uint` | `CutVisualFlags` |  |

---

# `FFXIV::Application::Network::BasePacketHeader` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x10 (16 bytes)
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/FFXIV/Application/Network/PacketOpcodes.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `byte` | `IsAuthenticated` | 1 = Blowfish-encrypted, 0 = plaintext |
| `0x0001` | `byte` | `IsCompressed` | 1 = zlib compressed, 0 = raw |
| `0x0002` | `ushort` | `ConnectionType` | see ConnectionType enum |
| `0x0004` | `ushort` | `PacketSize` | total bytes including this header |
| `0x0006` | `ushort` | `NumSubPackets` |  |
| `0x0008` | `ulong` | `Timestamp` | milliseconds since epoch (UTC) |

---

# `FFXIV::Application::Network::SubPacketHeader` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x10 (16 bytes)
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/FFXIV/Application/Network/PacketOpcodes.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `ushort` | `SubPacketSize` | total bytes including this header |
| `0x0002` | `ushort` | `Type` | see SubPacketType enum |
| `0x0004` | `uint` | `SourceId` | actor/character ID (sender) |
| `0x0008` | `uint` | `TargetId` | session ID (recipient) |
| `0x000c` | `uint` | `Unknown0C` | always 0x00 |

---

# `FFXIV::Application::Network::GameMessageHeader` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x10 (16 bytes)
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/FFXIV/Application/Network/PacketOpcodes.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `ushort` | `Unknown00` | always 0x14 |
| `0x0002` | `ushort` | `Opcode` | see ZoneOpcode / LobbyOpcode enums |
| `0x0004` | `uint` | `Unknown04` | always 0x00 |
| `0x0008` | `uint` | `Timestamp` | unix timestamp (seconds) |
| `0x000c` | `uint` | `Unknown0C` | always 0x00 |

---

# `STD::StdMapNode` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x10 (16 bytes)
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/STD/StdMap.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `StdMapNode<TKey, TValue>*` | `Left` |  |
| `0x0004` | `StdMapNode<TKey, TValue>*` | `Parent` |  |
| `0x0008` | `StdMapNode<TKey, TValue>*` | `Right` |  |
| `0x000c` | `byte` | `Color` | 0=red, 1=black |
| `0x000d` | `byte` | `IsNil` |  |

---

# `STD::StdString` — object layout (imported)

> **Imported from FFXIVLegacyClientStructs** (github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI
> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is
> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a
> strong hint while matching; confirm offsets against the function you're on.


- **Size**: 0x1c (28 bytes)
- **Source**: `FFXIVLegacyClientStructs/FFXIVClientStructs/STD/StdString.cs`

## Fields

| Offset | Type | Name | Note |
|--------|------|------|------|
| `0x0000` | `fixed byte` | `Buffer[16]` |  |
| `0x0000` | `byte*` | `HeapPtr` |  |
| `0x0010` | `uint` | `Length` |  |
| `0x0014` | `uint` | `Capacity` |  |
