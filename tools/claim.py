#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Pure, unit-testable claim-ledger library + CLI for the fork-based
distributed-decomp claim system (issue #11, D1/D2).

THE MODEL (read this before touching anything)
-----------------------------------------------
A dedicated orphan ``claims`` branch holds the ledger: ONE file per
ACTIVE claim, at path ``claims/<bin>/<decimal_rva>.json``. No solved set
is stored here — the committed ``src/<bin>/_rosetta/FUN_<va>.cpp`` tree
on ``develop`` IS the solved set (a VA is "solved" iff that file
exists). The claims branch is therefore tiny: it only ever holds the
handful of in-flight + PR-pinned claims, never the 67k-file solved set.

Claim-file schema (one JSON object per file)::

    {
      "binary":           "ffxivgame",
      "rva":              196636,            # decimal RVA (matches path)
      "va":               "0x0042fc1c",      # RVA + IMAGE_BASE, _rosetta key
      "owner":            "octocat",         # AUTHENTICATED identity only
      "claimed_at":       "2026-05-31T12:00:00+00:00",
      "lease_expires_at": "2026-06-02T00:00:00+00:00",   # null when pinned
      "state":            "active"           # active | expiring | pinned-by-PR
    }

ATOMICITY lives in the workflow, NOT here: the workflow checks out the
``claims`` branch, runs this tool to mutate the local ledger files,
commits, and pushes. GitHub's serialized per-ref commit rejects a
non-fast-forward push, which is the compare-and-swap; on rejection the
workflow re-fetches and RE-RUNS this tool. This module is pure: it never
touches git, never reads the wall clock (``now`` is always injected), and
operates only on a local ledger directory + a local ``src/`` tree (or an
explicit solved-VA index). That makes every decision deterministic and
testable without GitHub.

3-way ``decide`` semantics (ported from
decomp-agents ``work_queue.py::claim_next`` + ``release_stale_claims``):
  (1) VA solved (``_rosetta`` file present)        -> REJECT "already solved"
  (2) live lease held by SAME owner                -> EXTEND (idempotent heartbeat)
  (2) live lease held by ANOTHER owner             -> REJECT "held by <owner>"
  (3) no claim file, or lease expired              -> GRANT (write claim file)

Subcommands (CLI):
  decide          3-way claim decision; mutates ledger; prints JSON result
  pin             pin VA(s) to a PR author (state=pinned-by-PR, no expiry)
  free            delete claim file(s) for VA(s) (un-merged PR close)
  sweep           grace/quarantine expired non-pinned claims
  release-solved  delete claim files whose VA is now solved on develop

Every command takes ``--now ISO8601`` (or env ``CLAIM_NOW``) for
determinism; the workflow passes the real UTC time. ``--ttl-hours``
(env ``CLAIM_TTL_HOURS``, default 60) sets the no-PR-yet lease length.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from pathlib import Path
from typing import Callable, Iterable

# IMAGE_BASE matches tools/progress.py — _rosetta filenames are keyed by
# the VA (RVA + IMAGE_BASE), the claim path is keyed by the RVA.
IMAGE_BASE = 0x400000

# Default lease length for a claim that has no open PR yet.
DEFAULT_TTL_HOURS = 60

# Default grace window applied on the first sweep past expiry before a
# later sweep is allowed to delete an un-renewed claim.
DEFAULT_GRACE_HOURS = 6

# _rosetta solved-file name: FUN_<va_hex>.cpp (hex, no 0x, any width).
RE_FUN_NAME = re.compile(r"^FUN_([0-9a-fA-F]+)\.cpp$")

# Claim ledger file name: <decimal_rva>.json
RE_CLAIM_NAME = re.compile(r"^([0-9]+)\.json$")

STATE_ACTIVE = "active"
STATE_EXPIRING = "expiring"
STATE_PINNED = "pinned-by-PR"

VALID_STATES = (STATE_ACTIVE, STATE_EXPIRING, STATE_PINNED)


# ---------------------------------------------------------------------------
# time helpers (pure — "now" is always injected)
# ---------------------------------------------------------------------------


def parse_iso(value: str) -> datetime:
    """Parse an ISO-8601 timestamp into a timezone-aware UTC datetime.

    Accepts a trailing ``Z`` (normalised to ``+00:00``) and naive
    timestamps (assumed UTC). Always returns an aware datetime so
    comparisons never raise the naive/aware ``TypeError``.
    """
    text = value.strip()
    if text.endswith("Z"):
        text = text[:-1] + "+00:00"
    dt = datetime.fromisoformat(text)
    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=timezone.utc)
    return dt.astimezone(timezone.utc)


def iso(dt: datetime) -> str:
    """Render an aware datetime as a stable second-resolution UTC string."""
    return dt.astimezone(timezone.utc).isoformat(timespec="seconds")


def resolve_now(now: str | datetime | None) -> datetime:
    """Resolve an injected ``now`` (str | datetime | None) to aware UTC.

    ``None`` falls back to the ``CLAIM_NOW`` env var, then to the real
    wall clock. Pure logic should always pass an explicit value; only the
    CLI entrypoint is allowed to reach the wall clock.
    """
    if now is None:
        env = os.environ.get("CLAIM_NOW")
        if env:
            return parse_iso(env)
        return datetime.now(timezone.utc)
    if isinstance(now, datetime):
        return now if now.tzinfo else now.replace(tzinfo=timezone.utc)
    return parse_iso(now)


# ---------------------------------------------------------------------------
# VA / RVA helpers
# ---------------------------------------------------------------------------


def rva_to_va(rva: int, image_base: int = IMAGE_BASE) -> int:
    return rva + image_base


def va_to_rva(va: int, image_base: int = IMAGE_BASE) -> int:
    return va - image_base


def va_hex(va: int) -> str:
    """8-wide lowercase hex with 0x prefix, matching _rosetta filenames."""
    return f"0x{va:08x}"


# ---------------------------------------------------------------------------
# solved set (the committed _rosetta tree IS the solved set)
# ---------------------------------------------------------------------------


def is_solved(
    binary: str,
    rva: int,
    *,
    src_root: Path,
    image_base: int = IMAGE_BASE,
) -> bool:
    """True iff src/<binary>/_rosetta/FUN_<va>.cpp exists.

    The _rosetta filename is keyed by VA. The committed tree IS the
    durable solved set (see tools/progress.py::_rosetta_overlay), so
    presence-of-file is the single source of truth for "done".
    """
    va = rva_to_va(rva, image_base)
    return (src_root / binary / "_rosetta" / f"FUN_{va:08x}.cpp").exists()


def solved_vas_for_binary(
    binary: str,
    *,
    src_root: Path,
    image_base: int = IMAGE_BASE,
) -> set[int]:
    """Every solved RVA for a binary, parsed from its _rosetta tree."""
    out: set[int] = set()
    rosetta_dir = src_root / binary / "_rosetta"
    if not rosetta_dir.is_dir():
        return out
    for cpp in rosetta_dir.glob("FUN_*.cpp"):
        m = RE_FUN_NAME.match(cpp.name)
        if not m:
            continue
        va = int(m.group(1), 16)
        out.add(va_to_rva(va, image_base))
    return out


# ---------------------------------------------------------------------------
# claim record + ledger I/O
# ---------------------------------------------------------------------------


@dataclass
class Claim:
    binary: str
    rva: int
    owner: str
    claimed_at: str
    lease_expires_at: str | None
    state: str

    @property
    def va(self) -> int:
        return rva_to_va(self.rva)

    @property
    def va_hex(self) -> str:
        return va_hex(self.va)

    @property
    def is_pinned(self) -> bool:
        return self.state == STATE_PINNED

    def lease_expires_dt(self) -> datetime | None:
        if self.lease_expires_at is None:
            return None
        return parse_iso(self.lease_expires_at)

    def is_live(self, now: datetime) -> bool:
        """A claim is 'live' (blocks other owners) if pinned, or its lease
        has not yet expired. Pinned claims have no expiry and are always
        live."""
        if self.is_pinned:
            return True
        exp = self.lease_expires_dt()
        if exp is None:
            return True
        return now < exp

    def to_json(self) -> dict:
        return {
            "binary": self.binary,
            "rva": self.rva,
            "va": self.va_hex,
            "owner": self.owner,
            "claimed_at": self.claimed_at,
            "lease_expires_at": self.lease_expires_at,
            "state": self.state,
        }


def claim_path(ledger_dir: Path, binary: str, rva: int) -> Path:
    return ledger_dir / binary / f"{rva}.json"


def render_claim(claim: Claim) -> str:
    """Stable, sorted, two-space JSON with trailing newline (git-friendly)."""
    return json.dumps(claim.to_json(), indent=2, sort_keys=True) + "\n"


def read_claim(ledger_dir: Path, binary: str, rva: int) -> Claim | None:
    path = claim_path(ledger_dir, binary, rva)
    if not path.exists():
        return None
    data = json.loads(path.read_text())
    return Claim(
        binary=data["binary"],
        rva=int(data["rva"]),
        owner=data["owner"],
        claimed_at=data["claimed_at"],
        lease_expires_at=data.get("lease_expires_at"),
        state=data.get("state", STATE_ACTIVE),
    )


def write_claim(ledger_dir: Path, claim: Claim) -> Path:
    path = claim_path(ledger_dir, claim.binary, claim.rva)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(render_claim(claim))
    return path


def delete_claim(ledger_dir: Path, binary: str, rva: int) -> bool:
    path = claim_path(ledger_dir, binary, rva)
    if path.exists():
        path.unlink()
        return True
    return False


def iter_claims(ledger_dir: Path) -> Iterable[Claim]:
    """Yield every claim in the ledger, in deterministic (binary, rva) order."""
    if not ledger_dir.is_dir():
        return
    for bindir in sorted(p for p in ledger_dir.iterdir() if p.is_dir()):
        binary = bindir.name
        # Sort by the parsed integer RVA, NOT the filename string — the
        # ledger paths are `<decimal_rva>.json`, and lexical filename
        # order ("131072" < "65536") would scramble numeric order.
        rvas = sorted(
            int(m.group(1))
            for f in bindir.glob("*.json")
            if (m := RE_CLAIM_NAME.match(f.name))
        )
        for rva in rvas:
            c = read_claim(ledger_dir, binary, rva)
            if c is not None:
                yield c


# ---------------------------------------------------------------------------
# core decisions (pure: ledger dir + solved predicate + injected now)
# ---------------------------------------------------------------------------

SolvedFn = Callable[[str, int], bool]


def decide(
    *,
    binary: str,
    rva: int,
    owner: str,
    now: datetime,
    ledger_dir: Path,
    solved: SolvedFn,
    ttl_hours: int = DEFAULT_TTL_HOURS,
) -> dict:
    """3-way claim decision. Mutates the ledger on grant/extend.

    Returns a JSON-able result dict::

        {"decision": "granted"|"extended"|"rejected",
         "binary","rva","va","owner",
         "reason"?: str,                 # only when rejected
         "holder"?: str,                 # current owner, when rejected by lease
         "claim"?: {...}}                # the live claim file, when written

    Ported from work_queue.py::claim_next: a VA is unavailable if it is
    already solved (here: _rosetta file present) OR held by a live claim
    owned by someone else. An expired lease is re-grantable; an
    owner-held live lease is an idempotent heartbeat (extend).
    """
    va = rva_to_va(rva)
    base = {"binary": binary, "rva": rva, "va": va_hex(va), "owner": owner}

    # (1) already solved -> REJECT. The committed _rosetta tree is the
    #     authoritative "done" signal; never hand out a solved VA.
    if solved(binary, rva):
        return {**base, "decision": "rejected", "reason": "already solved"}

    existing = read_claim(ledger_dir, binary, rva)
    if existing is not None and existing.is_live(now):
        # (2) live lease.
        if existing.owner == owner:
            # idempotent heartbeat: extend the lease (pinned stays pinned,
            # no expiry; active/expiring gets a fresh TTL + reset to active).
            if existing.is_pinned:
                # An open PR pins the claim with no expiry; a heartbeat is
                # a no-op that just re-affirms ownership.
                return {
                    **base,
                    "decision": "extended",
                    "claim": existing.to_json(),
                }
            existing.lease_expires_at = iso(now + timedelta(hours=ttl_hours))
            existing.state = STATE_ACTIVE
            write_claim(ledger_dir, existing)
            return {**base, "decision": "extended", "claim": existing.to_json()}
        # held by someone else -> REJECT.
        return {
            **base,
            "decision": "rejected",
            "reason": f"held by {existing.owner}",
            "holder": existing.owner,
        }

    # (3) free, or lease expired -> GRANT.
    claim = Claim(
        binary=binary,
        rva=rva,
        owner=owner,
        claimed_at=iso(now),
        lease_expires_at=iso(now + timedelta(hours=ttl_hours)),
        state=STATE_ACTIVE,
    )
    write_claim(ledger_dir, claim)
    return {**base, "decision": "granted", "claim": claim.to_json()}


def pin(
    *,
    binary: str,
    rva: int,
    owner: str,
    now: datetime,
    ledger_dir: Path,
) -> dict:
    """Pin a VA to an open PR: state=pinned-by-PR, no expiry.

    If a claim already exists (any owner/state) it is converted in place
    to pinned-by-PR but its ORIGINAL owner is preserved — the PR author
    only becomes owner when no prior claim existed. This keeps a claim's
    attribution stable across the claim->PR handoff while still pinning
    work that was opened without a prior reservation.
    """
    existing = read_claim(ledger_dir, binary, rva)
    if existing is not None:
        existing.state = STATE_PINNED
        existing.lease_expires_at = None
        write_claim(ledger_dir, existing)
        return {
            "binary": binary,
            "rva": rva,
            "va": va_hex(rva_to_va(rva)),
            "action": "pinned",
            "owner": existing.owner,
            "claim": existing.to_json(),
        }
    claim = Claim(
        binary=binary,
        rva=rva,
        owner=owner,
        claimed_at=iso(now),
        lease_expires_at=None,
        state=STATE_PINNED,
    )
    write_claim(ledger_dir, claim)
    return {
        "binary": binary,
        "rva": rva,
        "va": va_hex(rva_to_va(rva)),
        "action": "pinned-created",
        "owner": owner,
        "claim": claim.to_json(),
    }


def free(*, binary: str, rva: int, ledger_dir: Path) -> dict:
    """Delete a claim file (un-merged PR close)."""
    deleted = delete_claim(ledger_dir, binary, rva)
    return {
        "binary": binary,
        "rva": rva,
        "va": va_hex(rva_to_va(rva)),
        "action": "freed" if deleted else "absent",
    }


def sweep(
    *,
    now: datetime,
    ledger_dir: Path,
    grace_hours: int = DEFAULT_GRACE_HOURS,
) -> dict:
    """Two-phase grace/quarantine sweep of expired, non-pinned claims.

    Phase 1 (first sweep past expiry): an ``active`` claim whose lease
    has expired is marked ``expiring`` and given a short grace window
    (lease_expires_at pushed to now + grace_hours). It is NOT deleted yet
    — a worker that heartbeats within the grace window reclaims it (the
    next ``decide`` by its owner resets it to ``active``).

    Phase 2 (a later sweep): an ``expiring`` claim whose (grace-extended)
    lease is STILL expired is deleted.

    ``pinned-by-PR`` claims are never touched.
    """
    marked: list[dict] = []
    deleted: list[dict] = []
    for claim in list(iter_claims(ledger_dir)):
        if claim.is_pinned:
            continue
        exp = claim.lease_expires_dt()
        if exp is None:
            # Non-pinned with no expiry is malformed; treat as live-forever
            # rather than delete — better to leak than to nuke real work.
            continue
        if now < exp:
            continue  # still within its (possibly grace-extended) window
        if claim.state == STATE_EXPIRING:
            # phase 2: grace already elapsed un-renewed -> delete.
            delete_claim(ledger_dir, claim.binary, claim.rva)
            deleted.append(
                {"binary": claim.binary, "rva": claim.rva, "va": claim.va_hex}
            )
        else:
            # phase 1: mark expiring + grant grace window.
            claim.state = STATE_EXPIRING
            claim.lease_expires_at = iso(now + timedelta(hours=grace_hours))
            write_claim(ledger_dir, claim)
            marked.append(
                {"binary": claim.binary, "rva": claim.rva, "va": claim.va_hex}
            )
    return {"marked_expiring": marked, "deleted": deleted}


def release_solved(
    *,
    ledger_dir: Path,
    src_root: Path,
    image_base: int = IMAGE_BASE,
) -> dict:
    """Delete every claim whose VA is now solved on the develop tree.

    This is the SOLE releaser for merged VAs (reconcile.yml calls it
    AFTER the solved state is on develop). It scans the ledger, and for
    each claim checks the committed _rosetta tree; if the file exists,
    the work is durably done and the claim is removed regardless of state
    (a pinned-by-PR claim for a now-merged PR is exactly the case to
    release).
    """
    released: list[dict] = []
    for claim in list(iter_claims(ledger_dir)):
        if is_solved(
            claim.binary, claim.rva, src_root=src_root, image_base=image_base
        ):
            delete_claim(ledger_dir, claim.binary, claim.rva)
            released.append(
                {"binary": claim.binary, "rva": claim.rva, "va": claim.va_hex}
            )
    return {"released": released}


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def _add_va_args(p: argparse.ArgumentParser, *, multi: bool = False) -> None:
    p.add_argument("--binary", required=True, help="binary stem, e.g. ffxivgame")
    if multi:
        p.add_argument(
            "--rva",
            action="append",
            default=[],
            help="decimal RVA (repeatable). Use --va for VA-keyed input.",
        )
        p.add_argument(
            "--va",
            action="append",
            default=[],
            help="VA hex (0x...) or decimal (repeatable); converted to RVA.",
        )
    else:
        g = p.add_mutually_exclusive_group(required=True)
        g.add_argument("--rva", help="decimal RVA")
        g.add_argument("--va", help="VA hex (0x...) or decimal; converted to RVA")


def _parse_int(value: str) -> int:
    v = value.strip()
    return int(v, 16) if v.lower().startswith("0x") else int(v)


def _resolve_single_rva(args, image_base: int) -> int:
    if getattr(args, "rva", None):
        return _parse_int(args.rva)
    return va_to_rva(_parse_int(args.va), image_base)


def _resolve_multi_rvas(args, image_base: int) -> list[int]:
    out: list[int] = []
    for r in args.rva:
        out.append(_parse_int(r))
    for v in args.va:
        out.append(va_to_rva(_parse_int(v), image_base))
    return out


def _emit(obj: dict) -> None:
    print(json.dumps(obj, indent=2, sort_keys=True))


def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(
        prog="claim.py",
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument(
        "--ledger-dir",
        default="claims",
        help="root of the claim ledger (default: claims/)",
    )
    ap.add_argument(
        "--src-root",
        default="src",
        help="root of the source tree holding _rosetta/ (default: src/)",
    )
    ap.add_argument(
        "--now",
        default=None,
        help="ISO-8601 'now' (default: $CLAIM_NOW, then wall clock)",
    )
    ap.add_argument(
        "--ttl-hours",
        type=int,
        default=int(os.environ.get("CLAIM_TTL_HOURS", DEFAULT_TTL_HOURS)),
        help=f"lease TTL hours for no-PR-yet claims (default: {DEFAULT_TTL_HOURS})",
    )
    ap.add_argument(
        "--grace-hours",
        type=int,
        default=int(os.environ.get("CLAIM_GRACE_HOURS", DEFAULT_GRACE_HOURS)),
        help=f"sweep grace window hours (default: {DEFAULT_GRACE_HOURS})",
    )
    ap.add_argument(
        "--image-base",
        type=lambda s: int(s, 0),
        default=IMAGE_BASE,
        help="PE image base for RVA<->VA (default: 0x400000)",
    )

    sub = ap.add_subparsers(dest="cmd", required=True)

    p_decide = sub.add_parser("decide", help="3-way claim decision (mutates ledger)")
    _add_va_args(p_decide)
    p_decide.add_argument(
        "--owner",
        required=True,
        help="AUTHENTICATED requester login (never request-body data)",
    )

    p_pin = sub.add_parser("pin", help="pin VA(s) to a PR (state=pinned-by-PR)")
    _add_va_args(p_pin, multi=True)
    p_pin.add_argument(
        "--owner",
        required=True,
        help="PR author login (used only when no prior claim exists)",
    )

    p_free = sub.add_parser("free", help="delete claim file(s) for VA(s)")
    _add_va_args(p_free, multi=True)

    sub.add_parser("sweep", help="grace/quarantine expired non-pinned claims")

    sub.add_parser(
        "release-solved",
        help="delete claims whose VA is now solved in src/<bin>/_rosetta",
    )

    return ap


def main(argv: list[str] | None = None) -> int:
    ap = build_parser()
    args = ap.parse_args(argv)

    ledger_dir = Path(args.ledger_dir)
    src_root = Path(args.src_root)
    now = resolve_now(args.now)
    image_base = args.image_base

    def solved_fn(binary: str, rva: int) -> bool:
        return is_solved(binary, rva, src_root=src_root, image_base=image_base)

    if args.cmd == "decide":
        rva = _resolve_single_rva(args, image_base)
        result = decide(
            binary=args.binary,
            rva=rva,
            owner=args.owner,
            now=now,
            ledger_dir=ledger_dir,
            solved=solved_fn,
            ttl_hours=args.ttl_hours,
        )
        _emit(result)
        # Non-zero exit on rejection so a workflow `if` can branch on it
        # without parsing JSON, while still printing the structured reason.
        return 0 if result["decision"] in ("granted", "extended") else 2

    if args.cmd == "pin":
        rvas = _resolve_multi_rvas(args, image_base)
        results = [
            pin(
                binary=args.binary,
                rva=rva,
                owner=args.owner,
                now=now,
                ledger_dir=ledger_dir,
            )
            for rva in rvas
        ]
        _emit({"pinned": results})
        return 0

    if args.cmd == "free":
        rvas = _resolve_multi_rvas(args, image_base)
        results = [
            free(binary=args.binary, rva=rva, ledger_dir=ledger_dir) for rva in rvas
        ]
        _emit({"freed": results})
        return 0

    if args.cmd == "sweep":
        result = sweep(
            now=now, ledger_dir=ledger_dir, grace_hours=args.grace_hours
        )
        _emit(result)
        return 0

    if args.cmd == "release-solved":
        result = release_solved(
            ledger_dir=ledger_dir, src_root=src_root, image_base=image_base
        )
        _emit(result)
        return 0

    ap.error(f"unknown command: {args.cmd}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
