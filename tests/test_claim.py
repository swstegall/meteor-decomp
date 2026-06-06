# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Unit tests for tools/claim.py — the pure claim-ledger library.

Every test injects a fixed ``now`` and operates on a tmp ledger dir +
a tmp ``src/<bin>/_rosetta`` tree, so nothing here touches GitHub or the
wall clock. Mirrors the 3-way semantics ported from decomp-agents
work_queue.py::claim_next.
"""

from __future__ import annotations

import sys
from datetime import datetime, timedelta, timezone
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))

import claim  # noqa: E402

BINARY = "ffxivgame"
RVA = 0x2FC1C  # -> VA 0x0042fc1c
OWNER_A = "alice"
OWNER_B = "bob"

T0 = datetime(2026, 5, 31, 12, 0, 0, tzinfo=timezone.utc)


# --------------------------------------------------------------------------
# fixtures / helpers
# --------------------------------------------------------------------------


@pytest.fixture
def ledger(tmp_path: Path) -> Path:
    d = tmp_path / "claims"
    d.mkdir()
    return d


@pytest.fixture
def src_root(tmp_path: Path) -> Path:
    d = tmp_path / "src"
    d.mkdir()
    return d


def make_solved_fn(src_root: Path):
    def solved(binary: str, rva: int) -> bool:
        return claim.is_solved(binary, rva, src_root=src_root)

    return solved


def mark_solved(src_root: Path, binary: str, rva: int) -> Path:
    """Create the durable _rosetta solved-file for a VA."""
    va = claim.rva_to_va(rva)
    rosetta = src_root / binary / "_rosetta"
    rosetta.mkdir(parents=True, exist_ok=True)
    f = rosetta / f"FUN_{va:08x}.cpp"
    f.write_text("// solved\n")
    return f


def do_decide(ledger, src_root, *, owner, now, rva=RVA, ttl_hours=60):
    return claim.decide(
        binary=BINARY,
        rva=rva,
        owner=owner,
        now=now,
        ledger_dir=ledger,
        solved=make_solved_fn(src_root),
        ttl_hours=ttl_hours,
    )


# --------------------------------------------------------------------------
# va / rosetta plumbing
# --------------------------------------------------------------------------


def test_rva_va_roundtrip():
    assert claim.rva_to_va(RVA) == 0x42FC1C
    assert claim.va_to_rva(0x42FC1C) == RVA
    assert claim.va_hex(0x42FC1C) == "0x0042fc1c"


def test_is_solved_matches_rosetta_filename(src_root):
    assert not claim.is_solved(BINARY, RVA, src_root=src_root)
    mark_solved(src_root, BINARY, RVA)
    assert claim.is_solved(BINARY, RVA, src_root=src_root)


def test_solved_vas_for_binary_parses_tree(src_root):
    mark_solved(src_root, BINARY, RVA)
    mark_solved(src_root, BINARY, 0x10000)
    (src_root / BINARY / "_rosetta" / "garbage.cpp").write_text("x")
    assert claim.solved_vas_for_binary(BINARY, src_root=src_root) == {RVA, 0x10000}


# --------------------------------------------------------------------------
# decide — the 3-way decision
# --------------------------------------------------------------------------


def test_grant_on_free(ledger, src_root):
    result = do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    assert result["decision"] == "granted"
    assert result["owner"] == OWNER_A
    assert result["va"] == "0x0042fc1c"

    stored = claim.read_claim(ledger, BINARY, RVA)
    assert stored is not None
    assert stored.owner == OWNER_A
    assert stored.state == claim.STATE_ACTIVE
    assert stored.claimed_at == claim.iso(T0)
    assert stored.lease_expires_at == claim.iso(T0 + timedelta(hours=60))


def test_reject_on_solved(ledger, src_root):
    mark_solved(src_root, BINARY, RVA)
    result = do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    assert result["decision"] == "rejected"
    assert result["reason"] == "already solved"
    # nothing written to the ledger for a solved VA
    assert claim.read_claim(ledger, BINARY, RVA) is None


def test_reject_on_nonowner_live_lease(ledger, src_root):
    granted = do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    assert granted["decision"] == "granted"

    # Bob asks 1h later — Alice's 60h lease is still live.
    result = do_decide(ledger, src_root, owner=OWNER_B, now=T0 + timedelta(hours=1))
    assert result["decision"] == "rejected"
    assert result["reason"] == f"held by {OWNER_A}"
    assert result["holder"] == OWNER_A
    # ownership unchanged
    assert claim.read_claim(ledger, BINARY, RVA).owner == OWNER_A


def test_extend_on_owner(ledger, src_root):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    later = T0 + timedelta(hours=10)
    result = do_decide(ledger, src_root, owner=OWNER_A, now=later)
    assert result["decision"] == "extended"

    stored = claim.read_claim(ledger, BINARY, RVA)
    # lease pushed forward from the heartbeat time, not the original claim.
    assert stored.lease_expires_at == claim.iso(later + timedelta(hours=60))
    # claimed_at preserved across the heartbeat
    assert stored.claimed_at == claim.iso(T0)
    assert stored.state == claim.STATE_ACTIVE


def test_expire_then_grant(ledger, src_root):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    # Bob asks AFTER Alice's lease has fully expired (60h + slack).
    after = T0 + timedelta(hours=61)
    result = do_decide(ledger, src_root, owner=OWNER_B, now=after)
    assert result["decision"] == "granted"
    assert result["owner"] == OWNER_B

    stored = claim.read_claim(ledger, BINARY, RVA)
    assert stored.owner == OWNER_B
    assert stored.claimed_at == claim.iso(after)


def test_extend_owner_reclaims_within_grace_after_sweep(ledger, src_root):
    # grant, expire, sweep->expiring, then original owner heartbeats inside grace
    do_decide(ledger, src_root, owner=OWNER_A, now=T0, ttl_hours=60)
    sweep_at = T0 + timedelta(hours=61)
    claim.sweep(now=sweep_at, ledger_dir=ledger, grace_hours=6)
    assert claim.read_claim(ledger, BINARY, RVA).state == claim.STATE_EXPIRING

    # Alice heartbeats 1h into the 6h grace -> reclaimed as active.
    result = do_decide(
        ledger, src_root, owner=OWNER_A, now=sweep_at + timedelta(hours=1)
    )
    assert result["decision"] == "extended"
    assert claim.read_claim(ledger, BINARY, RVA).state == claim.STATE_ACTIVE


# --------------------------------------------------------------------------
# pin / free
# --------------------------------------------------------------------------


def test_pin_existing_claim_preserves_owner(ledger, src_root):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    result = claim.pin(binary=BINARY, rva=RVA, owner=OWNER_B, now=T0, ledger_dir=ledger)
    assert result["action"] == "pinned"
    stored = claim.read_claim(ledger, BINARY, RVA)
    assert stored.state == claim.STATE_PINNED
    assert stored.lease_expires_at is None
    # original owner preserved even though PR author (Bob) opened the pin
    assert stored.owner == OWNER_A


def test_pin_without_prior_claim_attributes_to_pr_author(ledger, src_root):
    result = claim.pin(binary=BINARY, rva=RVA, owner=OWNER_B, now=T0, ledger_dir=ledger)
    assert result["action"] == "pinned-created"
    stored = claim.read_claim(ledger, BINARY, RVA)
    assert stored.owner == OWNER_B
    assert stored.state == claim.STATE_PINNED
    assert stored.lease_expires_at is None


def test_free_deletes(ledger, src_root):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    result = claim.free(binary=BINARY, rva=RVA, ledger_dir=ledger)
    assert result["action"] == "freed"
    assert claim.read_claim(ledger, BINARY, RVA) is None
    # idempotent: freeing an absent claim is fine
    again = claim.free(binary=BINARY, rva=RVA, ledger_dir=ledger)
    assert again["action"] == "absent"


def test_pinned_blocks_other_owner_forever(ledger, src_root):
    claim.pin(binary=BINARY, rva=RVA, owner=OWNER_A, now=T0, ledger_dir=ledger)
    # far in the future, no TTL applies to a pinned claim
    result = do_decide(
        ledger, src_root, owner=OWNER_B, now=T0 + timedelta(days=365)
    )
    assert result["decision"] == "rejected"
    assert result["holder"] == OWNER_A


# --------------------------------------------------------------------------
# sweep — grace then delete
# --------------------------------------------------------------------------


def test_pinned_never_swept(ledger, src_root):
    claim.pin(binary=BINARY, rva=RVA, owner=OWNER_A, now=T0, ledger_dir=ledger)
    result = claim.sweep(now=T0 + timedelta(days=999), ledger_dir=ledger)
    assert result["marked_expiring"] == []
    assert result["deleted"] == []
    assert claim.read_claim(ledger, BINARY, RVA).state == claim.STATE_PINNED


def test_sweep_does_not_touch_live_claim(ledger, src_root):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0, ttl_hours=60)
    # within the lease window
    result = claim.sweep(now=T0 + timedelta(hours=10), ledger_dir=ledger)
    assert result["marked_expiring"] == []
    assert result["deleted"] == []
    assert claim.read_claim(ledger, BINARY, RVA).state == claim.STATE_ACTIVE


def test_sweep_grace_then_delete(ledger, src_root):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0, ttl_hours=60)

    # phase 1: first sweep past expiry -> marked expiring, NOT deleted
    s1 = claim.sweep(now=T0 + timedelta(hours=61), ledger_dir=ledger, grace_hours=6)
    assert len(s1["marked_expiring"]) == 1
    assert s1["deleted"] == []
    marked = claim.read_claim(ledger, BINARY, RVA)
    assert marked.state == claim.STATE_EXPIRING
    # grace window granted
    assert marked.lease_expires_at == claim.iso(
        T0 + timedelta(hours=61) + timedelta(hours=6)
    )

    # a sweep INSIDE the grace window deletes nothing
    s_mid = claim.sweep(
        now=T0 + timedelta(hours=64), ledger_dir=ledger, grace_hours=6
    )
    assert s_mid["deleted"] == []
    assert claim.read_claim(ledger, BINARY, RVA) is not None

    # phase 2: a later sweep past the grace window -> deleted
    s2 = claim.sweep(now=T0 + timedelta(hours=70), ledger_dir=ledger, grace_hours=6)
    assert len(s2["deleted"]) == 1
    assert s2["marked_expiring"] == []
    assert claim.read_claim(ledger, BINARY, RVA) is None


# --------------------------------------------------------------------------
# release-solved
# --------------------------------------------------------------------------


def test_release_solved_deletes_only_solved(ledger, src_root):
    # two active claims; only one becomes solved on develop
    rva_solved = RVA
    rva_unsolved = 0x10000
    do_decide(ledger, src_root, owner=OWNER_A, now=T0, rva=rva_solved)
    do_decide(ledger, src_root, owner=OWNER_B, now=T0, rva=rva_unsolved)

    mark_solved(src_root, BINARY, rva_solved)

    result = claim.release_solved(ledger_dir=ledger, src_root=src_root)
    released_rvas = {r["rva"] for r in result["released"]}
    assert released_rvas == {rva_solved}

    assert claim.read_claim(ledger, BINARY, rva_solved) is None
    assert claim.read_claim(ledger, BINARY, rva_unsolved) is not None


def test_release_solved_releases_pinned(ledger, src_root):
    # a merged PR leaves a pinned claim; release-solved is the sole releaser
    claim.pin(binary=BINARY, rva=RVA, owner=OWNER_A, now=T0, ledger_dir=ledger)
    mark_solved(src_root, BINARY, RVA)
    result = claim.release_solved(ledger_dir=ledger, src_root=src_root)
    assert len(result["released"]) == 1
    assert claim.read_claim(ledger, BINARY, RVA) is None


# --------------------------------------------------------------------------
# ledger I/O round-trip + JSON shape
# --------------------------------------------------------------------------


def test_claim_file_json_shape(ledger, src_root):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    import json

    path = claim.claim_path(ledger, BINARY, RVA)
    data = json.loads(path.read_text())
    assert set(data) == {
        "binary",
        "rva",
        "va",
        "owner",
        "claimed_at",
        "lease_expires_at",
        "state",
    }
    assert data["binary"] == BINARY
    assert data["rva"] == RVA
    assert data["va"] == "0x0042fc1c"


def test_iter_claims_deterministic_order(ledger, src_root):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0, rva=0x20000)
    do_decide(ledger, src_root, owner=OWNER_A, now=T0, rva=0x10000)
    claim.pin(binary="ffxivboot", rva=0x5000, owner=OWNER_B, now=T0, ledger_dir=ledger)
    keys = [(c.binary, c.rva) for c in claim.iter_claims(ledger)]
    assert keys == [("ffxivboot", 0x5000), (BINARY, 0x10000), (BINARY, 0x20000)]


# --------------------------------------------------------------------------
# CLI smoke (exercises arg parsing + exit codes)
# --------------------------------------------------------------------------


def test_cli_decide_grant_exit0(ledger, src_root, capsys):
    rc = claim.main(
        [
            "--ledger-dir",
            str(ledger),
            "--src-root",
            str(src_root),
            "--now",
            "2026-05-31T12:00:00+00:00",
            "decide",
            "--binary",
            BINARY,
            "--va",
            "0x0042fc1c",
            "--owner",
            OWNER_A,
        ]
    )
    assert rc == 0
    out = capsys.readouterr().out
    assert '"decision": "granted"' in out


def test_cli_decide_rejected_solved_exit2(ledger, src_root, capsys):
    mark_solved(src_root, BINARY, RVA)
    rc = claim.main(
        [
            "--ledger-dir",
            str(ledger),
            "--src-root",
            str(src_root),
            "--now",
            "2026-05-31T12:00:00+00:00",
            "decide",
            "--binary",
            BINARY,
            "--rva",
            str(RVA),
            "--owner",
            OWNER_A,
        ]
    )
    assert rc == 2
    assert '"already solved"' in capsys.readouterr().out


def test_cli_release_solved(ledger, src_root, capsys):
    do_decide(ledger, src_root, owner=OWNER_A, now=T0)
    mark_solved(src_root, BINARY, RVA)
    rc = claim.main(
        [
            "--ledger-dir",
            str(ledger),
            "--src-root",
            str(src_root),
            "release-solved",
        ]
    )
    assert rc == 0
    assert claim.read_claim(ledger, BINARY, RVA) is None
