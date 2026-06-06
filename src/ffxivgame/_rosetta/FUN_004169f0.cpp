// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later

struct C {
    int m_f0;
    int m_f4;
    int m_f8;
    int m_fC;

    bool isEqual() const;
};

bool C::isEqual() const {
    return m_f8 == m_fC;
}
