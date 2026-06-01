# claims — distributed-decomp claim ledger (issue #11)

This **orphan branch** is the live claim ledger for meteor-decomp's
distributed, fork-based decompilation workflow. It is written **only** by the
base-repo claim automation — `.github/workflows/{claim,claim-pr,sweeper,reconcile}.yml`
using the `CLAIMS_BOT_TOKEN` secret. **Do not hand-edit or merge feature work here.**

## Layout

    claims/<binary>/<decimal_rva>.json   # one file per ACTIVE claim

Each claim file:

    { "binary", "rva", "va", "owner", "claimed_at", "lease_expires_at", "state" }
    state ∈ { "active", "expiring", "pinned-by-PR" }

The committed `src/<binary>/_rosetta/FUN_<va>.cpp` tree on `develop` is the
**solved set**; a merged match auto-releases its claim via `reconcile.yml`.
A claim has a 60h lease (heartbeat to extend); an open PR pins it with no
expiry; the sweeper reclaims abandoned leases after a grace pass.

See **`docs/claim-protocol.md`** on `develop` for the full lifecycle and the
maintainer bootstrap.
