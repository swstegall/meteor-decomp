// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Emit ONLY config/<binary>.symbols.json (RVA → name + size + section) for
// currentProgram. A symbols-only variant of DumpFunctions.java that does NOT
// write the per-function asm/<binary>/*.s files — used after a Function ID
// pass to refresh just the symbol names, and to side-step a broken asm/ dir.
// Output format is byte-for-byte the same as DumpFunctions' symbols.json.
//
// Needs METEOR_DECOMP_ROOT in the environment (the launcher sets it).
//
//@category meteor-decomp

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Program;
import ghidra.program.model.mem.MemoryBlock;

public class DumpSymbolsOnly extends GhidraScript {

	@Override
	public void run() throws Exception {
		Program prog = currentProgram;
		long imageBase = prog.getImageBase().getOffset();

		String repoRoot = System.getenv("METEOR_DECOMP_ROOT");
		if (repoRoot == null) {
			repoRoot = new File(getScriptArgs().length > 0 ? getScriptArgs()[0] : ".").getAbsolutePath();
		}
		String binary = prog.getName().toLowerCase().replace(".exe", "");
		File configDir = new File(repoRoot, "config");
		configDir.mkdirs();

		FunctionIterator iter = prog.getFunctionManager().getFunctions(true);
		File symbolsJson = new File(configDir, binary + ".symbols.json");
		PrintWriter sym = new PrintWriter(new FileWriter(symbolsJson));
		sym.println("[");
		boolean first = true;
		int count = 0;

		while (iter.hasNext() && !monitor.isCancelled()) {
			Function fn = iter.next();
			if (fn.isExternal()) {
				continue;
			}
			Address entry = fn.getEntryPoint();
			long rva = entry.getOffset() - imageBase;
			String name = fn.getName();
			AddressSetView body = fn.getBody();
			long size = 0;
			for (AddressRange r : body.getAddressRanges()) {
				size += r.getLength();
			}
			String section = sectionOf(prog, entry);
			if (!first) {
				sym.println(",");
			}
			first = false;
			sym.printf("  {\"rva\": %d, \"rva_hex\": \"0x%x\", \"name\": %s, \"size\": %d, \"section\": %s}",
				rva, rva, jsonStr(name), size, jsonStr(section));
			count++;
		}
		sym.println();
		sym.println("]");
		sym.close();
		println(String.format("DumpSymbolsOnly: %s — %d functions → %s", binary, count, symbolsJson));
	}

	private static String sectionOf(Program prog, Address addr) {
		MemoryBlock blk = prog.getMemory().getBlock(addr);
		return blk == null ? "?" : blk.getName();
	}

	private static String jsonStr(String s) {
		StringBuilder sb = new StringBuilder("\"");
		for (int i = 0; i < s.length(); i++) {
			char c = s.charAt(i);
			switch (c) {
				case '"':  sb.append("\\\""); break;
				case '\\': sb.append("\\\\"); break;
				case '\n': sb.append("\\n"); break;
				case '\r': sb.append("\\r"); break;
				case '\t': sb.append("\\t"); break;
				default:
					if (c < 0x20) sb.append(String.format("\\u%04x", (int) c));
					else sb.append(c);
			}
		}
		sb.append('"');
		return sb.toString();
	}
}
