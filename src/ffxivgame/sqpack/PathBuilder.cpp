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
// FUNCTION: ffxivgame 0x0044b3a0 — Sqex resource-id → path builder
// (functional decomp; not byte-matched).
//
// 1.x is resource-id-addressed (NOT string-path-hashed like ARR
// Sqpack). Every asset has a 32-bit `resource_id` and the file lives at:
//
//     <game-root>/data/<b3>/<b2>/<b1>/<b0>.DAT
//
// where `b3..b0` are the four bytes of `resource_id` written as
// 2-digit uppercase hex.
//
// The orig function at RVA 0x0004b3a0 (615 B) is a complex 2-path
// implementation: an "extended" path (taken when a global flag at
// `[0x01266b64]` is non-zero) calls a sprintf-like helper at
// `FUN_00447620` with a format string at `.rdata 0x00b672bc`:
//
//     "%cdata%c%02X%c%02X%c%02X%c%02X.DAT"
//
// passing `'\\'` as each `%c` separator and the four bytes of
// `resource_id` (b3, b2, b1, b0) as the four `%02X` fields. Output:
// `\data\<b3>\<b2>\<b1>\<b0>.DAT`.
//
// The other path (`[0x01266b64] == 0`) is a more elaborate "modded
// resource lookup" — first calls `FUN_0044b2a0(resource_id >> 16)` to
// look up an override entry (probably for replaceable game assets),
// uses indirection through `[0x0132cb8c]` and an alternative format
// string at `0x00b672e4`. This path is unimplemented here — garlemald
// doesn't have replaceable resources so the standard path suffices.
//
// Cross-check vs SeventhUmbral's port (`SeventhUmbral/dataobjects/
// FileManager.cpp::CFileManager::GetResourcePath`):
//
//     auto resourceIdName = string_format(
//         "%0.2X/%0.2X/%0.2X/%0.2X.DAT",
//         (resourceId >> 24) & 0xFF, (resourceId >> 16) & 0xFF,
//         (resourceId >>  8) & 0xFF, (resourceId >>  0) & 0xFF);
//     return dataPath / resourceIdName;
//
// SeventhUmbral uses `/` (cross-platform); the binary uses `\\`
// (Windows-native). Both produce the same byte sequence on disk after
// the OS path layer normalises separators. For tools that need to
// open the file portably (e.g. on macOS / Linux), pass the result
// through a separator-normalisation step before touching the FS.
//
// Verification: the function is a pure mapping from a 32-bit input
// to a fixed-format string. Test by feeding known resource_ids and
// string-comparing against the format above. See
// `tools/test_path_builder.py` for the round-trip check.

#include <stdio.h>
#include <string.h>

namespace meteor_decomp {
namespace sqex {

// Build the data-DAT path for a given resource_id.
//
// `out` must have room for at least 24 bytes. Returns the number of
// bytes written (excluding the terminating NUL), or -1 on overflow.
//
// Output format (Windows-style separators, matching the binary's
// behaviour at RVA 0x0004b3a0):
//
//     \data\BB\BB\BB\BB.DAT
//
// where the four BBs are the resource_id bytes (high to low) printed
// as 2-digit uppercase hex. Always exactly 21 characters + NUL.
int build_resource_path(unsigned resource_id, char *out, unsigned out_size) {
    unsigned char b3 = (unsigned char)((resource_id >> 24) & 0xff);
    unsigned char b2 = (unsigned char)((resource_id >> 16) & 0xff);
    unsigned char b1 = (unsigned char)((resource_id >>  8) & 0xff);
    unsigned char b0 = (unsigned char)((resource_id >>  0) & 0xff);
    int n = snprintf(out, out_size,
                     "\\data\\%02X\\%02X\\%02X\\%02X.DAT",
                     b3, b2, b1, b0);
    if (n < 0 || (unsigned)n >= out_size) {
        return -1;
    }
    return n;
}

// Cross-platform variant — emits forward-slash separators so callers
// can use the result directly with POSIX file I/O. Same byte content
// as the Windows path with separators swapped; identical filesystem
// semantics on Windows (which accepts both '/' and '\\').
int build_resource_path_posix(unsigned resource_id, char *out, unsigned out_size) {
    unsigned char b3 = (unsigned char)((resource_id >> 24) & 0xff);
    unsigned char b2 = (unsigned char)((resource_id >> 16) & 0xff);
    unsigned char b1 = (unsigned char)((resource_id >>  8) & 0xff);
    unsigned char b0 = (unsigned char)((resource_id >>  0) & 0xff);
    int n = snprintf(out, out_size,
                     "/data/%02X/%02X/%02X/%02X.DAT",
                     b3, b2, b1, b0);
    if (n < 0 || (unsigned)n >= out_size) {
        return -1;
    }
    return n;
}

}  // namespace sqex
}  // namespace meteor_decomp

// Embedded self-test — compile with `-DPATH_BUILDER_SELF_TEST` to get
// a `main()` that exercises a few canonical resource_ids and exits
// nonzero on mismatch.
#ifdef PATH_BUILDER_SELF_TEST
#include <stdio.h>
int main(void) {
    using meteor_decomp::sqex::build_resource_path;
    using meteor_decomp::sqex::build_resource_path_posix;
    struct case_t {
        unsigned    rid;
        const char *expect_win;
        const char *expect_posix;
    };
    const case_t cases[] = {
        // Trivial corners.
        {0x00000000u, "\\data\\00\\00\\00\\00.DAT", "/data/00/00/00/00.DAT"},
        {0xffffffffu, "\\data\\FF\\FF\\FF\\FF.DAT", "/data/FF/FF/FF/FF.DAT"},
        // Mid-range — single byte set per position.
        {0x12000000u, "\\data\\12\\00\\00\\00.DAT", "/data/12/00/00/00.DAT"},
        {0x00340000u, "\\data\\00\\34\\00\\00.DAT", "/data/00/34/00/00.DAT"},
        {0x00005600u, "\\data\\00\\00\\56\\00.DAT", "/data/00/00/56/00.DAT"},
        {0x00000078u, "\\data\\00\\00\\00\\78.DAT", "/data/00/00/00/78.DAT"},
        // Combined.
        {0x12345678u, "\\data\\12\\34\\56\\78.DAT", "/data/12/34/56/78.DAT"},
        // 1-byte values — confirm hex zero-padding.
        {0x010203abu, "\\data\\01\\02\\03\\AB.DAT", "/data/01/02/03/AB.DAT"},
    };
    int fails = 0;
    char buf[64];
    for (unsigned i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
        int n = build_resource_path(cases[i].rid, buf, sizeof(buf));
        if (n < 0 || strcmp(buf, cases[i].expect_win) != 0) {
            printf("FAIL win  rid=0x%08x: got %s expected %s\n",
                   cases[i].rid, buf, cases[i].expect_win);
            fails++;
        }
        n = build_resource_path_posix(cases[i].rid, buf, sizeof(buf));
        if (n < 0 || strcmp(buf, cases[i].expect_posix) != 0) {
            printf("FAIL posix rid=0x%08x: got %s expected %s\n",
                   cases[i].rid, buf, cases[i].expect_posix);
            fails++;
        }
    }
    if (fails == 0) {
        printf("PathBuilder: all %u test cases pass\n",
               (unsigned)(sizeof(cases)/sizeof(cases[0])));
    }
    return fails ? 1 : 0;
}
#endif
