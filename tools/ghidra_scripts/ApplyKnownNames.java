// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Apply a JSON list of {rva, name} entries onto the functions in the current
// program — the mechanism for pushing meteor-decomp's accumulated names
// (config/<bin>.name_overrides.json: FFXIVLegacyClientStructs ctors +
// ffxivDecomp names; and the generated vtable-method names) into a freshly
// re-imported Ghidra project so the disassembly is navigable by real names.
//
// Input JSON is a list of objects with at least `rva` (int, offset from the
// program image base) and `name` (the symbol to apply). `rva_hex` is accepted
// as a fallback. Names containing "::" are applied verbatim (Ghidra keeps them
// as readable flat symbols). Only functions are renamed; an entry with no
// function at its address gets a primary label instead so the name still shows.
//
// Path comes from env var APPLY_NAMES_JSON (one or more, ';'-separated), else
// defaults to config/<program-name>.name_overrides.json under
// METEOR_DECOMP_ROOT. Idempotent and re-runnable.
//
//@category meteor-decomp

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.SymbolTable;

public class ApplyKnownNames extends GhidraScript {

	// Minimal object scanner: pull "rva", "rva_hex" and "name" out of each
	// top-level {...} object in the JSON array. Avoids a JSON-lib dependency,
	// matching the other meteor-decomp post-scripts' hand-rolled parsing.
	private static final Pattern OBJ = Pattern.compile("\\{[^{}]*\\}");
	private static final Pattern RVA = Pattern.compile("\"rva\"\\s*:\\s*(\\d+)");
	private static final Pattern RVA_HEX =
		Pattern.compile("\"rva_hex\"\\s*:\\s*\"0x([0-9a-fA-F]+)\"");
	private static final Pattern NAME =
		Pattern.compile("\"name\"\\s*:\\s*\"((?:[^\"\\\\]|\\\\.)*)\"");

	@Override
	public void run() throws Exception {
		String root = System.getenv("METEOR_DECOMP_ROOT");
		if (root == null || root.isEmpty()) {
			root = Paths.get("").toAbsolutePath().toString();
		}
		String env = System.getenv("APPLY_NAMES_JSON");
		String[] files;
		if (env != null && !env.isEmpty()) {
			files = env.split(";");
		} else {
			String bin = currentProgram.getName().replaceAll("\\.exe$", "");
			files = new String[] {
				Paths.get(root, "config", bin + ".name_overrides.json").toString()
			};
		}

		long imageBase = currentProgram.getImageBase().getOffset();
		SymbolTable symtab = currentProgram.getSymbolTable();
		int applied = 0, labelled = 0, missing = 0, unchanged = 0, failed = 0;

		for (String f : files) {
			Path p = Paths.get(f.trim());
			if (!Files.exists(p)) {
				println("ApplyKnownNames: SKIP (missing) " + p);
				continue;
			}
			String json = new String(Files.readAllBytes(p));
			Matcher om = OBJ.matcher(json);
			while (om.find()) {
				String obj = om.group();
				Matcher nm = NAME.matcher(obj);
				if (!nm.find()) {
					continue;
				}
				String name = nm.group(1);

				long rva = -1;
				Matcher rm = RVA.matcher(obj);
				if (rm.find()) {
					rva = Long.parseLong(rm.group(1));
				} else {
					Matcher rh = RVA_HEX.matcher(obj);
					if (rh.find()) {
						rva = Long.parseLong(rh.group(1), 16);
					}
				}
				if (rva < 0) {
					continue;
				}

				Address addr;
				try {
					addr = toAddr(imageBase + rva);
				} catch (Exception e) {
					failed++;
					continue;
				}

				Function fn = getFunctionAt(addr);
				try {
					if (fn != null) {
						if (name.equals(fn.getName())) {
							unchanged++;
						} else {
							fn.setName(name, SourceType.USER_DEFINED);
							applied++;
						}
					} else {
						symtab.createLabel(addr, name, SourceType.USER_DEFINED);
						labelled++;
					}
				} catch (Exception e) {
					failed++;
				}
			}
			println("ApplyKnownNames: processed " + p.getFileName());
		}
		println("ApplyKnownNames DONE: renamed=" + applied + " labelled=" + labelled
			+ " already-named=" + unchanged + " no-func=" + missing
			+ " failed=" + failed);
	}
}
