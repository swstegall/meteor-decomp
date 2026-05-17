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
