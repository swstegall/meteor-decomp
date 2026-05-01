# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Ghidra-Jython post-analysis script. Run via tools/import_to_ghidra.py.
#
# Walks every function in the program and writes:
#   asm/<binary>/<rva>_<symbol>.s     — disassembly per function
#   config/<binary>.symbols.json      — RVA → name + size + section
#
# STUB. Sketch only — flesh out in Phase 1 once Ghidra + the rest of
# the headless pipeline are running locally. The shape here matches
# the LEGO Island decomp's `dumpfns.py` and is intended as a direct
# cue for the contributor implementing Phase 1.
#
# Pseudocode:
#
#   import json, os
#   from ghidra.program.model.listing import Function
#   from ghidra.app.decompiler import DecompInterface
#
#   prog = currentProgram
#   binary = prog.getName().lower().replace(".exe", "")
#   asm_dir = os.path.join(REPO_ROOT, "asm", binary)
#   os.makedirs(asm_dir, exist_ok=True)
#
#   syms = []
#   fnmgr = prog.getFunctionManager()
#   for fn in fnmgr.getFunctions(True):
#       rva = fn.getEntryPoint().getOffset() - prog.getImageBase().getOffset()
#       name = fn.getName()
#       size = sum(b.getMaxAddress().subtract(b.getMinAddress()) + 1
#                  for b in fn.getBody().getAddressRanges())
#       syms.append({"rva": rva, "name": name, "size": size,
#                    "section": <determine from address>})
#       safe = sanitize(name)
#       path = os.path.join(asm_dir, f"{rva:08x}_{safe}.s")
#       with open(path, "w") as f:
#           f.write(disassemble(fn))   # walk InstructionIterator
#
#   with open(os.path.join(REPO_ROOT, "config",
#             f"{binary}.symbols.json"), "w") as f:
#       json.dump(syms, f, indent=2)

raise NotImplementedError(
    "dump_functions.py is a Phase 1 stub — see PLAN.md §6 Phase 1"
)
