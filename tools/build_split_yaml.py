#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Fold the Ghidra dumps for one binary into the per-function work pool YAML.

Reads:
  config/<binary>.symbols.json    (from DumpFunctions.java)
  config/<binary>.strings.json    (from DumpStrings.java)
  config/<binary>.rtti.json       (from DumpRtti.java)
  build/pe-layout/<binary>.json   (from extract_pe.py)

Writes:
  config/<binary>.yaml             — work pool: one row per function
  config/<binary>.middleware.json  — auto-classified middleware breakdown
                                     (count + bytes per category)

For each function we decide:
  - module guess           (from RTTI vtable hits + __FILE__ string proximity)
  - matching | functional | middleware-{miles,crt,stl,mfc,atl,dx9,...}
  - status: always `unmatched` on first generation

Heuristics (ordered, first match wins):

  1. RVA falls inside a non-`.text` executable section (e.g. MSSMIXER):
        → middleware-miles
  2. Function name matches a known CRT/STL/MFC/ATL pattern (regex list
     below):
        → middleware-{crt,stl,mfc,atl}
  3. Function is referenced by a recovered RTTI vtable for a known
     middleware class (CWnd, CObject, CMFCComObject, std::, ATL::):
        → middleware-mfc / middleware-stl / middleware-atl
  4. Otherwise:
        → matching   (the contributor work pool)

The function-name heuristic is intentionally conservative — better to
leave a few middleware functions in the pool than to silently drop
real game functions. A contributor can always reclassify a row by
hand.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
PE_LAYOUT = REPO_ROOT / "build" / "pe-layout"

# --- Middleware classifiers --------------------------------------------------

# Function-name regexes. Order matters; first match wins.
NAME_PATTERNS = [
    # MSVC C runtime
    (re.compile(r"^_?(__)?(security_(init_)?cookie|chkstk|alloca_probe|except_handler[24]?|report_(rangeerror|gsfailure)|EH_prolog\d?|GSHandlerCheck|CxxFrameHandler\d*|abi::__cxa_)"), "middleware-crt"),
    (re.compile(r"^_?(_initterm|atexit|_onexit|exit|_exit|abort|terminate|_purecall|_amsg_exit|raise|signal|setjmp|longjmp|_local_unwind\d*|_global_unwind\d*|_seh_(filter|longjmp_unwind))"), "middleware-crt"),
    (re.compile(r"^_?(memcpy|memset|memcmp|strcpy|strncpy|strcmp|strncmp|strlen|strcat|strncat|strchr|strstr|sprintf|swprintf|vsprintf|fprintf|printf|fwrite|fread|fopen|fclose|fseek|ftell|atoi|atol|itoa|ltoa)$"), "middleware-crt"),
    # MSVC C++ STL (Dinkumware)
    (re.compile(r"std::"), "middleware-stl"),
    (re.compile(r"^_(Char|Wide|Floor|Tidy|Yarn|Locinfo)"), "middleware-stl"),
    # MSVC MFC
    (re.compile(r"^C(Wnd|Object|Cmd|Frame|View|Dialog|Doc|String|Archive|File|Async|Bitmap|Brush|Button|Control|DC|Dialog|EditView|Font|GdiObject|Image|ListCtrl|Menu|MFCToolBar|Pen|Static|TabCtrl|TimeSpan|TreeCtrl|Win)\w*::"), "middleware-mfc"),
    (re.compile(r"^Afx[A-Z]"), "middleware-mfc"),
    (re.compile(r"^_AFX_"), "middleware-mfc"),
    # MSVC ATL
    (re.compile(r"^ATL::"), "middleware-atl"),
    (re.compile(r"^CMFCComObject"), "middleware-mfc"),  # MFC's COM bridge
    # zlib
    (re.compile(r"^(inflate|deflate|adler32|crc32|zError|zlibVersion|gz)\w*"), "middleware-zlib"),
    # Lua VM (we keep the lua_* layer as `middleware-lua` and decompile
    # only the game's own bindings around it)
    (re.compile(r"^(lua|luaL|luaB|luaO|luaG|luaS|luaT|luaV|luaH|luaE|luaC|luaK|luaP|luaX|luaY|luaZ|luaD|luaF|luaI)_"), "middleware-lua"),
    # Miles AIL
    (re.compile(r"^(AIL|MSS|ail|mss)_"), "middleware-miles"),
    # DirectX 9 thunks
    (re.compile(r"IDirect3D|D3DX|XAudio|DSound|DInput|IDirectInput|IDirectSound|IDirectPlay"), "middleware-dx9"),
]

VTABLE_PATTERNS = [
    (re.compile(r"^C(Wnd|Object|Cmd|Frame|View|Dialog|Doc|MFC|GdiObject|DC|Frame|MDIChildWnd|MDIFrame|TestCmdUI)"), "middleware-mfc"),
    (re.compile(r"^_AFX_"), "middleware-mfc"),
    (re.compile(r"^ATL::"), "middleware-atl"),
    (re.compile(r"std::"), "middleware-stl"),
    (re.compile(r"^exception$|^bad_alloc$|^bad_cast$|^type_info$"), "middleware-stl"),
]

# --- Module-guess heuristics ------------------------------------------------

# The Phase-1 RTTI dump on ffxivgame.exe revealed the project's namespaces:
#   SQEX::CDev::*           (2,145 vtables) — Square Enix's in-house "CDev"
#                                              engine (memory, framework,
#                                              renderer, layout, animation)
#   Application::Scene::*   (824)            — actor / scene graph / VFX
#   Application::Main::*    (495)            — main loop / window containers
#   Application::Misc::*    (21+)            — RaptureFontModule,
#                                              RaptureSupportModule,
#                                              RaptureLogModule, etc.
#   Application::Lua::*     (238)            — Lua bindings (game side)
#   Component::Lua::*       (135)            — Lua VM bindings (engine side)
#   Sqwt::*                 (~600)           — "Square Window Toolkit" — the
#                                              WPF/XAML-style UI framework
#                                              (Controls, Markup, Input,
#                                              Threading, EventHandler<T>)
#   Sqex::*                 (~200)           — Sqex utility namespace
#                                              (Misc, Data, Input, Socket,
#                                              Http, File, Thread)
#   Component::*            — per-feature subsystems (GAM, Network, Font,
#                              Patch, Excel, Install, Resource, Sound, Text,
#                              Xml)
#   GRC                     — graphics resources
#
# Source-tree paths (from __FILE__ literals):
#   D:/rapture/src/Application/Rapture/source/...   <- game side
#   c:/work/project/cdev/src/{dw,lay,common}/...    <- engine side
#
# Order matters; first match wins.
MODULE_HINTS = [
    # --- Network (highest priority — Phase 3 unblocks garlemald-server) ----
    (re.compile(r"Application::Network|Component::Network|Sqex::Socket|Sqex::Http|ChannelManager|Packet$|Blowfish|\bNet\b", re.IGNORECASE), "net"),
    # --- Sqpack / ZiPatch / Excel (Phase 4 file-format unblock) -----------
    (re.compile(r"Component::Patch|ZiPatch|Sqpack"), "sqpack"),
    (re.compile(r"Component::Excel|::Excel"), "excel"),
    (re.compile(r"Component::Resource|Sqex::File|Sqex::Data|ResourceInterface|Cib(?:Resource)?Binder"), "resource"),
    # --- Game scene / battle / quest / director (Phase 5–6) ---------------
    (re.compile(r"Director"), "director"),
    (re.compile(r"Quest|Event\b"), "quest"),
    (re.compile(r"WeatherManager|Application::Scene::.*Weather|Weather"), "weather"),
    (re.compile(r"Damage|Hit|Crit|Status|Combo|Regimen|BattleCommand|BattleEffect"), "battle"),
    (re.compile(r"Application::Scene::.*Actor|RaptureActor|BattleNpc|\bMob\b|\bNpc\b|Aetheryte"), "actor"),
    (re.compile(r"Inventory|InventoryItem|Shop"), "inventory"),
    (re.compile(r"RaptureWorld|Application::IRapture|Application::Rapture\b|Zone\b|Map\b|Region\b|World\b"), "world"),
    # --- Lua bindings (Phase 6) -------------------------------------------
    (re.compile(r"Application::Lua|Component::Lua|LuaPlayer|LuaQuest|LuaActor"), "lua"),
    # --- UI / Sqwt --------------------------------------------------------
    (re.compile(r"^Sqwt::|RaptureUserControl|Sqwt::Markup|Sqwt::Controls|Sqwt::Input|Sqwt::Media|Sqwt::Threading"), "ui"),
    (re.compile(r"\bUI\b|HUD|Menu|Window\b"), "ui"),
    # --- Renderer / VFX / fonts ------------------------------------------
    (re.compile(r"GRC::|MaterialManager|Application::Scene::Vfx|RaptureQixControl|D3D9|D3DX|RenderInterface|Render|Draw|Graphics"), "render"),
    (re.compile(r"Component::Font|RaptureFontModule|Component::Text"), "font"),
    (re.compile(r"Application::Scene::Cut|Cinematic|\bClip\b"), "cinematic"),
    (re.compile(r"Component::Sound|RaptureSoundModule|Audio|Music|BGM"), "audio"),
    # --- Engine layer (large, low-priority work pool) ---------------------
    (re.compile(r"^SQEX::CDev::Engine::Memory|MemoryAllocator"), "engine_memory"),
    (re.compile(r"^SQEX::CDev::Engine::Lay::Stella|LayoutMemory"), "engine_layout"),
    (re.compile(r"^SQEX::CDev::Engine::Fw::Framework|InitialConfiguration"), "engine_framework"),
    (re.compile(r"^SQEX::CDev::Engine::Animation"), "engine_animation"),
    (re.compile(r"^SQEX::CDev::"), "engine_cdev"),
    # --- Sqex / Component catch-alls ---------------------------------------
    (re.compile(r"^Component::GAM"), "engine_gam"),
    (re.compile(r"^Component::Install"), "install"),
    (re.compile(r"^Component::Xml"), "xml"),
    (re.compile(r"^Sqex::Misc"), "sqex_misc"),
    (re.compile(r"^Sqex::Thread|^Sqex::Input"), "sqex_misc"),
    # --- Game-side core threading / scheduling ----------------------------
    (re.compile(r"SceneThread|MainThread|GameThread"), "core"),
    (re.compile(r"^Application::Misc::Rapture(Log|Support|Macro|String)"), "core"),
]


def classify_function(
    fn: dict,
    rtti_by_class: dict[str, str],
    fn_to_class: dict[int, str],
) -> tuple[str, str]:
    """Return (tier, module). tier ∈ matching | middleware-*."""
    name = fn["name"]
    rva = fn["rva"]

    # Section first — anything in MSSMIXER is statically-linked Miles.
    if fn.get("section", "") == "MSSMIXER":
        return ("middleware-miles", "_skip")

    # Function-name regex (catches CRT/STL/MFC/zlib that demangle cleanly).
    for pat, tier in NAME_PATTERNS:
        if pat.search(name):
            module = guess_module(name) or "_middleware"
            return (tier, module)

    # Vtable-slot reverse lookup: if this function's RVA appears in any
    # vtable, the class that owns the vtable is the strongest signal we
    # have for the function's purpose. Use it for module classification
    # AND tier (some classes are middleware).
    cls = fn_to_class.get(rva)
    if cls is not None:
        # First check if the class itself looks like middleware.
        for pat, tier in VTABLE_PATTERNS:
            if pat.search(cls):
                return (tier, guess_module(cls) or "_middleware")
        # Otherwise, classify by namespace.
        return ("matching", guess_module(cls) or _module_from_namespace(cls))

    # Vtable substring match on function name (rare but happens for thunks).
    for cls_key in rtti_by_class:
        if cls_key in name:
            for pat, tier in VTABLE_PATTERNS:
                if pat.search(cls_key):
                    return (tier, guess_module(cls_key) or "_middleware")
            return ("matching", guess_module(cls_key) or _module_from_namespace(cls_key))

    return ("matching", guess_module(name) or "_unknown")


def _module_from_namespace(cls: str) -> str:
    """Coarse module by top-level namespace when no MODULE_HINTS pattern matches."""
    if cls.startswith("SQEX::CDev::"):
        return "engine_cdev"
    if cls.startswith("Application::Scene"):
        return "scene"
    if cls.startswith("Application::Main"):
        return "ui"
    if cls.startswith("Application::Misc"):
        return "core"
    if cls.startswith("Application::Lua") or cls.startswith("Component::Lua"):
        return "lua"
    if cls.startswith("Application::Network") or cls.startswith("Component::Network"):
        return "net"
    if cls.startswith("Application::"):
        return "_application"
    if cls.startswith("Sqwt::"):
        return "ui"
    if cls.startswith("Sqex::Socket") or cls.startswith("Sqex::Http"):
        return "net"
    if cls.startswith("Sqex::"):
        return "sqex_misc"
    if cls.startswith("Component::Patch"):
        return "sqpack"
    if cls.startswith("Component::Excel"):
        return "excel"
    if cls.startswith("Component::Sound"):
        return "audio"
    if cls.startswith("Component::Font") or cls.startswith("Component::Text"):
        return "font"
    if cls.startswith("Component::Resource") or cls.startswith("Component::Xml"):
        return "resource"
    if cls.startswith("Component::"):
        return "_component"
    return "_unknown_class"


def guess_module(name: str) -> str | None:
    for pat, mod in MODULE_HINTS:
        if pat.search(name):
            return mod
    return None


def yaml_str(s: str) -> str:
    """Minimal YAML scalar quoter — quote anything with ':' or weirdness."""
    if re.match(r"^[A-Za-z0-9_./-]+$", s):
        return s
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", help="binary stem (without .exe), e.g. ffxivgame")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    sym_path = CONFIG / f"{stem}.symbols.json"
    rtti_path = CONFIG / f"{stem}.rtti.json"
    slots_path = CONFIG / f"{stem}.vtable_slots.jsonl"
    if not sym_path.exists() or not rtti_path.exists():
        print(f"error: missing dumps for {stem}; run import_to_ghidra.py first", file=sys.stderr)
        return 1

    fns = json.loads(sym_path.read_text())
    rtti = json.loads(rtti_path.read_text())
    rtti_by_class: dict[str, str] = {r["class"]: r["vtable_symbol"] for r in rtti}

    # Vtable slot map: fn_rva → class. Multiple classes may own the same
    # function (multiple inheritance, vtable overlap); we take the most
    # specific (longest fully-qualified name).
    fn_to_class: dict[int, str] = {}
    if slots_path.exists():
        for line in slots_path.read_text().splitlines():
            if not line.strip():
                continue
            entry = json.loads(line)
            rva = entry["fn_rva"]
            cls = entry["class"]
            existing = fn_to_class.get(rva)
            if existing is None or len(cls) > len(existing):
                fn_to_class[rva] = cls
        print(f"  vtable slots: {len(fn_to_class)} functions class-mapped")

    fns.sort(key=lambda f: f["rva"])

    out_yaml = CONFIG / f"{stem}.yaml"
    out_mw = CONFIG / f"{stem}.middleware.json"

    tier_stats: dict[str, dict] = {}
    module_stats: dict[str, dict] = {}

    with out_yaml.open("w") as f:
        f.write("# meteor-decomp work-pool — auto-generated by tools/build_split_yaml.py\n")
        f.write(f"# Source binary: {stem}.exe\n")
        f.write(f"# Function count: {len(fns)}\n")
        f.write("# Schema: see PLAN.md §6 / docs/matching-workflow.md\n\n")
        for fn in fns:
            tier, module = classify_function(fn, rtti_by_class, fn_to_class)
            size = int(fn.get("size", 0))
            tier_stats.setdefault(tier, {"count": 0, "bytes": 0})
            tier_stats[tier]["count"] += 1
            tier_stats[tier]["bytes"] += size
            module_stats.setdefault(module, {"count": 0, "bytes": 0})
            module_stats[module]["count"] += 1
            module_stats[module]["bytes"] += size

            status = "matched" if tier.startswith("middleware-") else "unmatched"
            f.write(f"- rva: 0x{fn['rva']:08x}\n")
            f.write(f"  end: 0x{fn['rva'] + size:08x}\n")
            f.write(f"  size: 0x{size:x}\n")
            f.write(f"  module: {yaml_str(module)}\n")
            f.write(f"  symbol: {yaml_str(fn['name'])}\n")
            f.write(f"  type: {tier}\n")
            f.write(f"  status: {status}\n")
            f.write(f"  owner: null\n")
            f.write(f"  section: {fn.get('section','?')}\n")

    summary = {
        "binary": stem,
        "function_count": len(fns),
        "tiers": tier_stats,
        "modules": module_stats,
    }
    out_mw.write_text(json.dumps(summary, indent=2))

    print(f"=== {stem}: {len(fns)} functions ===")
    print(f"\n  tiers:")
    for k, v in sorted(tier_stats.items(), key=lambda kv: -kv[1]["bytes"]):
        print(f"    {k:24s}  count={v['count']:>6d}  bytes={v['bytes']:>10,}")
    print(f"\n  top modules (top 12):")
    top = sorted(module_stats.items(), key=lambda kv: -kv[1]["bytes"])[:12]
    for k, v in top:
        print(f"    {k:24s}  count={v['count']:>6d}  bytes={v['bytes']:>10,}")
    print(f"\nwrote: {out_yaml}")
    print(f"       {out_mw}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
