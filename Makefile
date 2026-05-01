# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later

PY      ?= python3
ORIG    := orig
BUILD   := build
TOOLS   := tools

# --- Phase 0 (works today) ---------------------------------------------

.PHONY: help bootstrap pe-info clean

help:
	@echo "meteor-decomp — top-level targets"
	@echo "  make bootstrap            symlink orig/ + dump PE structure (Phase 0)"
	@echo "  make pe-info              re-run tools/extract_pe.py (Phase 0)"
	@echo "  make split BINARY=X.exe   Phase 1: Ghidra import + dump + work-pool YAML"
	@echo "  make split-all            Phase 1: split every binary in orig/"
	@echo "  make setup-msvc           Phase 2: verify VS 2005 SP1 + Wine setup"
	@echo "  make find-rosetta         Phase 2: pick best Rosetta-Stone candidate"
	@echo "  make rosetta              Phase 2: compile + diff staged Rosetta function"
	@echo "  make extract-net          Phase 3: net-class vtable → fn_rva map"
	@echo "  make extract-gam          Phase 3: GAM property registry (id → type)"
	@echo "  make emit-gam-header      Phase 3: include/net/gam_registry.h from GAM"
	@echo "  make extract-paramnames   Phase 3: dereference per-class PARAMNAME dispatchers"
	@echo "  make validate-chara-make  Phase 3: garlemald chara_info.rs ↔ GAM CharaMakeData diff"
	@echo "  make diff FUNC=X          objdiff-cli on one matched function"
	@echo "  make progress             print matched/total across all *.yaml"
	@echo "  make clean                wipe build/"

bootstrap:
	@$(TOOLS)/symlink_orig.sh
	@$(PY) $(TOOLS)/extract_pe.py

pe-info:
	@$(PY) $(TOOLS)/extract_pe.py

clean:
	rm -rf $(BUILD)

# --- Phase 1 (TODO once Ghidra is wired) -------------------------------

.PHONY: split split-all

# Single-binary split. Usage: make split BINARY=ffxivgame.exe
split:
	@if [ -z "$(BINARY)" ]; then echo "usage: make split BINARY=<binary>.exe"; exit 1; fi
	mkdir -p $(BUILD)/logs
	$(PY) $(TOOLS)/import_to_ghidra.py $(BINARY) 2>&1 | tee $(BUILD)/logs/$(basename $(BINARY)).import.log
	$(PY) $(TOOLS)/build_split_yaml.py $(basename $(BINARY))

split-all:
	@for exe in $(ORIG)/*.exe; do \
	    bn=$$(basename $$exe); \
	    $(MAKE) split BINARY=$$bn || exit $$?; \
	done

# --- Phase 2 (TODO once MSVC is wired) ---------------------------------

.PHONY: setup-msvc find-rosetta rosetta diff progress extract-net extract-gam emit-gam-header extract-paramnames validate-chara-make

# Walk the RTTI dump for net-relevant classes; emit class→slot→fn_rva map.
extract-net:
	$(PY) $(TOOLS)/extract_net_vtables.py $(or $(BINARY),ffxivgame)

# Parse Component::GAM::CompileTimeParameter mangled types from .rdata
# strings; emit the structured (id, namespace, type, decorator) registry.
extract-gam:
	$(PY) $(TOOLS)/extract_gam_params.py $(or $(BINARY),ffxivgame)

# Emit include/net/gam_registry.h from the GAM extraction.
emit-gam-header: extract-gam
	$(PY) $(TOOLS)/emit_gam_header.py $(or $(BINARY),ffxivgame)

# Resolve PARAMNAME_<id> string pointers from each Data class's
# MetadataProvider dispatcher (currently CharaMakeData; extend the
# DISPATCHERS dict in the script to cover Player / etc. as their
# dispatchers are identified). Enriches config/<bin>.gam_params.json
# in-place with a `paramname` field per entry.
extract-paramnames:
	$(PY) $(TOOLS)/extract_paramnames_dispatch.py $(or $(BINARY),ffxivgame)

# Cross-validate garlemald-server's chara_info.rs parser flow against
# the GAM CharaMakeData schema.
validate-chara-make: extract-gam extract-paramnames
	$(PY) $(TOOLS)/validate_chara_make.py $(or $(BINARY),ffxivgame)


# Run the setup checks: wine + MSVC_TOOLCHAIN_DIR + cl.exe + objdiff.
setup-msvc:
	$(TOOLS)/setup-msvc.sh

# Pick the best Rosetta-Stone candidate function. Output:
#   build/rosetta/ffxivgame.candidates.json
#   build/rosetta/ffxivgame.top.txt
find-rosetta:
	$(PY) $(TOOLS)/find_rosetta.py $(or $(BINARY),ffxivgame)

# Compile + diff the staged Rosetta source against the original binary.
# The candidate's .cpp lives at src/ffxivgame/_rosetta/<sym>.cpp.
ROSETTA_FLAGS ?= /c /O2 /Oy /GR /EHsc /Gy /GS /MT /Zc:wchar_t /Zc:forScope /TP
rosetta: setup-msvc
	@if ! ls src/ffxivgame/_rosetta/*.cpp >/dev/null 2>&1; then \
	    echo "no rosetta source staged in src/ffxivgame/_rosetta/"; \
	    echo "run 'make find-rosetta' and hand-translate the top candidate."; \
	    exit 1; \
	fi
	mkdir -p $(BUILD)/obj/_rosetta
	@for cpp in src/ffxivgame/_rosetta/*.cpp; do \
	    name=$$(basename $$cpp .cpp); \
	    obj=$(BUILD)/obj/_rosetta/$$name.obj; \
	    echo ">>> cl $$cpp -> $$obj"; \
	    $(TOOLS)/cl-wine.sh $(ROSETTA_FLAGS) /Fo$$(./tools/cl-wine.sh --winpath $$obj 2>/dev/null || echo $$obj) $$cpp; \
	    echo ">>> objdiff $$obj vs orig"; \
	    $(PY) $(TOOLS)/compare.py FUNC=$$name; \
	done

diff:
	@if [ -z "$(FUNC)" ]; then echo "usage: make diff FUNC=Symbol::Name"; exit 1; fi
	$(PY) $(TOOLS)/compare.py FUNC=$(FUNC)

progress:
	$(PY) $(TOOLS)/progress.py
