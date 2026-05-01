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
// Walk every function in currentProgram and emit:
//   asm/<binary>/<rva>_<symbol>.s        — disassembly per function
//   config/<binary>.symbols.json         — RVA → name + size + section
//
// Invoked from headless via:
//   analyzeHeadless ... -postScript DumpFunctions.java
// with the binary's stem (e.g. "ffxivgame") passed as a script argument.
//
//@category meteor-decomp
//@menupath
//@toolbar

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.Program;
import ghidra.program.model.mem.MemoryBlock;

public class DumpFunctions extends GhidraScript {

    @Override
    public void run() throws Exception {
        Program prog = currentProgram;
        long imageBase = prog.getImageBase().getOffset();

        String repoRoot = System.getenv("METEOR_DECOMP_ROOT");
        if (repoRoot == null) {
            // Fall back to script's parent's parent (tools/ghidra_scripts/.. == tools/ ; tools/.. == repo)
            repoRoot = new File(getScriptArgs().length > 0 ? getScriptArgs()[0] : ".").getAbsolutePath();
        }

        String binary = prog.getName().toLowerCase().replace(".exe", "");
        File asmDir = new File(repoRoot, "asm/" + binary);
        asmDir.mkdirs();
        File configDir = new File(repoRoot, "config");
        configDir.mkdirs();

        Listing listing = prog.getListing();
        FunctionIterator iter = prog.getFunctionManager().getFunctions(true);

        File symbolsJson = new File(configDir, binary + ".symbols.json");
        PrintWriter sym = new PrintWriter(new FileWriter(symbolsJson));
        sym.println("[");
        boolean first = true;

        int count = 0;
        long totalBytes = 0;

        while (iter.hasNext() && !monitor.isCancelled()) {
            Function fn = iter.next();
            if (fn.isExternal() || fn.isThunk()) {
                // Thunks come back labelled as their target; we still emit them
                // because the call site needs an .obj to link against, but we
                // skip externals (they're imports from other DLLs).
                if (fn.isExternal()) continue;
            }

            Address entry = fn.getEntryPoint();
            long rva = entry.getOffset() - imageBase;
            String name = fn.getName();
            AddressSetView body = fn.getBody();
            long size = 0;
            for (AddressRange r : body.getAddressRanges()) {
                size += r.getLength();
            }
            totalBytes += size;
            count++;

            String section = sectionOf(prog, entry);
            String safe = sanitize(name);
            File asmFile = new File(asmDir, String.format("%08x_%s.s", rva, safe));
            try (PrintWriter asm = new PrintWriter(new FileWriter(asmFile))) {
                asm.printf("# function %s%n", name);
                asm.printf("# rva     0x%08x%n", rva);
                asm.printf("# size    0x%x (%d bytes)%n", size, size);
                asm.printf("# section %s%n", section);
                asm.println();
                Instruction insn = listing.getInstructionAt(entry);
                while (insn != null && body.contains(insn.getAddress())) {
                    long iRva = insn.getAddress().getOffset() - imageBase;
                    asm.printf("    %08x:  %-32s  %s%n", iRva, bytesHex(insn), insn.toString());
                    insn = insn.getNext();
                }
            }

            if (!first) sym.println(",");
            first = false;
            sym.printf("  {\"rva\": %d, \"rva_hex\": \"0x%x\", \"name\": %s, \"size\": %d, \"section\": %s}",
                rva, rva, jsonStr(name), size, jsonStr(section));
        }
        sym.println();
        sym.println("]");
        sym.close();

        println(String.format("DumpFunctions: %s — %d functions, %,d bytes total → %s",
            binary, count, totalBytes, asmDir));
    }

    private static String sectionOf(Program prog, Address addr) {
        MemoryBlock blk = prog.getMemory().getBlock(addr);
        return blk == null ? "?" : blk.getName();
    }

    private static String bytesHex(Instruction insn) {
        try {
            byte[] b = insn.getBytes();
            StringBuilder sb = new StringBuilder();
            for (byte x : b) sb.append(String.format("%02x ", x & 0xff));
            return sb.toString().trim();
        } catch (Exception e) {
            return "??";
        }
    }

    private static String sanitize(String name) {
        // Replace anything that isn't safe in a filename across mac+linux+windows.
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < name.length(); i++) {
            char c = name.charAt(i);
            if (Character.isLetterOrDigit(c) || c == '_' || c == '.' || c == '-') {
                sb.append(c);
            } else if (c == ':') {
                sb.append('_');
            } else {
                sb.append('-');
            }
        }
        String s = sb.toString();
        if (s.length() > 120) s = s.substring(0, 120) + "_trunc";
        return s.isEmpty() ? "anon" : s;
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
