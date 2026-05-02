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
	@echo "  make rosetta-bulk         Phase 2: like rosetta, but never bails — for stamped clusters"
	@echo "  make extract-net          Phase 3: net-class vtable → fn_rva map"
	@echo "  make extract-gam          Phase 3: GAM property registry (id → type)"
	@echo "  make emit-gam-header      Phase 3: include/net/gam_registry.h from GAM"
	@echo "  make extract-paramnames   Phase 3: dereference per-class PARAMNAME dispatchers"
	@echo "  make extract-gam-types-rtti Phase 3: enrich gam_params.json with RTTI types"
	@echo "  make validate-chara-make  Phase 3: garlemald parse_new_char_request ↔ GAM CharaMakeData"
	@echo "  make validate-chara-list  Phase 3: garlemald build_for_chara_list ↔ GAM ClientSelectData"
	@echo "  make validate-murmur2     Phase 3: MurmurHash2 vectors (FUN_00d31490 ↔ garlemald)"
	@echo "  make extract-opcodes      Phase 3: opcode → vtable-slot map (zone/lobby/chat Down)"
	@echo "  make extract-up-opcodes   Phase 3: Up-direction CPB ctor inventory + RX-opcode validation"
	@echo "  make extract-crypt-engine Phase 3: decode LobbyCryptEngine 9 slots + validate Blowfish init"
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

.PHONY: setup-msvc find-rosetta rosetta diff progress extract-net extract-gam extract-gam-types-rtti emit-gam-header extract-paramnames validate-chara-make validate-chara-list validate-murmur2 extract-opcodes extract-up-opcodes extract-crypt-engine

# Walk the RTTI dump for net-relevant classes; emit class→slot→fn_rva map.
extract-net:
	$(PY) $(TOOLS)/extract_net_vtables.py $(or $(BINARY),ffxivgame)

# Parse Component::GAM::CompileTimeParameter mangled types from .rdata
# strings; emit the structured (id, namespace, type, decorator) registry.
extract-gam:
	$(PY) $(TOOLS)/extract_gam_params.py $(or $(BINARY),ffxivgame)

# Emit include/net/gam_registry.h from the GAM extraction. Pulls
# extract-paramnames first so the header carries property names.
emit-gam-header: extract-gam extract-paramnames
	$(PY) $(TOOLS)/emit_gam_header.py $(or $(BINARY),ffxivgame)

# Resolve PARAMNAME_<id> string pointers from each Data class's
# MetadataProvider dispatcher (currently CharaMakeData; extend the
# DISPATCHERS dict in the script to cover Player / etc. as their
# dispatchers are identified). Enriches config/<bin>.gam_params.json
# in-place with a `paramname` field per entry.
extract-paramnames:
	$(PY) $(TOOLS)/extract_paramnames_dispatch.py $(or $(BINARY),ffxivgame)

# Augment gam_params.json with `rtti_type` parsed from the
# CompileTimeParameter template-parameter signature in the binary's
# RTTI. This is ground truth and corrects the legacy extractor's
# off-by-one Array sizes (e.g. Array<int,4> vs reported int[3]).
extract-gam-types-rtti: extract-gam extract-net
	$(PY) $(TOOLS)/extract_gam_types_rtti.py $(or $(BINARY),ffxivgame)

# Cross-validate garlemald-server's chara_info.rs parser flow against
# the GAM CharaMakeData schema.
validate-chara-make: extract-gam extract-paramnames extract-gam-types-rtti
	$(PY) $(TOOLS)/validate_chara_make.py $(or $(BINARY),ffxivgame)

# Cross-validate garlemald's build_for_chara_list against GAM
# ClientSelectData (schema-level — not byte-layout).
validate-chara-list: extract-gam extract-paramnames extract-gam-types-rtti
	$(PY) $(TOOLS)/validate_chara_list.py $(or $(BINARY),ffxivgame)

# Walk the binary's Down dispatchers (zone/lobby/chat) and emit
# opcode → vtable-slot map cross-referenced with garlemald opcodes.rs.
extract-opcodes:
	$(PY) $(TOOLS)/extract_opcode_dispatch.py $(or $(BINARY),ffxivgame)

# Up-direction (client → server) opcode reconnaissance — locates CPB
# constructors and validates that all garlemald OP_RX_* values appear in
# .text as PUSH immediates.
extract-up-opcodes:
	$(PY) $(TOOLS)/extract_up_opcodes.py $(or $(BINARY),ffxivgame)

# Decode LobbyCryptEngine's 9 vtable slots, verify the Blowfish P/S
# init tables are canonical OpenSSL pi-derived, and cross-check
# garlemald-server/common/src/blowfish_tables.rs byte-for-byte.
extract-crypt-engine:
	$(PY) $(TOOLS)/extract_crypt_engine.py $(or $(BINARY),ffxivgame)

# Compute MurmurHash2 test vectors (Python port of FUN_00d31490).
# Cross-check against garlemald's murmur_hash2; see docs/murmur2.md.
validate-murmur2:
	$(PY) $(TOOLS)/validate_murmur2.py


# Run the setup checks: wine + MSVC_TOOLCHAIN_DIR + cl.exe + objdiff.
setup-msvc:
	$(TOOLS)/setup-msvc.sh

# Pick the best Rosetta-Stone candidate function. Output:
#   build/rosetta/ffxivgame.candidates.json
#   build/rosetta/ffxivgame.top.txt
find-rosetta:
	$(PY) $(TOOLS)/find_rosetta.py $(or $(BINARY),ffxivgame)

# Compile + diff the staged Rosetta source against the original binary.
# The candidate's .cpp lives at src/<binary>/_rosetta/<sym>.cpp.
# Pass BINARY=ffxivboot.exe (or any other split binary) to retarget.
ROSETTA_FLAGS ?= /c /O2 /Oy /GR /EHsc /Gy /GS /MT /Zc:wchar_t /Zc:forScope /TP
ROSETTA_BIN_STEM = $(basename $(or $(BINARY),ffxivgame.exe))
ROSETTA_SRC_DIR = src/$(ROSETTA_BIN_STEM)/_rosetta
ROSETTA_OBJ_DIR = $(BUILD)/obj/_rosetta/$(ROSETTA_BIN_STEM)
rosetta: setup-msvc
	@if ! ls $(ROSETTA_SRC_DIR)/*.cpp >/dev/null 2>&1; then \
	    echo "no rosetta source staged in $(ROSETTA_SRC_DIR)/"; \
	    echo "run 'make find-rosetta BINARY=$(ROSETTA_BIN_STEM).exe' and hand-translate the top candidate,"; \
	    echo "or use tools/seed_templates.py to bootstrap canonical stub templates from another binary."; \
	    exit 1; \
	fi
	mkdir -p $(ROSETTA_OBJ_DIR)
	@for cpp in $(ROSETTA_SRC_DIR)/*.cpp; do \
	    name=$$(basename $$cpp .cpp); \
	    obj=$(ROSETTA_OBJ_DIR)/$$name.obj; \
	    echo ">>> cl $$cpp -> $$obj"; \
	    $(TOOLS)/cl-wine.sh $(ROSETTA_FLAGS) /Fo$$obj $$cpp || exit $$?; \
	    echo ">>> objdiff $$obj vs orig"; \
	    $(PY) $(TOOLS)/compare.py BINARY=$(ROSETTA_BIN_STEM).exe FUNC=$$name || exit $$?; \
	done

# Like `rosetta` but never bails on a non-GREEN verdict — useful for
# bulk validation across hundreds of stamped cluster siblings, where a
# single PARTIAL or MISMATCH shouldn't stop the whole sweep. The
# verdict per file is still printed; aggregate counts come from the
# log via `grep -c GREEN/PARTIAL/MISMATCH`. The tally is also written
# to build/logs/rosetta_bulk_summary_<binary>.txt for quick review.
.PHONY: rosetta-bulk
rosetta-bulk: setup-msvc
	@if ! ls $(ROSETTA_SRC_DIR)/*.cpp >/dev/null 2>&1; then \
	    echo "no rosetta source staged in $(ROSETTA_SRC_DIR)/"; \
	    exit 1; \
	fi
	mkdir -p $(ROSETTA_OBJ_DIR) $(BUILD)/logs
	@green=0; partial=0; mismatch=0; cl_fail=0; \
	for cpp in $(ROSETTA_SRC_DIR)/*.cpp; do \
	    name=$$(basename $$cpp .cpp); \
	    obj=$(ROSETTA_OBJ_DIR)/$$name.obj; \
	    echo ">>> cl $$cpp -> $$obj"; \
	    if ! $(TOOLS)/cl-wine.sh $(ROSETTA_FLAGS) /Fo$$obj $$cpp; then \
	        cl_fail=$$((cl_fail + 1)); echo "  ⚠️  cl failed (continuing)"; continue; \
	    fi; \
	    echo ">>> objdiff $$obj vs orig"; \
	    if $(PY) $(TOOLS)/compare.py BINARY=$(ROSETTA_BIN_STEM).exe FUNC=$$name >/tmp/.rb.$$$$ 2>&1; then \
	        green=$$((green + 1)); grep -E "GREEN" /tmp/.rb.$$$$ | head -1; \
	    else \
	        rc=$$?; \
	        if grep -q PARTIAL /tmp/.rb.$$$$; then partial=$$((partial + 1)); \
	        else mismatch=$$((mismatch + 1)); fi; \
	        grep -E "PARTIAL|MISMATCH" /tmp/.rb.$$$$ | head -1; \
	    fi; \
	    rm -f /tmp/.rb.$$$$; \
	done; \
	echo; echo "rosetta-bulk[$(ROSETTA_BIN_STEM)] summary  GREEN=$$green  PARTIAL=$$partial  MISMATCH=$$mismatch  cl_failed=$$cl_fail"; \
	{ echo "GREEN=$$green"; echo "PARTIAL=$$partial"; echo "MISMATCH=$$mismatch"; echo "cl_failed=$$cl_fail"; } > $(BUILD)/logs/rosetta_bulk_summary_$(ROSETTA_BIN_STEM).txt

diff:
	@if [ -z "$(FUNC)" ]; then echo "usage: make diff FUNC=Symbol::Name"; exit 1; fi
	$(PY) $(TOOLS)/compare.py FUNC=$(FUNC)

progress:
	$(PY) $(TOOLS)/progress.py
