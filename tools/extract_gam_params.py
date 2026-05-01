#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Parse the mangled `Component::GAM::CompileTimeParameter<...>` type names
in `.rdata` and emit a structured table:

  (id, namespace, value_type, decorator_class, source_string)

The GAM (Game Attribute Manager) registry is the binary's master list
of wire-serializable game-state properties. Each `CompileTimeParameter`
template instantiation is one entry: a numeric id, a per-namespace
display-name placeholder, the C++ type carried by the wire payload,
and an "assignment decorator" (almost always `DecoratorSimpleAssign<T>`).

The `PARAMNAME_<id>` symbols are NOT separate strings — they're
type-template placeholders. The actual semantic interpretation
(which id == health, which == XP, etc.) lives on the server side
(Project Meteor / garlemald-server). The binary just enforces the
type discipline.

Output:
  config/<binary>.gam_params.json   — list of {id, ns, type, decorator, raw}
  config/<binary>.gam_params.csv    — same in csv (for spreadsheet import)
  build/wire/<binary>.gam_params.md — human-readable grouped report

Reads:
  config/<binary>.strings.json   (the .rdata strings dump)
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
WIRE_DIR = REPO_ROOT / "build" / "wire"

# MSVC mangled-name primitive type codes.
TYPE_CODES = {
    "C": "signed char",
    "D": "char",
    "E": "unsigned char",
    "F": "short",
    "G": "unsigned short",
    "H": "int",
    "I": "unsigned int",
    "J": "long",
    "K": "unsigned long",
    "M": "float",
    "N": "double",
    "_J": "__int64",
    "_K": "unsigned __int64",
    "_N": "bool",   # MSVC mangles bool as _N (collision with double — disambiguated by underscore prefix)
    "_W": "wchar_t",
}


def decode_int_literal(token: str) -> int | None:
    """Decode MSVC's mangled int literal: `$0<payload>@`.
    Two forms:
      - Small (0 ≤ v ≤ 9): a single digit character, e.g. `$03@` → 3.
      - Large (v ≥ 10): base-16 with A=0..P=15, e.g. `$0HE@` → 0x74 = 116.
    The trailing `@` terminates the literal in both forms."""
    m = re.match(r"^\$0([0-9A-Pa-p@]+)@$", token)
    if not m:
        return None
    payload = m.group(1)
    if payload == "@":
        return 0
    # If the first character is a digit, this is the small form (value
    # 0..9) — single digit followed by `@`.
    if payload[0].isdigit():
        try:
            return int(payload, 10)
        except ValueError:
            return None
    # Otherwise base-16 letter form.
    val = 0
    for c in payload:
        if c == "@":
            break
        if not c.isalpha():
            return None
        digit = ord(c.lower()) - ord("a")
        val = val * 16 + digit
    return val


# Match the full CompileTimeParameter mangled string. The shape:
#   .?AV?$CompileTimeParameter@<id-token>@$1?<paramname>@<ns chain>@@<sigtype>P<storage>D<storage>...<type-code>V?$DecoratorSimpleAssign@<type-code>@GAM@Component@@@GAM@Component@@
# We capture:
#   group(1) = id token (`$0XX@` form)
#   group(2) = PARAMNAME_<id> + namespace chain (e.g. "PARAMNAME_116@CharaMakeData@Data@GameAttributeManager@Network@Application@@")
#   group(3) = type sig (everything between the closing `@@` of paramname and `V?$Decorator...`)
#   group(4) = decorator class (e.g. "DecoratorSimpleAssign@F@GAM@Component@@")
RE_CTP = re.compile(
    # Match both the standalone (".?AV?$CompileTimeParameter@...") and
    # the ParameterCollection-wrapped ("V?$CompileTimeParameter@...")
    # forms — the latter shows up nested inside Collection_N templates.
    r"\?\$CompileTimeParameter@"
    r"(\$0[0-9A-Pa-p@]+@)"                        # id literal (small or large form)
    r"\$1\?"
    r"([A-Za-z_][A-Za-z0-9_]*@(?:[A-Za-z_][A-Za-z0-9_]*@)*@)"  # PARAMNAME + ns chain ending @@
    r"3PADA"                                      # storage class: const char* const
    r"(.*?)"                                      # arbitrary middle: storage qualifiers + type code
    r"V\?\$(Decorator[A-Za-z]+)@"                 # decorator class name
)


def _ns_to_canonical(ns_chain: str) -> tuple[str, str]:
    """Take the MSVC-mangled namespace chain like
    'PARAMNAME_116@CharaMakeData@Data@GameAttributeManager@Network@Application@@'
    and return (paramname, canonical_namespace).
    The chain is reverse: outermost-first."""
    parts = [p for p in ns_chain.split("@") if p]
    if not parts:
        return ("?", "?")
    paramname = parts[0]
    # Outer-to-inner namespace = reverse of the rest.
    ns = "::".join(reversed(parts[1:]))
    return (paramname, ns)


def _decode_type_token(token: str) -> str:
    """Best-effort decode of the mangled type code from the middle
    section of a CompileTimeParameter mangled string. Returns a
    friendly type name like `signed char[7]` or `Sqex::Misc::Utf8String`,
    or the raw token if unrecognised."""
    token = token.strip()
    if not token:
        return "?"
    # Direct primitive code (`F`, `C`, `H`, etc.)
    if token in TYPE_CODES:
        return TYPE_CODES[token]
    # Trailing primitive code after some prefix.
    if len(token) <= 3 and token[-1] in TYPE_CODES:
        return TYPE_CODES[token[-1]]

    # Component::GAM::Array<T, N> — signature: U?$Array@<typecode>$0<n>@<ns>@@
    # Note: MSVC int literals terminate with a single `@`, not double.
    m = re.match(r"^U\?\$Array@(_?[A-Z])\$0([0-9A-Pa-p@]+)@GAM@Component@@$", token)
    if m:
        elem_code, n_token = m.group(1), m.group(2)
        elem_type = TYPE_CODES.get(elem_code, f"raw:{elem_code}")
        n = decode_int_literal(f"$0{n_token}@")
        return f"{elem_type}[{n}]"

    # Component::GAM::Array<Component::GAM::Blob<N>, M>
    m = re.match(r"^U\?\$Array@U\?\$Blob@\$0([0-9A-Pa-p@]+)@GAM@Component@@\$0([0-9A-Pa-p@]+)@GAM@Component@@$", token)
    if m:
        blob_size = decode_int_literal(f"$0{m.group(1)}@")
        n = decode_int_literal(f"$0{m.group(2)}@")
        return f"Blob<{blob_size}>[{n}]"

    # Component::GAM::Blob<N>
    m = re.match(r"^U\?\$Blob@\$0([0-9A-Pa-p@]+)@GAM@Component@@$", token)
    if m:
        n = decode_int_literal(f"$0{m.group(1)}@")
        return f"Blob<{n}>"

    # Sqex::Misc::Utf8String
    if token == "VUtf8String@Misc@Sqex@@":
        return "Sqex::Misc::Utf8String"

    return f"raw:{token}"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", help="binary stem, e.g. ffxivgame")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    strings_path = CONFIG / f"{stem}.strings.json"
    if not strings_path.exists():
        print(f"error: missing {strings_path}; run import_to_ghidra.py first", file=sys.stderr)
        return 1

    strs = json.loads(strings_path.read_text())
    out_json = CONFIG / f"{stem}.gam_params.json"
    out_csv = CONFIG / f"{stem}.gam_params.csv"
    WIRE_DIR.mkdir(parents=True, exist_ok=True)
    out_md = WIRE_DIR / f"{stem}.gam_params.md"

    seen: set[tuple[int, str]] = set()
    rows: list[dict] = []
    parse_errors = 0
    for s in strs:
        v = s["value"]
        if "CompileTimeParameter" not in v:
            continue
        # ParameterCollection_<N> strings hold many CompileTimeParameter
        # entries inside them; iterate over all matches.
        matches = list(RE_CTP.finditer(v))
        if not matches:
            parse_errors += 1
            continue
        for m in matches:
            id_token, ns_chain, mid_section, decorator = m.group(1), m.group(2), m.group(3), m.group(4)
            id_value = decode_int_literal(id_token)
            if id_value is None:
                continue
            paramname, ns = _ns_to_canonical(ns_chain)
            type_str = _decode_type_token(mid_section)

            key = (id_value, ns)
            if key in seen:
                continue
            seen.add(key)

            rows.append({
                "id": id_value,
                "ns": ns,
                "paramname": paramname,
                "type": type_str,
                "decorator": decorator,
                "raw": v[:200],   # truncate to keep JSON small
                "source_rva_hex": f"0x{s['rva']:x}",
            })

    rows.sort(key=lambda r: (r["ns"], r["id"]))

    # JSON.
    out_json.write_text(json.dumps(rows, indent=2))

    # CSV.
    with out_csv.open("w", newline="") as f:
        wr = csv.DictWriter(f, fieldnames=["id", "ns", "paramname", "type", "decorator", "source_rva_hex"])
        wr.writeheader()
        for r in rows:
            row = {k: r[k] for k in wr.fieldnames}
            wr.writerow(row)

    # Markdown report grouped by namespace.
    by_ns: dict[str, list[dict]] = defaultdict(list)
    for r in rows:
        by_ns[r["ns"]].append(r)

    with out_md.open("w") as f:
        f.write(f"# {stem}.exe — GAM CompileTimeParameter registry\n\n")
        f.write(f"Auto-generated by `tools/extract_gam_params.py`. {len(rows)} unique\n")
        f.write(f"`(id, namespace)` parameters recovered from `.rdata` mangled\n")
        f.write(f"type names. Each row is one wire-serializable game-state\n")
        f.write(f"property; the *semantic* interpretation lives in\n")
        f.write(f"`project-meteor-server` / `garlemald-server` (where IDs map\n")
        f.write(f"to player health, quest flags, etc.).\n\n")
        f.write(f"Type codes: `signed char` = i8, `short` = i16, `int` = i32,\n")
        f.write(f"`float` = f32, `double` = f64. Most properties carry a single\n")
        f.write(f"primitive value via `Component::GAM::DecoratorSimpleAssign<T>`;\n")
        f.write(f"more complex shapes use other decorators.\n\n")
        for ns in sorted(by_ns):
            ns_rows = by_ns[ns]
            f.write(f"## `{ns}` — {len(ns_rows)} parameters\n\n")
            f.write("| id | type | decorator |\n")
            f.write("|---:|:---|:---|\n")
            for r in ns_rows:
                f.write(f"| {r['id']} | `{r['type']}` | `{r['decorator']}` |\n")
            f.write("\n")

    print(f"=== {stem}: {len(rows)} GAM params recovered ===")
    print(f"  parse errors: {parse_errors}")
    print(f"  by namespace:")
    for ns in sorted(by_ns):
        types = defaultdict(int)
        for r in by_ns[ns]:
            types[r["type"]] += 1
        type_summary = ", ".join(f"{t}={c}" for t, c in sorted(types.items()))
        print(f"    {ns}: {len(by_ns[ns])}  ({type_summary})")
    print()
    print(f"wrote: {out_json.relative_to(REPO_ROOT)}")
    print(f"       {out_csv.relative_to(REPO_ROOT)}")
    print(f"       {out_md.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
