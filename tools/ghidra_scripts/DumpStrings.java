// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Walk every defined string in currentProgram and emit:
//   config/<binary>.strings.json   — RVA → string value
//
// Strings that match recognisable __FILE__ / __FUNCTION__ patterns are
// flagged with a `kind` field so build_split_yaml.py can use them as
// seed-symbol hints downstream.
//
//@category meteor-decomp

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.regex.Pattern;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.Program;

public class DumpStrings extends GhidraScript {

    private static final Pattern FILE_HINT     = Pattern.compile("(?i)[a-z]:[\\\\/].*\\.(c|cpp|cc|cxx|h|hpp)$");
    private static final Pattern FUNCTION_HINT = Pattern.compile("^[A-Za-z_][A-Za-z0-9_]*::[A-Za-z_][A-Za-z0-9_]*$");
    private static final Pattern LUA_HINT      = Pattern.compile("^(on|process|run)[A-Z][A-Za-z]+$");

    @Override
    public void run() throws Exception {
        Program prog = currentProgram;
        long imageBase = prog.getImageBase().getOffset();

        String repoRoot = System.getenv("METEOR_DECOMP_ROOT");
        if (repoRoot == null) repoRoot = ".";
        String binary = prog.getName().toLowerCase().replace(".exe", "");
        File configDir = new File(repoRoot, "config");
        configDir.mkdirs();
        File out = new File(configDir, binary + ".strings.json");

        Listing listing = prog.getListing();
        DataIterator iter = listing.getDefinedData(true);

        PrintWriter w = new PrintWriter(new FileWriter(out));
        w.println("[");
        boolean first = true;
        int count = 0;
        int hintCount = 0;

        while (iter.hasNext() && !monitor.isCancelled()) {
            Data d = iter.next();
            if (!d.hasStringValue()) continue;
            Object val = d.getValue();
            if (val == null) continue;
            String s = val.toString();
            if (s.length() < 4) continue;  // junk filter

            Address a = d.getAddress();
            long rva = a.getOffset() - imageBase;
            String kind = "string";
            if (FILE_HINT.matcher(s).find()) { kind = "file_hint"; hintCount++; }
            else if (FUNCTION_HINT.matcher(s).matches()) { kind = "function_hint"; hintCount++; }
            else if (LUA_HINT.matcher(s).matches()) { kind = "lua_hint"; hintCount++; }

            if (!first) w.println(",");
            first = false;
            w.printf("  {\"rva\": %d, \"rva_hex\": \"0x%x\", \"len\": %d, \"kind\": \"%s\", \"value\": %s}",
                rva, rva, s.length(), kind, jsonStr(s));
            count++;
        }

        w.println();
        w.println("]");
        w.close();
        println(String.format("DumpStrings: %s — %d strings, %d seed hints → %s",
            binary, count, hintCount, out));
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
