// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Walk the symbol table for PARAMNAME_* symbols and resolve each to
// the actual property-name string it points at. Output:
//
//   config/<binary>.paramnames.json
//
// The 343 Component::GAM::CompileTimeParameter<id, &PARAMNAME_id>
// instantiations recovered in Phase 1 each reference a
// `const char* PARAMNAME_N` symbol whose value is the property's wire
// name (e.g. "playerWork.activeQuest"). This script dereferences each
// one and emits the (id, namespace, value) table.
//
//@category meteor-decomp

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.Iterator;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressFactory;
import ghidra.program.model.listing.Program;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolTable;

public class DumpParamNames extends GhidraScript {

    @Override
    public void run() throws Exception {
        Program prog = currentProgram;
        long imageBase = prog.getImageBase().getOffset();

        String repoRoot = System.getenv("METEOR_DECOMP_ROOT");
        if (repoRoot == null) repoRoot = ".";
        String binary = prog.getName().toLowerCase().replace(".exe", "");
        File configDir = new File(repoRoot, "config");
        configDir.mkdirs();
        File out = new File(configDir, binary + ".paramnames.json");

        SymbolTable tbl = prog.getSymbolTable();
        Memory mem = prog.getMemory();
        AddressFactory af = prog.getAddressFactory();

        PrintWriter w = new PrintWriter(new FileWriter(out));
        w.println("[");
        boolean first = true;
        int count = 0;
        int resolved = 0;

        Iterator<Symbol> iter = tbl.getAllSymbols(true).iterator();
        while (iter.hasNext() && !monitor.isCancelled()) {
            Symbol s = iter.next();
            String name = s.getName(false);  // unqualified
            if (!name.startsWith("PARAMNAME_")) continue;

            // Parse the integer id from "PARAMNAME_<n>".
            int id;
            try {
                id = Integer.parseInt(name.substring("PARAMNAME_".length()));
            } catch (NumberFormatException e) {
                continue;
            }

            String fullName = s.getName(true);  // qualified
            Address addr = s.getAddress();
            long ptrRva = addr.getOffset() - imageBase;

            // Read the pointer value at this address (32-bit absolute pointer
            // on PE32 i386). Then load the C string at the target.
            String value = null;
            String error = null;
            try {
                int ptr = mem.getInt(addr);
                long ptrU = ((long) ptr) & 0xFFFFFFFFL;
                Address target = af.getDefaultAddressSpace().getAddress(ptrU);
                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < 256; i++) {
                    byte b;
                    try {
                        b = mem.getByte(target.add(i));
                    } catch (Exception e) {
                        error = "memory-read failed at " + (i) + ": " + e.getMessage();
                        break;
                    }
                    if (b == 0) break;
                    sb.append((char) (b & 0xff));
                }
                if (error == null) value = sb.toString();
            } catch (Exception e) {
                error = e.getMessage();
            }

            if (!first) w.println(",");
            first = false;
            w.printf("  {\"id\": %d, \"symbol\": %s, \"ptr_rva\": %d, \"ptr_rva_hex\": \"0x%x\", \"value\": %s, \"error\": %s}",
                id, jsonStr(fullName), ptrRva, ptrRva,
                (value == null ? "null" : jsonStr(value)),
                (error == null ? "null" : jsonStr(error)));
            count++;
            if (value != null) resolved++;
        }

        w.println();
        w.println("]");
        w.close();
        println(String.format("DumpParamNames: %s — %d PARAMNAME_* symbols, %d resolved → %s",
            binary, count, resolved, out));
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
