#!/usr/bin/env python3
"""Sweep all __RTDynamicCast (FUN_009da6cc) callsites in ffxivgame.exe,
extract (SrcType, TargetType) RTTI pairs from preceding PUSH literals,
resolve via PE-data-section lookup, and dump a class-hierarchy table."""

import os
import re
import struct
import json
import sys
from collections import defaultdict

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASM_DIR = os.path.join(REPO, 'asm', 'ffxivgame')
BIN = os.path.join(REPO, 'orig', 'ffxivgame.exe')
IMAGE_BASE = 0x400000


def load_pe_sections(path):
    with open(path, 'rb') as f:
        dos = f.read(0x40)
        e_lfanew = struct.unpack('<I', dos[0x3c:0x40])[0]
        f.seek(e_lfanew)
        pe_hdr = f.read(0x18)
        n = struct.unpack('<H', pe_hdr[6:8])[0]
        so = struct.unpack('<H', pe_hdr[0x14:0x16])[0]
        f.seek(e_lfanew + 0x18 + so)
        return [struct.unpack('<8sIIII', f.read(40)[:24]) for _ in range(n)]


def rva_to_off(sects, rva):
    for name, vs, va, rs, ro in sects:
        if va <= rva < va + max(vs, rs):
            return ro + (rva - va)
    return None


def read_rtti_name(bin_data, sects, abs_addr):
    """Read an MSVC RTTI Type Descriptor's mangled-name string."""
    rva = abs_addr - IMAGE_BASE
    off = rva_to_off(sects, rva)
    if off is None or off + 8 + 200 > len(bin_data):
        return None
    # Type Descriptor: [vtable_ptr][spare][mangled_name_null_terminated]
    name_bytes = bin_data[off + 8: off + 8 + 200]
    null = name_bytes.find(b'\x00')
    if null < 0:
        return None
    try:
        return name_bytes[:null].decode('latin-1', errors='replace')
    except Exception:
        return None


def demangle(name):
    """Crude MSVC mangling demangle: .?AV<Name>@<NS>@<NS>@...@@ → NS::...::Name"""
    if not name or not name.startswith('.?A'):
        return name
    m = re.match(r'\.\?A[VU](.*?)@@', name)
    if not m:
        return name
    parts = m.group(1).rstrip('@').split('@')
    if not parts:
        return name
    cls = parts[0]
    ns = parts[1:]
    return '::'.join(reversed(ns)) + '::' + cls if ns else cls


PUSH_LITERAL_RE = re.compile(r'^\s+[0-9a-f]+:\s+[0-9a-f ]+\s+PUSH 0x([0-9a-f]+)\s*$')
CALL_RE = re.compile(r'^\s+([0-9a-f]+):\s+[0-9a-f ]+\s+CALL 0x009da6cc\s*$')
FUNC_HEADER_RE = re.compile(r'^# function (FUN_[0-9a-f]+)')


def parse_asm_file(path):
    """Yield (fn_name, call_rva, src_type, target_type) tuples for each
    __RTDynamicCast callsite in the file."""
    fn_name = None
    pushes = []  # list of (rva, literal_value) — most recent at end
    with open(path) as f:
        for line in f:
            if line.startswith('# function '):
                m = FUNC_HEADER_RE.match(line)
                if m:
                    fn_name = m.group(1)
                pushes = []
                continue
            push_m = PUSH_LITERAL_RE.match(line)
            if push_m:
                val = int(push_m.group(1), 16)
                # Keep only literals that LOOK like RTTI/data pointers (>= 0x100000)
                pushes.append(val)
                if len(pushes) > 10:
                    pushes.pop(0)
                continue
            call_m = CALL_RE.match(line)
            if call_m:
                call_rva = int(call_m.group(1), 16)
                # Extract last 2 PUSH literals that look like RTTI pointers
                rtti_pushes = [v for v in pushes if v >= 0x1000000 and v < 0x2000000]
                if len(rtti_pushes) >= 2:
                    # The 2 most recent rtti pushes — order: src first, target second
                    # (Since PUSHes go right-to-left for args, last pushed = arg5 (isRef);
                    # arg3 (SrcType) is pushed BEFORE arg4 (TargetType).
                    # In file order, SrcType appears AFTER TargetType.
                    # So in our `pushes` list (chronological by file), TargetType comes
                    # FIRST (earlier in file), SrcType comes SECOND (later in file, closer
                    # to CALL).
                    src, target = rtti_pushes[-1], rtti_pushes[-2]
                    yield (fn_name, call_rva, src, target)
                else:
                    yield (fn_name, call_rva, None, None)
                # Don't reset pushes — same fn may have multiple casts


def main():
    sects = load_pe_sections(BIN)
    with open(BIN, 'rb') as f:
        bin_data = f.read()

    # Walk every asm file and find every __RTDynamicCast callsite
    rtti_cache = {}
    def name_of(addr):
        if addr in rtti_cache:
            return rtti_cache[addr]
        raw = read_rtti_name(bin_data, sects, addr) if addr else None
        rtti_cache[addr] = (raw, demangle(raw) if raw else None)
        return rtti_cache[addr]

    rows = []
    edge_counts = defaultdict(int)  # (src_dem, tgt_dem) -> count
    src_targets = defaultdict(set)   # src_dem -> set of target_dem
    target_sites = defaultdict(list) # tgt_dem -> list of (caller_fn, call_rva)
    callers_by_target = defaultdict(set) # tgt_dem -> set of caller_fn
    all_classes = set()

    for fname in sorted(os.listdir(ASM_DIR)):
        if not fname.endswith('.s'):
            continue
        path = os.path.join(ASM_DIR, fname)
        for fn_name, call_rva, src, target in parse_asm_file(path):
            if not src or not target:
                continue
            src_name, src_dem = name_of(src)
            tgt_name, tgt_dem = name_of(target)
            if not src_dem or not tgt_dem:
                continue
            rows.append({
                'caller_fn': fn_name,
                'call_rva': call_rva,
                'src_addr': src,
                'tgt_addr': target,
                'src': src_dem,
                'target': tgt_dem,
            })
            edge_counts[(src_dem, tgt_dem)] += 1
            src_targets[src_dem].add(tgt_dem)
            target_sites[tgt_dem].append((fn_name, call_rva))
            callers_by_target[tgt_dem].add(fn_name)
            all_classes.add(src_dem)
            all_classes.add(tgt_dem)

    # Output summary
    print(f'## Summary')
    print(f'')
    print(f'- Total __RTDynamicCast callsites parsed: {len(rows)}')
    print(f'- Distinct SrcType classes: {len(src_targets)}')
    print(f'- Distinct TargetType classes: {len(callers_by_target)}')
    print(f'- Distinct (SrcType, TargetType) edges: {len(edge_counts)}')
    print(f'- Total distinct classes seen (src or tgt): {len(all_classes)}')
    print(f'')

    print(f'## Edges by SrcType (sorted by # distinct targets, then count)')
    print(f'')
    for src in sorted(src_targets.keys(), key=lambda s: (-len(src_targets[s]), s)):
        targets = sorted(src_targets[src])
        total_casts = sum(edge_counts[(src, t)] for t in targets)
        print(f'### SrcType: `{src}` ({len(targets)} distinct targets, {total_casts} total casts)')
        for tgt in targets:
            n = edge_counts[(src, tgt)]
            print(f'  - → `{tgt}` ({n} casts)')
        print()

    # Save raw rows for downstream processing
    out_json = os.path.join(REPO, 'build', 'dynamic_cast_callsites.json')
    os.makedirs(os.path.dirname(out_json), exist_ok=True)
    with open(out_json, 'w') as f:
        # Also include raw RTTI addresses for follow-up
        json.dump({
            'rows': rows,
            'edge_counts': {f'{k[0]} -> {k[1]}': v for k, v in edge_counts.items()},
            'rtti_addresses': {
                dem: addr for addr, (raw, dem) in rtti_cache.items()
                if dem and addr
            },
        }, f, indent=2)
    print(f'\n(Raw data dumped to {out_json})', file=sys.stderr)


if __name__ == '__main__':
    main()
