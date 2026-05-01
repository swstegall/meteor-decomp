# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Ghidra post-analysis script. Run Ghidra's RTTI analyser (it ships
# with a Microsoft RTTI plugin) and serialise the recovered class
# hierarchy + vtables to `config/rtti.json`.
#
# STUB. Phase 1 deliverable. Pseudocode:
#
#   import json
#   from ghidra.program.model.symbol import SymbolType
#
#   classes = []
#   sym_table = currentProgram.getSymbolTable()
#   for sym in sym_table.getSymbols(SymbolType.CLASS):
#       cls = {"name": sym.getName(True), "address": sym.getAddress().getOffset()}
#       # Walk vtable: scan from `??_R4` complete object locator backwards
#       # for the function pointer array; emit each slot's RVA + name.
#       cls["vtable"] = walk_vtable(sym)
#       cls["bases"] = walk_base_classes(sym)
#       classes.append(cls)
#
#   with open(os.path.join(REPO_ROOT, "config", "rtti.json"), "w") as f:
#       json.dump(classes, f, indent=2)
#
# Project Meteor / Seventh Umbral class names are matched against the
# discovered RTTI names so the existing C# / C++ symbol nomenclature
# becomes the meteor-decomp naming convention.

raise NotImplementedError("dump_rtti.py is a Phase 1 stub")
