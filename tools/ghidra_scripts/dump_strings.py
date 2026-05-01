# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Ghidra post-analysis script. Walks all strings in `.rdata` (and any
# other read-only section) and writes them to `config/strings.json`
# keyed by RVA.
#
# STUB. Phase 1 deliverable. Pseudocode:
#
#   import json
#   from ghidra.program.model.listing import Data
#
#   strings = []
#   listing = currentProgram.getListing()
#   for d in listing.getDefinedData(True):
#       if d.hasStringValue():
#           rva = d.getAddress().getOffset() - currentProgram.getImageBase().getOffset()
#           val = d.getValue()
#           strings.append({"rva": rva, "len": len(val), "value": val})
#
#   with open(os.path.join(REPO_ROOT, "config", "strings.json"), "w") as f:
#       json.dump(strings, f, indent=2, ensure_ascii=False)
#
# Post-processing: tools/build_split_yaml.py grep-walks this file for
# patterns like:
#   - `c:\\dev\\ffxiv\\src\\<module>\\<file>.cpp`  → __FILE__ macros
#   - `<ClassName>::<MethodName>`                  → __FUNCTION__ macros
# and writes the discovered (rva → module/file/function) hints into
# config/seed-symbols.json.

raise NotImplementedError("dump_strings.py is a Phase 1 stub")
