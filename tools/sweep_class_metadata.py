#!/usr/bin/env python3
"""For every RTTI Type Descriptor in build/dynamic_cast_callsites.json,
recover the class's CompleteObjectLocator(s), vtable(s), and ctor/dtor
candidate sites via PE byte-pattern walks.

Recipe (MSVC RTTI on x86):
1. RTTI Type Descriptor (TD) — fixed address (input)
2. CompleteObjectLocator (COL) — has TD ptr at +0xc. Search binary
   for 4-byte LE of TD addr; for each match, validate the prefix
   bytes 12 ahead is a valid COL (signature dword == 0 at COL+0x0).
3. vftable — has COL ptr at vftable[-1]. Search binary for 4-byte LE
   of COL addr; for each match, vtable starts at match_offset + 4.
4. Ctor/dtor — write vtable addr to [this]. Search binary for 4-byte
   LE of vtable abs addr (= image_base + vtable_rva). Filter to
   matches in .text section. Classify by containment + pattern."""

import json
import os
import re
import struct
import sys
from collections import defaultdict

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(REPO, 'orig', 'ffxivgame.exe')
ASM_DIR = os.path.join(REPO, 'asm', 'ffxivgame')
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
        sects = []
        for _ in range(n):
            s = f.read(40)
            name = s[:8].rstrip(b'\x00').decode()
            vsz, vaddr, rsz, roff = struct.unpack('<IIII', s[8:24])
            sects.append({'name': name, 'vaddr': vaddr, 'vsize': vsz,
                          'raw_off': roff, 'raw_size': rsz})
        return sects


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


def off_to_section(sects, off):
    for s in sects:
        if s['raw_off'] <= off < s['raw_off'] + s['raw_size']:
            return s['name']
    return None


def find_all_4byte_refs(bin_data, target_value):
    """Find all file offsets where target_value appears as a 4-byte LE integer."""
    pattern = struct.pack('<I', target_value)
    out = []
    pos = 0
    while pos < len(bin_data):
        i = bin_data.find(pattern, pos)
        if i < 0:
            break
        out.append(i)
        pos = i + 1
    return out


def find_cols_for_td(bin_data, sects, td_abs_addr):
    """For a Type Descriptor at td_abs_addr, find all COL addresses
    that reference it. A COL has the TD ptr at its +0xc field."""
    cols = []
    for ref_off in find_all_4byte_refs(bin_data, td_abs_addr):
        # If this match is the TD ptr at COL+0xc, then COL_off = ref_off - 0xc
        col_off = ref_off - 0xc
        if col_off < 0:
            continue
        # Validate: COL+0x0 should be signature dword (= 0 for 32-bit PE)
        sig = int.from_bytes(bin_data[col_off:col_off+4], 'little')
        if sig != 0:
            continue
        # Validate: COL must be in .rdata or .data
        sec = off_to_section(sects, col_off)
        if sec not in ('.rdata', '.data'):
            continue
        col_rva = off_to_rva(sects, col_off)
        cols.append({
            'rva': col_rva,
            'abs': IMAGE_BASE + col_rva,
            'sig': sig,
            'offset_in_obj': int.from_bytes(bin_data[col_off+4:col_off+8], 'little'),
            'chd_ptr': int.from_bytes(bin_data[col_off+0x10:col_off+0x14], 'little'),
        })
    return cols


def find_vtables_for_col(bin_data, sects, col_abs_addr):
    """For a COL at col_abs_addr, find all vftable addresses.
    vftable[-1] = COL ptr, so vtable starts at match_offset + 4."""
    vtables = []
    for ref_off in find_all_4byte_refs(bin_data, col_abs_addr):
        # If this match is vftable[-1], vtable_off = ref_off + 4
        vt_off = ref_off + 4
        sec = off_to_section(sects, vt_off)
        if sec not in ('.rdata', '.data'):
            continue
        vt_rva = off_to_rva(sects, vt_off)
        # Read slot 0 (first method ptr) to give a quick sanity signal
        slot0 = int.from_bytes(bin_data[vt_off:vt_off+4], 'little')
        vtables.append({
            'rva': vt_rva,
            'abs': IMAGE_BASE + vt_rva,
            'slot0': slot0,
        })
    return vtables


def find_vtable_writes(bin_data, sects, vt_abs_addr):
    """Find all file offsets where vt_abs_addr appears as a 4-byte LE
    integer in .text. These are most likely vtable-install instructions
    in ctors/dtors."""
    writes = []
    for ref_off in find_all_4byte_refs(bin_data, vt_abs_addr):
        sec = off_to_section(sects, ref_off)
        if sec != '.text':
            continue
        rva = off_to_rva(sects, ref_off)
        writes.append(rva)
    return writes


def build_asm_function_index():
    """Walk asm/ffxivgame/ and build a sorted list of (function_start_rva,
    function_name) pairs so we can map a given RVA to its containing
    function."""
    files = sorted(os.listdir(ASM_DIR))
    out = []
    for fn in files:
        m = re.match(r'^([0-9a-f]{8})_(FUN_[0-9a-f]+|.+?)\.s$', fn)
        if m:
            rva = int(m.group(1), 16)
            out.append((rva, fn))
    return out


def find_containing_function(asm_index, rva):
    """Binary-search asm_index for the function containing rva."""
    lo, hi = 0, len(asm_index)
    while lo < hi:
        mid = (lo + hi) // 2
        if asm_index[mid][0] <= rva:
            lo = mid + 1
        else:
            hi = mid
    if lo == 0:
        return None
    return asm_index[lo - 1]


def classify_writes(writes, vt_abs, asm_index):
    """Group writes by containing function, then classify each group:
    - 1 write near function start: ctor candidate
    - 2 writes in same function (first early, second late): dtor candidate
    - Other patterns: uncategorized."""
    by_fn = defaultdict(list)
    for w_rva in writes:
        c = find_containing_function(asm_index, w_rva)
        if c is None:
            continue
        fn_start, fn_name = c
        offset_in_fn = w_rva - fn_start
        by_fn[(fn_start, fn_name)].append(offset_in_fn)

    ctors = []
    dtors = []
    uncategorized = []
    for (fn_start, fn_name), offsets in by_fn.items():
        offsets.sort()
        if len(offsets) == 1:
            # Single write — likely ctor (vtable install once at start)
            ctors.append({'fn_rva': fn_start, 'fn_name': fn_name,
                          'write_offsets': offsets})
        elif len(offsets) == 2:
            # Two writes — likely dtor (install own, then base)
            dtors.append({'fn_rva': fn_start, 'fn_name': fn_name,
                          'write_offsets': offsets})
        else:
            uncategorized.append({'fn_rva': fn_start, 'fn_name': fn_name,
                                  'write_offsets': offsets})
    return ctors, dtors, uncategorized


def estimate_vtable_slot_count(bin_data, sects, vt_off):
    """Walk vtable forward; each slot is a 4-byte function ptr in .text.
    Stop when we hit a non-.text-ptr value."""
    slots = 0
    pos = vt_off
    while pos + 4 <= len(bin_data) and slots < 256:
        val = int.from_bytes(bin_data[pos:pos+4], 'little')
        rva = val - IMAGE_BASE
        # Valid if rva is in .text
        in_text = False
        for s in sects:
            if s['name'] == '.text' and s['vaddr'] <= rva < s['vaddr'] + s['vsize']:
                in_text = True
                break
        if not in_text:
            break
        slots += 1
        pos += 4
    return slots


def main():
    # Load PE + dynamic_cast sweep output
    print('Loading PE sections...', file=sys.stderr)
    sects = load_pe_sections(BIN)
    with open(BIN, 'rb') as f:
        bin_data = f.read()

    print('Loading dynamic_cast sweep output...', file=sys.stderr)
    with open(os.path.join(REPO, 'build', 'dynamic_cast_callsites.json')) as f:
        sweep = json.load(f)

    rtti_map = sweep['rtti_addresses']  # demangled_name -> rtti abs_addr
    print(f'Loaded {len(rtti_map)} RTTI addresses', file=sys.stderr)

    # Build asm function index
    print('Building asm function index...', file=sys.stderr)
    asm_index = build_asm_function_index()
    print(f'Indexed {len(asm_index)} asm functions', file=sys.stderr)

    # For each RTTI, find COLs, vtables, ctors, dtors
    print('Walking classes...', file=sys.stderr)
    results = {}
    for i, (cls_name, rtti_abs) in enumerate(sorted(rtti_map.items())):
        if i % 20 == 0:
            print(f'  [{i}/{len(rtti_map)}] {cls_name}', file=sys.stderr)
        rtti_rva = rtti_abs - IMAGE_BASE
        cols = find_cols_for_td(bin_data, sects, rtti_abs)
        vtables = []
        for col in cols:
            for vt in find_vtables_for_col(bin_data, sects, col['abs']):
                vt_off = rva_to_off(sects, vt['rva'])
                vt['slot_count'] = estimate_vtable_slot_count(bin_data, sects, vt_off)
                vt['col_rva'] = col['rva']
                vtables.append(vt)

        # Find all ctor/dtor writes for ALL vtables of this class
        all_ctors = []
        all_dtors = []
        all_uncat = []
        for vt in vtables:
            writes = find_vtable_writes(bin_data, sects, vt['abs'])
            ctors, dtors, uncat = classify_writes(writes, vt['abs'], asm_index)
            for c in ctors: c['vtable_rva'] = vt['rva']
            for d in dtors: d['vtable_rva'] = vt['rva']
            for u in uncat: u['vtable_rva'] = vt['rva']
            all_ctors.extend(ctors)
            all_dtors.extend(dtors)
            all_uncat.extend(uncat)

        results[cls_name] = {
            'rtti_rva': rtti_rva,
            'rtti_abs': rtti_abs,
            'col_count': len(cols),
            'cols': cols,
            'vtable_count': len(vtables),
            'vtables': vtables,
            'ctor_candidates': all_ctors,
            'dtor_candidates': all_dtors,
            'uncategorized_sites': all_uncat,
        }

    # Output
    out_json = os.path.join(REPO, 'build', 'class_metadata.json')
    with open(out_json, 'w') as f:
        json.dump(results, f, indent=2)
    print(f'\nWrote {out_json}', file=sys.stderr)

    # Print summary
    print(f'\n## Class-metadata sweep summary\n')
    print(f'- Total classes processed: {len(results)}')
    n_with_vtable = sum(1 for v in results.values() if v['vtable_count'] > 0)
    n_with_ctor = sum(1 for v in results.values() if v['ctor_candidates'])
    n_with_dtor = sum(1 for v in results.values() if v['dtor_candidates'])
    n_multi_vt = sum(1 for v in results.values() if v['vtable_count'] > 1)
    print(f'- Classes with ≥1 vtable found: {n_with_vtable}')
    print(f'- Classes with multiple vtables (MI): {n_multi_vt}')
    print(f'- Classes with ≥1 ctor candidate: {n_with_ctor}')
    print(f'- Classes with ≥1 dtor candidate: {n_with_dtor}')
    print()

    # Distribution of vtable slot counts
    slot_counts = []
    for v in results.values():
        for vt in v['vtables']:
            slot_counts.append(vt['slot_count'])
    if slot_counts:
        slot_counts.sort()
        print(f'- Vtable slot count: min={slot_counts[0]} median={slot_counts[len(slot_counts)//2]} max={slot_counts[-1]} (n={len(slot_counts)})')

    # Classes with no ctor found — sentinel of incompleteness
    no_ctor = sorted(k for k, v in results.items() if not v['ctor_candidates'])
    print(f'\n- Classes with NO ctor candidate ({len(no_ctor)}):')
    for n in no_ctor[:30]:
        print(f'    {n}')
    if len(no_ctor) > 30:
        print(f'    ... and {len(no_ctor)-30} more')

    # Print top-15 most-complex classes (multi-vtable or many ctor sites)
    by_complexity = sorted(results.items(),
                           key=lambda x: -(x[1]['vtable_count']
                                           + len(x[1]['ctor_candidates'])))
    print(f'\n## Top-15 most-complex classes (by vtable count + ctor sites)\n')
    for name, info in by_complexity[:15]:
        vt_str = ', '.join(f'0x{v["rva"]:x}({v["slot_count"]} slots)' for v in info['vtables'])
        ctor_fns = ', '.join(c['fn_name'][:40] for c in info['ctor_candidates'][:3])
        if len(info['ctor_candidates']) > 3:
            ctor_fns += f' (+{len(info["ctor_candidates"])-3} more)'
        print(f'- `{name}` — {info["vtable_count"]} vtables [{vt_str}], '
              f'{len(info["ctor_candidates"])} ctors [{ctor_fns}], '
              f'{len(info["dtor_candidates"])} dtors')


if __name__ == '__main__':
    main()
