#!/usr/bin/env python3
"""Phase 9 #5 — map each Receiver class to the LuaActorImpl / NullActorImpl
vtable slot that dispatches to it.

Discovery: the 43 Receiver classes don't have direct CALL-rel32 callers.
Their `Receive` method is invoked from inside a sibling class's vtable slot
under the same Lua-engine namespace:

    Component::Lua::GameEngine::{LuaActorImpl, NullActorImpl}

Each Receiver maps to ONE slot in those two parallel 90-slot vtables. The
slot's body either:
  (a) stack-builds the Receiver as a temporary, invokes Receive, then dtors
      it in-place — the pattern for the 24 short-lived 2-slot Receivers
      (e.g. SetEventStatus, ChangeSystemStat, AchievementPoint), OR
  (b) heap-allocates the Receiver and threads it through the event lifecycle
      (ctor, Receive via vtable, dtor via virtual dtor) — the pattern for the
      3 long-lived 5-slot Service-Order-Event Receivers (KickReceiver,
      StartServerOrderEventFunctionReceiver, EndClientOrderEventReceiver)
      which occupy slots 56/57/58.

Receivers without a LuaActorImpl wrapper (~14 of 43) are held as instance
fields of some higher-level event handler — they're invoked via that owner's
polymorphic dispatch rather than a stack temporary.

Output: `build/wire/ffxivgame.receiver_actorimpl_map.{json,md}`.
"""
import json
import os
import re
import struct
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(REPO, 'orig', 'ffxivgame.exe')
ASM_DIR = os.path.join(REPO, 'asm', 'ffxivgame')
IMAGE_BASE = 0x400000

# Vtable RVAs for LuaActorImpl + NullActorImpl, from the COL→TD walk
LUA_ACTOR_IMPL_VT_RVA = 0xbdfb2c
NULL_ACTOR_IMPL_VT_RVA = 0xbe02ac
VT_SLOT_COUNT = 90

# 43 Receivers — source: docs/receiver_classes_inventory.md
RECEIVERS = [
    # (leaf_name, namespace, vt_rva, slot_count)
    ('ExecutePushOnEnterTriggerBoxReceiver', 'System', 0xbdfaf8, 2),
    ('ExecutePushOnLeaveTriggerBoxReceiver', 'System', 0xbdfb04, 2),
    ('AttributeTypeEventEnterReceiver',      'System', 0xbdfb10, 2),
    ('AttributeTypeEventLeaveReceiver',      'System', 0xbdfb1c, 2),
    ('ChocoboReceiver',                      'System', 0xc57598, 2),
    ('ChocoboGradeReceiver',                 'System', 0xc575a4, 2),
    ('GoobbueReceiver',                      'System', 0xc575b0, 2),
    ('VehicleGradeReceiver',                 'System', 0xc575bc, 2),
    ('ChangeActorSubStatStatusReceiver',     'System', 0xc575c8, 5),
    ('ChangeActorSubStatModeBorderReceiver', 'System', 0xc575e0, 2),
    ('ExecuteDebugCommandReceiver',          'System', 0xc575ec, 2),
    ('AchievementPointReceiver',             'Network', 0xc572ac, 2),
    ('AchievementTitleReceiver',             'Network', 0xc572b8, 2),
    ('AchievementIdReceiver',                'Network', 0xc572c4, 2),
    ('AchievementAchievedCountReceiver',     'Network', 0xc572d0, 2),
    ('AddictLoginTimeKindReceiver',          'Network', 0xc572dc, 2),
    ('ChangeActorExtraStatReceiver',         'Network', 0xc572e8, 2),
    ('ChangeSystemStatReceiver',             'Network', 0xc572f4, 2),
    ('JobChangeReceiver',                    'Network', 0xc57300, 2),
    ('ChangeShadowActorFlagReceiver',        'Network', 0xc5730c, 2),
    ('GrandCompanyReceiver',                 'Network', 0xc57318, 2),
    ('HamletSupplyRankingReceiver',          'Network', 0xc57324, 2),
    ('HamletDefenseScoreReceiver',           'Network', 0xc57330, 2),
    ('HateStatusReceiver',                   'Network', 0xc5733c, 2),
    ('EndClientOrderEventReceiver',          'Network', 0xc57348, 5),
    ('JobQuestCompleteTripleReceiver',       'Network', 0xc57360, 6),
    ('SetCommandEventConditionReceiver',     'Network', 0xc5737c, 2),
    ('SetDisplayNameReceiver',               'Network', 0xc57388, 2),
    ('SetEmoteEventConditionReceiver',       'Network', 0xc57394, 2),
    ('SetEventStatusReceiver',               'Network', 0xc573a0, 2),
    ('SetNoticeEventConditionReceiver',      'Network', 0xc573ac, 2),
    ('SetPushEventConditionWithCircleReceiver',     'Network', 0xc573b8, 2),
    ('SetPushEventConditionWithFanReceiver',        'Network', 0xc573c4, 2),
    ('SetPushEventConditionWithTriggerBoxReceiver', 'Network', 0xc573d0, 2),
    ('SetTalkEventConditionReceiver',        'Network', 0xc573dc, 2),
    ('SetTargetTimeReceiver',                'Network', 0xc573f4, 2),
    ('EntrustItemReceiver',                  'Network', 0xc57470, 2),
    ('SyncMemoryReceiver',                   'Network', 0xc5747c, 2),
    ('UserDataReceiver',                     'Network', 0xc57488, 6),
    ('KickClientOrderEventReceiver',         'Network', 0xc574b0, 5),
    ('StartServerOrderEventFunctionReceiver','Network', 0xc574c8, 5),
    ('SendLogReceiver',                      'Network', 0xc574e0, 2),
]


def load_pe():
    with open(BIN, 'rb') as f:
        data = f.read()
    dos = data[:0x40]
    e_lfanew = struct.unpack('<I', dos[0x3c:0x40])[0]
    n = struct.unpack('<H', data[e_lfanew+6:e_lfanew+8])[0]
    so = struct.unpack('<H', data[e_lfanew+0x14:e_lfanew+0x16])[0]
    sects = []
    for i in range(n):
        off = e_lfanew + 0x18 + so + i*40
        s = data[off:off+40]
        name = s[:8].rstrip(b'\x00').decode(errors='replace')
        vsz, vaddr, rsz, roff = struct.unpack('<IIII', s[8:24])
        sects.append({'name': name, 'vaddr': vaddr, 'vsize': vsz,
                      'raw_off': roff, 'raw_size': rsz})
    return data, sects


def rva_to_off(sects, rva):
    for s in sects:
        if s['vaddr'] <= rva < s['vaddr'] + max(s['vsize'], s['raw_size']):
            return s['raw_off'] + (rva - s['vaddr'])
    return None


def off_to_rva(sects, off):
    for s in sects:
        if s['raw_off'] <= off < s['raw_off'] + s['raw_size']:
            return s['vaddr'] + (off - s['raw_off'])
    return None


def read_vt(data, sects, vt_rva, n_slots):
    out = {}
    for i in range(n_slots):
        off = rva_to_off(sects, vt_rva + i*4)
        if off is None:
            continue
        val = struct.unpack('<I', data[off:off+4])[0]
        out[i] = val - IMAGE_BASE
    return out


def read_slot(data, sects, vt_rva, slot):
    off = rva_to_off(sects, vt_rva + slot*4)
    if off is None:
        return None
    return struct.unpack('<I', data[off:off+4])[0] - IMAGE_BASE


def build_asm_index():
    files = sorted(os.listdir(ASM_DIR))
    out = []
    for fn in files:
        m = re.match(r'^([0-9a-f]{8})_(.+)\.s$', fn)
        if m:
            out.append((int(m.group(1), 16), m.group(2)))
    return out


def containing_fn(idx, rva):
    lo, hi = 0, len(idx)
    while lo < hi:
        mid = (lo + hi) // 2
        if idx[mid][0] <= rva:
            lo = mid + 1
        else:
            hi = mid
    if lo == 0:
        return None
    return idx[lo - 1]


def find_callers(data, sects, target_rva):
    """Find CALL rel32 instructions targeting target_rva."""
    out = []
    for s in sects:
        if s['name'] != '.text':
            continue
        end = s['raw_off'] + s['raw_size'] - 5
        for off in range(s['raw_off'], end):
            if data[off] == 0xe8:
                rel = struct.unpack('<i', data[off+1:off+5])[0]
                csr = off_to_rva(sects, off)
                if csr + 5 + rel == target_rva:
                    out.append(csr)
    return out


def discover_ctor_writes(data, sects, vt_rva):
    """Find all .text functions that write the vtable address to [this+N]."""
    pat = struct.pack('<I', IMAGE_BASE + vt_rva)
    out = []
    pos = 0
    while True:
        i = data.find(pat, pos)
        if i < 0:
            break
        if any(s['name'] == '.text' and s['raw_off'] <= i < s['raw_off'] + s['raw_size']
               for s in sects):
            out.append(off_to_rva(sects, i))
        pos = i + 1
    return out


def main():
    data, sects = load_pe()
    asm_idx = build_asm_index()
    lua_vt = read_vt(data, sects, LUA_ACTOR_IMPL_VT_RVA, VT_SLOT_COUNT)
    null_vt = read_vt(data, sects, NULL_ACTOR_IMPL_VT_RVA, VT_SLOT_COUNT)
    lua_by_fn = {fn: s for s, fn in lua_vt.items()}
    null_by_fn = {fn: s for s, fn in null_vt.items()}

    results = []
    for (name, ns, vt_rva, nslots) in RECEIVERS:
        # Pick the Receive slot (slot 1 for 2-slot, slot 2 for 5/6-slot)
        receive_slot = 1 if nslots == 2 else 2
        receive_fn = read_slot(data, sects, vt_rva, receive_slot)

        lua_slots = set()
        null_slots = set()
        other_callers = []
        for caller_rva in find_callers(data, sects, receive_fn or 0):
            cf = containing_fn(asm_idx, caller_rva)
            if not cf:
                continue
            fn_start, fn_name = cf
            if fn_start in lua_by_fn:
                lua_slots.add(lua_by_fn[fn_start])
            elif fn_start in null_by_fn:
                null_slots.add(null_by_fn[fn_start])
            else:
                other_callers.append(fn_name)

        # For 5/6-slot receivers Receive isn't called directly (it's a vtable slot
        # invoked via the heap-receiver's vtable). In that case fall back to
        # ctor-write callers: who calls the ctor of this Receiver?
        if not lua_slots and not null_slots and nslots > 2:
            ctor_writes = discover_ctor_writes(data, sects, vt_rva)
            ctor_fns = set()
            for w in ctor_writes:
                cf = containing_fn(asm_idx, w)
                if cf:
                    ctor_fns.add(cf[0])
            # Find callers of those ctor functions
            for ctor_fn in ctor_fns:
                for caller_rva in find_callers(data, sects, ctor_fn):
                    cf = containing_fn(asm_idx, caller_rva)
                    if not cf:
                        continue
                    fn_start, fn_name = cf
                    if fn_start in lua_by_fn:
                        lua_slots.add(lua_by_fn[fn_start])
                    elif fn_start in null_by_fn:
                        null_slots.add(null_by_fn[fn_start])

        results.append({
            'name': name,
            'namespace': ns,
            'vtable_rva': hex(vt_rva),
            'vtable_slots': nslots,
            'receive_slot': receive_slot,
            'receive_fn_rva': hex(receive_fn) if receive_fn else None,
            'lua_actor_impl_slots': sorted(lua_slots),
            'null_actor_impl_slots': sorted(null_slots),
            'other_caller_fns': sorted(set(other_callers))[:5],
        })

    out_json = os.path.join(REPO, 'build', 'wire',
                            'ffxivgame.receiver_actorimpl_map.json')
    os.makedirs(os.path.dirname(out_json), exist_ok=True)
    with open(out_json, 'w') as f:
        json.dump(results, f, indent=2)

    # Pretty markdown report
    out_md = out_json.replace('.json', '.md')
    with open(out_md, 'w') as f:
        f.write('# Receiver → LuaActorImpl / NullActorImpl slot map\n\n')
        f.write('> Auto-generated by `tools/extract_receiver_actorimpl_map.py`.\n')
        f.write('> Source: `ffxivgame.exe` static analysis.\n\n')
        f.write('Each Receiver\'s `Receive` method (slot 1 for 2-slot variants, '
                'slot 2 for 5/6-slot variants) has zero direct CALL rel32 '
                'callers — invocation happens via vtable slots on the sibling '
                'classes `Component::Lua::GameEngine::LuaActorImpl` '
                '(vt 0x%x, 90 slots) and `NullActorImpl` (vt 0x%x, 90 slots). '
                'This table maps each Receiver to the slot index(es) that '
                'dispatch to it.\n\n'
                % (LUA_ACTOR_IMPL_VT_RVA, NULL_ACTOR_IMPL_VT_RVA))
        f.write('For 5/6-slot Receivers (Kick / StartServerOrderEvent / '
                'EndClientOrderEvent / JobQuestCompleteTriple / UserData / '
                'ChangeActorSubStatStatus), `Receive` is itself only reachable '
                'through the heap-receiver\'s vtable — the slot mapping is '
                'derived from the **ctor**-write callsites instead.\n\n')
        f.write('| Receiver | NS | Slots | LuaActorImpl slot | NullActorImpl slot | Other callers |\n')
        f.write('|---|---|---:|---|---|---|\n')
        for r in results:
            lua = ', '.join(str(s) for s in r['lua_actor_impl_slots']) or '—'
            nul = ', '.join(str(s) for s in r['null_actor_impl_slots']) or '—'
            oth = ', '.join(r['other_caller_fns']) or '—'
            f.write(f'| `{r["name"]}` | {r["namespace"]} | {r["vtable_slots"]} '
                    f'| {lua} | {nul} | {oth} |\n')
        # By-slot view
        f.write('\n## By-slot view (LuaActorImpl)\n\n')
        f.write('| Slot | Slot fn | Receiver |\n|---:|:---|:---|\n')
        slot_to_recv = {}
        for r in results:
            for s in r['lua_actor_impl_slots']:
                slot_to_recv.setdefault(s, []).append(r['name'])
        for s in sorted(slot_to_recv):
            fn_rva = lua_vt.get(s)
            fn_str = f'`FUN_{fn_rva + IMAGE_BASE:08x}`' if fn_rva else '—'
            f.write(f'| {s} | {fn_str} | {", ".join(slot_to_recv[s])} |\n')

    print(f'Wrote {out_json}')
    print(f'Wrote {out_md}')
    mapped = sum(1 for r in results if r['lua_actor_impl_slots'] or r['null_actor_impl_slots'])
    print(f'Receivers mapped: {mapped} / {len(results)}')


if __name__ == '__main__':
    main()
