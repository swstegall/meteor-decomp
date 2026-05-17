# Channel dispatch tree — MSVC STL `std::map` + network class hierarchy

> Recovered 2026-05-17 while continuing Phase 9 #5 Half A.
> Identifies what's at `channel[+8]` (the per-opcode handler tree
> walked by `FUN_004e5ca0`) and surfaces the full network class
> hierarchy that owns it.

## TL;DR

- `channel[+8..+0x18]` is an **MSVC STL `std::map<K,V>`** instance, not
  a custom structure. Walked by `FUN_004e5ca0` using the classic
  `_Tree` traversal pattern.
- Node layout: `[+0]=_Left`, `[+8]=_Right`, `[+0xc]=_Myval` (key —
  likely u32 opcode), `[+0x15]=_Isnil`. Value layout extends from
  `[+0x10]` onward.
- `FUN_004e5ca0` is most plausibly **`std::map::operator[]`** —
  find-or-insert by key. The size check `CMP [EDI+0x8], 0x1FFFFFFE` in
  its helper `FUN_004e4dd0` is the classic STL max-size guard before
  insertion.
- The 3 channel classes are **`LobbyProtoChannel`**, **`ZoneProtoChannel`**,
  **`ChatProtoChannel`** (all under `Application::Network`), each
  nesting a `ServiceConsumerConnectionManager`, `ConsumerConnection`,
  and `ClientPacketBuilder`.

## The std::map walk in FUN_004e5ca0

The traversal pattern in `FUN_004e5ca0`:

```c
char *map_op_index(StdMap *this, char *out, void *key_ptr) {
    Node *cur = this->_Myhead;           // ESI = this[+4]
    Node *parent = cur->_Parent;         // EAX = ESI[+4]
    if (!parent->_Isnil) {                // [EAX+0x15] check
        do {
            uint32_t key = *(uint32_t*)key_ptr;
            if (key < parent->_Myval.first) {   // CMP EDX, [EAX+0xc]
                parent = parent->_Left;          // EAX = [EAX+0]
            } else {
                parent = parent->_Right;         // EAX = [EAX+8]
            }
        } while (!parent->_Isnil);
    }
    if (key_was_less_than_last) {
        // Insert new entry
        FUN_004e4dd0(this, /*flag=*/1, root, key);   // CALL 0x4e4dd0
        // ... copy node fields to out
    } else {
        // Found existing entry
        // ... copy node fields to out
    }
    return out;
}
```

Node field offsets (standard MSVC STL `_Tree_node`):

| Offset | Field | Notes |
|---:|---|---|
| `+0x00` | `_Left` | Child pointer |
| `+0x04` | `_Parent` | Parent pointer |
| `+0x08` | `_Right` | Child pointer |
| `+0x0c` | `_Myval.first` (the key) | u32 — likely opcode |
| `+0x10..+0x14` | `_Myval.second` (value) | 4 bytes — likely fn ptr or registry index |
| `+0x14` | `_Color` | Red/black bit |
| `+0x15` | `_Isnil` | Sentinel flag |

So each map entry is roughly: `(u32 opcode → 4-byte value)`. The
value at `+0x10` is what the dispatch chain ultimately uses — most
plausibly a function pointer or a registry index for a Lua closure.

## Insert helper `FUN_004e4dd0` — std::map::_Insert (494 B)

Starts with the classic STL max-size guard:

```asm
CMP [EDI+0x8], 0x1FFFFFFE     ; max_size check
JC <ok>
<throw length_error>           ; calls FUN_00404120 / FUN_00404390 with a string
<ok>:
... actual insert work calling FUN_004e4620 ...
```

`0x1FFFFFFE = (2^29 - 2)` is MSVC's stated max_size for `std::map<u32,
T>` where T is ~4 bytes. (The exact value depends on key + value
sizes; this is consistent with `(0x80000000 / sizeof(_Tree_node))-1`
≈ `0x1FFFFFFE` for ~16-byte nodes.)

So **`FUN_004e4dd0` is `std::map::_Insert_at`** — the insert helper
called by both `operator[]` (when key absent) and `insert()`. Its
helper `FUN_004e4620` is the lower-level tree-rebalance routine.

## The 3 channel classes

Searching for RTTI strings recovered the network class layout. The
3 channels are namespace-scoped under `Application::Network`:

| Channel | Namespace |
|---|---|
| **LobbyProtoChannel** | `Application::Network::LobbyProtoChannel` |
| **ZoneProtoChannel** | `Application::Network::ZoneProtoChannel` |
| **ChatProtoChannel** | `Application::Network::ChatProtoChannel` |

Each channel **nests** the same sub-class hierarchy:

```
<X>ProtoChannel                             (the channel namespace itself)
├── ServiceConsumerConnectionManager
│   └── ConsumerConnection
│       └── ConnectionManagerTmpl<T<X>ProtoUp, T<X>ProtoDown>  (template-instantiated)
├── ClientPacketBuilder                     (outgoing packet emit)
├── <X>ProtoDownCallbackInterface           (parent of DummyCallback)
└── <X>ProtoDownDummyCallback               (the production callback per Phase 8 #9)
```

(Where `<X>` is `Lobby`, `Zone`, or `Chat`.)

The base classes themselves (`LobbyProtoChannel`, `ZoneProtoChannel`,
`ChatProtoChannel`) do NOT have direct RTTI — they're namespace
containers for the nested classes that DO have RTTI. This is why
my initial search for `.?AVZoneProtoChannel@...` returned no hits —
the actual instantiable classes are the nested types.

### RTTI-confirmed nested classes (samples)

| Class | File offset of RTTI name | VA of TD |
|---|---:|---|
| `ZoneProtoDownDummyCallback@ZoneClient@Network@Application` | `0xf1b198` | TD at file off `0xf1b190` |
| `ZoneProtoDownCallbackInterface@ZoneProtoChannel@Network@Application` | `0xf1b1e0` | TD at file off `0xf1b1d8` |
| `ServiceConsumerConnectionManager@LobbyProtoChannel@Network@Application` | `0xf18e5c` | TD at file off `0xf18e54` |
| `ConsumerConnection@ServiceConsumerConnectionManager@LobbyProtoChannel@Network@Application` | `0xf18d24` | TD at file off `0xf18d1c` |
| `ClientPacketBuilder@LobbyProtoChannel@Network@Application` | `0xf1901c` | TD at file off `0xf19014` |
| `ServiceConsumerConnectionManager@ZoneProtoChannel@Network@Application` | `0xf1b504` | TD at file off `0xf1b4fc` |
| `ClientPacketBuilder@ZoneProtoChannel@Network@Application` | `0xf1dde4` | TD at file off `0xf1ddc` |
| `ServiceConsumerConnectionManager@ChatProtoChannel@Network@Application` | `0xf1bf5f` | TD at file off `0xf1bf57` |
| `ClientPacketBuilder@ChatProtoChannel@Network@Application` | `0xf23104` | TD at file off `0xf230fc` |

## What's at `[Rapture+0x60]` — the ChannelMgr

This is still TBD without further work. The candidates from the
Phase 8/9 analysis:

- `Application::Network::ZoneProtoChannel::ServiceConsumerConnectionManager` —
  most likely (the one that owns the Zone channel's connection state)
- A custom `Main::ChannelMgr` wrapping all 3 channels
- `Sqex::Socket::RUDP2::RUDPImpl` (the transport layer)

The "channel" at `[packet+8]` (used by `FUN_004e5ff0`) is most
plausibly one of the `<X>ProtoChannel` instances or its nested
`ServiceConsumerConnectionManager`. Each instance has its own
`std::map` of opcode → handler at `[+8]`.

## Where the tree gets populated — narrows further

`FUN_004e4dd0` is the INSERT helper. Looking at its callers:

| Caller | Insert call count | Role |
|---|---:|---|
| `FUN_004e5ca0` | 1 | `std::map::operator[]` (the lookup-or-insert) |
| `FUN_0067f1d0` | 1 | TBD — single-insert wrapper |
| `FUN_007facd0` | 1 | TBD |
| `FUN_00871050` | 1 | **ANOTHER `std::map::operator[]` — structurally identical to FUN_004e5ca0** (different K/V types) |
| `FUN_0081eeb0` | 7 | `std::map::_Insert_with_hint` (multi-branch STL machinery) |
| `FUN_00871950` | 7 | Same — different K/V types |

Critical realization: **there is no single "the binder"**. MSVC
template-instantiated the STL map operations once PER concrete K/V
type. So `FUN_004e5ca0` is operator[] for ONE map type (the one used
by Channel's per-opcode tree); `FUN_00871050` is operator[] for a
DIFFERENT map type (different concrete K/V). They have byte-identical
structure but different addresses because they belong to different
type instantiations.

This means the search for the binder narrows to: **callers of
`FUN_004e5ca0`-the-specific-instantiation that pass a Channel
instance + write a non-default value to the result**. The 20+ callers
of `FUN_004e5ca0` recovered are:

- `FUN_004e5ff0` — the known dispatch (lookup, not insert)
- `FUN_004e5d60`, `5df0`, `5f80`, `6080` — sibling network dispatchers
- `FUN_007a02d0` (×2), `7d2b10`, `7fa030` — Lua engine area helpers
- `FUN_0080d030`, `0080d170` — script-related
- `FUN_008a7b00`, `8a7b30` — receiver area
- `FUN_008e3600` — sync primitive area
- `FUN_00daf110`, `daf210`, `db33a0`, `db3430` — network primitives
- `FUN_00537620`, `FUN_00871050` (self), `FUN_0081eeb0` (self)

The most likely SCRIPT-LOAD-time binders are among the Lua-engine /
script-area callers (`FUN_007a02d0`, `0080d030`, `0080d170`). Walking
those would identify the per-opcode binder for the receiver path.

The other promising lead: each `<X>ProtoChannel::ServiceConsumerConnectionManager`
ctor (Phase 9 ext2 metadata sweep should have ctor RVAs) likely
initializes the std::map. Looking up the ctor in `class_metadata.json`
for `Application::Network::ZoneProtoChannel::ServiceConsumerConnectionManager`
would give a starting point.

### Architectural takeaway

The per-opcode receiver dispatch is implemented as a
**`std::map<opcode_u32, handler_value>` keyed on opcode**, instantiated
ONCE per Channel class. The handler_value (4 bytes) is most plausibly:

- A direct C function pointer to a wrapper that builds a Receiver
  (Pattern A/B/C from `receiver_gate_cheatsheet.md`)
- OR a Lua-VM closure handle / table index

To determine which, the next-step work is to find ONE binder call site
and look at what value gets stored. If it's a `.text` address (in
`0x1000..0xb3d000`), it's a C fn ptr. If it's a small index or a
`.rdata`/heap address, it's a Lua closure handle.

## 2026-05-17 — Found the std::map default ctor

Phase 9 ext2 metadata sweep already recovered the
`ZoneProtoChannel::ServiceConsumerConnectionManager` (SCCM) ctor at
**`FUN_00db7c50`** (183 B). Its parent ctor is **`FUN_00db8330`**
(`ZoneProtoChannel::TZoneProtoUp::ConnectionManagerTmpl`, 179 B), and
**that's the function that default-initialises the std::map**.

Confirmed std::map default-ctor pattern in `FUN_00db8330` (lines
`+0x36c..+0x38b`):

```asm
LEA EDI, [ESI+0xc]                  ; EDI = &map (this+0xc)
MOV [ESI], 0x01129754               ; write CMT vtable
MOV [ESI+8], EAX                    ; this+0x8 = ctor arg (parent ref)
CALL FUN_0095e590                   ; ⭐ _Buynode0() — allocate sentinel
MOV [EDI+0x4], EAX                  ; map._Myhead = sentinel (this+0x10)
MOV [EAX+0x15], 1                   ; sentinel->_Isnil = 1
MOV [EAX+0x4], EAX                  ; sentinel->_Parent = sentinel  (self-ref)
MOV [EAX+0x0], EAX                  ; sentinel->_Left = sentinel
MOV [EAX+0x8], EAX                  ; sentinel->_Right = sentinel
MOV [EDI+0x8], 0                    ; map._Mysize = 0
```

This is the **textbook MSVC STL `std::map::_Tree::_Tree()`** default
constructor. Confirms the data structure 100%.

The other recovered ctors:
- `ChatProtoChannel::SCCM` ctor: `FUN_00db2820` (write at +0x49)
- `ChatProtoChannel::TChatProtoUp::ConnectionManagerTmpl` ctor: `FUN_00db3bb0` (write at +0x35)
- `ZoneProtoChannel::SCCM` ctor: `FUN_00db7c50` (write at +0x49)
- `ZoneProtoChannel::TZoneProtoUp::ConnectionManagerTmpl` ctor: `FUN_00db8330` (write at +0x35) ⭐ this one

`LobbyProtoChannel::SCCM` was not recovered in the Phase 9 ext2 sweep
(only 2 of the 3 channels appeared). It's there in the binary but
without a `dynamic_cast` callsite the sweep didn't pick it up.

### Offset reconciliation note

The consumer (`FUN_004e5ff0`) passes `channel+0x8` to operator[], and
operator[] reads `[arg+0x4]` (= `channel+0xc`) as the sentinel ptr.
The CMT ctor writes the sentinel ptr to `this+0x10` (= `[EDI+0x4]`
where `EDI = this+0xc`). There's an apparent **4-byte offset
discrepancy** — most likely due to one of:

- MI: the consumer sees a SECONDARY base subobject offset by 4 bytes
- The "channel" parameter in `FUN_004e5ff0` is actually `&channel[-4]`
  effectively, perhaps because it was loaded from `packet[+8]` after a
  thunk-adjusted dispatch
- The ctor I identified is for ConnectionManagerTmpl, but the
  consumer's actual channel class is a DIFFERENT template instantiation
  with a different `this` layout

Resolution needs Ghidra GUI or careful walking of the SCCM/CMT
hierarchy with secondary-base thunks. Doesn't affect the high-level
architectural finding (std::map, sentinel-with-self-refs, default-
init at parent ctor).

### What the SCCM ctor adds on top of CMT

`FUN_00db7c50` (SCCM ctor) does AFTER calling the parent CMT ctor:

```c
SCCM::SCCM(this, arg) {
    CMT::CMT(this, arg);                ; parent ctor (FUN_00db8330)
    EDI = this+0x84;                    ; sub-object at +0x84
    [this] = 0x01129768;                ; OVERRIDE vtable to SCCM
    CALL FUN_00db7b40;                  ; init sub-object at +0x84 (?)
    PUSH 0x10e0;                        ; alloc 0x10e0 (4320) bytes
    CALL operator new;
    if (success) {
        FUN_00db76f0(buf, this);        ; init the 4320-byte buffer
        [this+0xf4] = buf;
    }
}
```

So SCCM adds:
- An override vtable at `[this+0]` (SCCM-specific)
- A sub-object at `[this+0x84]` (initialised by `FUN_00db7b40`)
- A 4320-byte heap-allocated buffer pointed to by `[this+0xf4]`
  (initialised by `FUN_00db76f0`)

The 4320-byte buffer is interesting — that's a substantial amount of
storage. Plausibly the per-opcode handler table itself, or the packet
queue. Worth a separate Ghidra walk.

The std::map at `this+0xc` (from CMT) remains EMPTY after the ctor
chain. Population happens later — exactly the binder we're hunting.

## 2026-05-17 — `ConsumerConnection` identified + lifecycle callbacks recovered

Walked `FUN_00db76f0` (the 4320-byte buffer initializer called by SCCM
ctor). Two important recoveries:

### 1. The buffer IS a `ConsumerConnection` instance

`FUN_00db76f0` writes its own vtable to `[buffer+0]` at offset +0x46:
```asm
MOV [ESI], 0x0112973c
```

Identified that vtable via COL→TD walk:

```
RTTI: .?AVConsumerConnection@ServiceConsumerConnectionManager@ZoneProtoChannel@Network@Application@@
vtable @ RVA 0xd2973c, COL @ RVA 0xda2f10, TD @ RVA 0xf1cc68

slot 0: FUN_00db8270  (scalar deleting dtor)
slot 1: FUN_00db7d10  (drain-incoming-buffer hook — 306 B)
slot 2: FUN_00db7440  (TBD, 268 B)
slot 3: FUN_00db7420  (small)
slot 4: FUN_00db73c0  (small)
slot 5: FUN_00da2f5c  (.rdata — not a fn; bleed into next vtable)
slot 6: FUN_00db83f0
```

So **`ConsumerConnection@ServiceConsumerConnectionManager@ZoneProtoChannel`
is the channel class** — a 4320-byte heap-allocated object owned by the
SCCM at SCCM[+0xf4]. Each `*ProtoChannel`'s SCCM owns its own
ConsumerConnection.

### 2. `FUN_00db76f0` registers 5 LIFECYCLE callbacks (not per-opcode)

The pattern at lines `+0xe5..+0x12e` of FUN_00db76f0:

```asm
PUSH 0xdb7660; MOV ECX, EDI; CALL FUN_00d362c0     ; register callback A
PUSH 0xdb7680; MOV ECX, EDI; CALL FUN_00d36280     ; register callback B
PUSH 0xdb76c0; MOV ECX, EDI; CALL FUN_00d364a0     ; register callback C
PUSH 0x78;    PUSH 0xdb76a0; MOV ECX, EDI; CALL FUN_00d363e0  ; register callback D with size 0x78
PUSH 0x708;   PUSH 0xdb76a0; MOV ECX, EDI; CALL FUN_00d36440  ; register callback D (again) with size 0x708
PUSH [ESI+0x4c]; MOV ECX, EDI; CALL FUN_00d361c0   ; bind something else
```

Where `EDI = buffer+0x58` (the registry sub-object inside
ConsumerConnection).

The 4 callbacks at `0xdb7660`, `0xdb7680`, `0xdb76a0`, `0xdb76c0` are
small (16-32 byte) handlers — first-bytes inspection shows they read
arg[+0x4], dereference further, and modify or null-check fields at
`+0x80`. **These are LIFECYCLE handlers** (on-connect/on-disconnect/
on-error/on-ready type events), NOT per-opcode dispatch.

### 3. Per-opcode std::map is NOT inside ConsumerConnection

Slot 1 of ConsumerConnection (`FUN_00db7d10`, 306 B) treats `[this+0x8..+0x18]`
as a **circular packet buffer** (read-position word at +0x16, write-
position word at +0x14, ring data at +0x10), NOT as an `std::map`.

So the `channel+0x8` I traced earlier as a std::map in `FUN_004e5ff0`'s
context belongs to a DIFFERENT class than ConsumerConnection. The
"channel" parameter in `FUN_004e5ff0` is some other type — most
likely an inner sub-object of ConsumerConnection or a different
hierarchy entirely.

The chain reconstruction needs revisiting:
- `FUN_004e30a0` (ChannelMgr tick) calls `FUN_00dae520` (dummy
  callback dispatch)
- `FUN_00dae520` populates a local 0x14-byte buffer via
  `FUN_00db1960` (the dequeue) — local buffer's [+0x8] = the
  dispatched packet
- Dispatch reads `packet[+0x8] = something at packet+0x24` (per
  Phase 8 #9's analysis of the dispatcher `FUN_00dbfd10`)
- `FUN_004e20a0` reads `[ESP+0x2c]` (= local buffer's [+0x8] = packet ptr)
- Calls `FUN_004e5ff0` with `MOV ECX, [EDI+8]` where EDI = local buf

So `FUN_004e5ff0`'s "channel" parameter is **`packet[+8]`**, which is
some struct INSIDE the packet (not the ConsumerConnection itself).
That struct has the std::map at its +0x8. It could be:
- A `PacketHeader` sub-struct with a dispatch sub-table
- A `Channel*` back-pointer that resolves to a different object than
  ConsumerConnection
- A `ContextSegment` (per the RUDP2 segment classes recovered)

### Recovered class names (from this round)

| Class | RTTI | Vtable RVA | Notes |
|---|---|---|---|
| `ConsumerConnection@SCCM@ZoneProtoChannel@Network@Application` | `0xf1cc68` | `0xd2973c` | 7+ slots; the per-channel object (4320 B) |
| (other 2 ProtoChannels' ConsumerConnection) | TBD | TBD | Same template instantiation expected |

### 5 register methods on the inner registry sub-object (buffer+0x58)

| Method | Used with | Notes |
|---|---|---|
| `FUN_00d362c0` | callback ptr `0xdb7660` | Register a "no-arg" callback |
| `FUN_00d36280` | callback ptr `0xdb7680` | Register a different "no-arg" callback |
| `FUN_00d364a0` | callback ptr `0xdb76c0` | Register another callback |
| `FUN_00d363e0` | callback ptr `0xdb76a0` + size `0x78` | Register with a size hint |
| `FUN_00d36440` | callback ptr `0xdb76a0` (same) + size `0x708` | Register the SAME callback with a different size |
| `FUN_00d361c0` | `&this+0x4c` ptr | Bind to an inner sub-object |

The "register same callback with 2 different sizes" pattern
(`0xdb76a0` with `0x78` then `0x708`) is interesting — could be 2
size-classes of the same packet kind, OR 2 different "buckets"
served by the same handler.

### What this leaves open for Half A

The original Half A goal — finding the per-opcode handler tree
binder — remains open. This session's progress:

- ✅ Confirmed `ConsumerConnection` is the per-channel class
- ✅ Identified its vtable (0xd2973c)
- ✅ Recovered 5 lifecycle callback registrations
- ❌ Per-opcode std::map is NOT at ConsumerConnection+0x8 (that's a
  circular buffer)
- 🔲 Need to identify what type `packet[+8]` actually is (the "channel"
  in FUN_004e5ff0's POV)
- 🔲 Find where the per-opcode tree lives

Concrete next-session leads:
1. **Walk slot 2 of ConsumerConnection** (`FUN_00db7440`, 268 B) — the
   "commit" hook called by `FUN_004e5ff0` after the operator[] lookup.
   May reveal where the per-opcode dispatch actually happens.
2. **Walk `FUN_00d36020`** — the dispatcher inside slot 1's drain loop;
   takes the registry sub-object at `buf+0x58` and processes a buffer
   chunk. Might contain the per-opcode lookup.
3. **Identify the actual `packet[+8]` type** — re-trace from
   `FUN_004e5ff0`'s caller through `FUN_004e20a0`. Re-validate that the
   "channel" parameter isn't actually something else (PacketHeader,
   DispatchContext, etc.).

## 2026-05-17 — ConsumerConnection slot 2 + FUN_00d36020 walked

Both walked; both **NOT** the per-opcode dispatcher.

### ConsumerConnection slot 2 (FUN_00db7440, 392 B) — outgoing buffer commit

Behavior: enters critical section via IAT call → manages a 4096-byte
outgoing buffer at `[this+0x88..+0x1088]` (write position `[+0x28]`,
size `[+0x1088..+0x1098]`) → memcpy via `FUN_009d4600` → if buffer
overflows (>= 0x1000) allocate spill via `FUN_009d5110` → exits
critical section via IAT JMP.

This is the **outgoing packet serialization commit** — assembling
bytes for transmission. Not per-opcode dispatch.

### FUN_00d36020 (52 B) — first-non-null callback dispatcher

```c
int dispatch(this) {
    if (this[+0xc])  JMP this[+0xc]->vtable[8](this[+0xc]);   // callback A
    if (this[+0x14]) JMP this[+0x14]->vtable[10](this[+0x14]); // callback B
    if (this[+0x1c]) {                                          // callback C
        ECX = this[+0x1c];
        JMP this[+0x1c]->vtable[9]();
    }
    return -1;
}
```

The registry at `ConsumerConnection+0x58` has 3 polymorphic callback
slots; this fn invokes the first non-null one. NOT per-opcode dispatch.

### Re-tracing packet[+8]'s true type

Re-examining `FUN_00db1960` (the dequeue), each queue entry has:
- `[+0]`: next pointer (list-style)
- `[+8]`: the "packet header" value (copied to out_struct[+8])

This is an **MSVC `std::list<T>`** node (the `_Next/_Prev/_Myval`
layout). So packets are enqueued as `std::list<PacketHeader*>` entries,
not a custom queue.

The "channel" parameter in `FUN_004e5ff0` (ECX = packet[+8]) is a
**`PacketHeader`** class — has its OWN vtable, std::map at +0x8, and
inspector context at +0x14. **Distinct from ConsumerConnection**.

PacketHeader's class identity is the remaining mystery. To find it:
- Need to identify what writes the vtable at `[fresh_packet_header+0]`
- Most likely in the RUDP2 receive layer (decompression / packet
  reassembly), upstream of the queue
- Could also be a template instantiation (per-channel PacketHeader
  type, since each ProtoChannel has its own `PacketBufferTmpl`
  per Phase 9 ext2 metadata)

### Cumulative class chain — what's known now

```
Main  (?AVMain@@)
  ↓ +0x30
Rapture::Application  (vt 0xb8cc1c)
  ↓ +0x60
ChannelMgr  (RTTI TBD; non-virtual tick = FUN_004e30a0)
  ↓ +0x234
NetworkManager  (RTTI TBD; vtable[7] = the actual work fn called by FUN_004e30a0)
  ↓ owns 3 instances of
ServiceConsumerConnectionManager (SCCM)  per ProtoChannel
  - ZoneProtoChannel::SCCM: vt @ 0x01129768, ctor FUN_00db7c50 (183 B)
  - ChatProtoChannel::SCCM: ctor FUN_00db2820
  - LobbyProtoChannel::SCCM: TBD (not in Phase 9 ext2 sweep)
  ↓ extends
ConnectionManagerTmpl (CMT)
  - ZoneProtoChannel::TZoneProtoUp::CMT: vt @ 0x01129754, ctor FUN_00db8330
  - CMT's ctor initializes a std::map at this+0xc (sentinel + size=0)
  ↓ allocates at SCCM[+0xf4]
ConsumerConnection (vt @ 0x0112973c, 4320 bytes)
  - ctor body: FUN_00db76f0
  - slot 1 (drain): FUN_00db7d10 — uses [+0x8..+0x18] as circular buffer
  - slot 2 (commit): FUN_00db7440 — uses [+0x88..+0x1088] as 4 KB outgoing buffer
  - lifecycle callbacks at +0x58 (registered via FUN_00d36280/2c0/3e0/440/4a0/61c0)
  ↓ produces/dequeues
PacketHeader  (RTTI TBD — the remaining mystery)
  - has vtable at [+0]
  - has std::map<u32 opcode, T> at [+0x8] — the per-opcode tree
  - has inspector context at [+0x14]
  - inspector inner struct at [+0x24] holds opcode at [+2]
  - referenced from queue entries' [+8]
```

**Per-opcode binder = STILL OPEN.** The PacketHeader class is the
remaining identification target. Once recovered, its ctor will reveal
where the per-opcode std::map gets populated.

### Recommendation for next pass

Rather than continuing the binder hunt (which has shown 5 successive
"close but not it" results), best ROI is probably to **pivot to a
different Phase 9 thread** (e.g. #8e final push, Phase 4 matching, or
SetPushEventCondition template). The architectural map is now
extensive enough that a future session with fresh focus can finish
Half A in one targeted attempt.

## Practical impact

- **The `channel[+8]` tree IS an `std::map<u32, T>`**, not a custom
  structure. This significantly narrows the search for the writer —
  we're looking for STL-shaped insert calls, not custom registration
  fns.
- **The 3 channels share an identical nested-class template**
  (`ProtoChannel::{ServiceConsumerConnectionManager, ConsumerConnection,
  ClientPacketBuilder, ProtoDownCallbackInterface, ProtoDownDummyCallback}`).
  This means any decomp on one channel transfers across all 3 — the
  per-opcode tree population code is parameterized on the channel
  type (likely a template).
- **Lookup result is just 4-byte node value (`_Myval.second`)**, which
  is most likely a function pointer or registry index. Confirming
  that it's an fn ptr (vs index) would tell us whether the dispatch
  is direct (call the value as a function) or indirect (look up the
  value in another table to get the function).

## Cross-references

- `docs/packet_dispatch_router.md` — Phase 9 #5 (`FUN_004e5ff0` →
  `FUN_004e5ca0` chain; this doc identifies what `FUN_004e5ca0`
  actually walks — `std::map<K,V>`)
- `docs/receiver_dispatch_via_actorimpl.md` — Phase 9 #5 (the 35
  receivers mapped to LuaActorImpl slots; the std::map's value at
  `[+0x10]` of each entry is probably an index or fn ptr that
  resolves through the LuaActorImpl dispatch)
- `docs/rapture_application_hierarchy.md` — Phase 9 #5 Half A
  (Main / Rapture::Application / ChannelMgr hierarchy)
- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 (the
  `ZoneProtoDownDummyCallback` analysis — confirms one of the
  RTTI-recovered class names here)
- `build/class_metadata.json` — RTTI sweep (Phase 9 ext2; ctor RVAs
  for the recovered network classes if any were RTTI-cast'd elsewhere)
