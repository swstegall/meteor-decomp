# Claim protocol — how to reserve a function

meteor-decomp is worked on by many contributors (human and automated) in
parallel, each from their own **fork**. The claim protocol is what keeps
two contributors from decompiling the same `FUN_<va>` at the same time.

It is **GitHub-native**: a claim is a comment on a coordination issue,
and the source of truth is a dedicated orphan `claims` branch. There is
no central server and no shared bot account — every claimer authenticates
as themselves.

> **Note on the obsolete protocol.** Older docs told you to set
> `status: wip` / `owner: <you>` in `config/<bin>.yaml` and commit that as
> the claim. That no longer works: `config/<bin>.yaml` is **gitignored and
> regenerated** by `make split`, so it can never be a durable claim ledger.
> Use the protocol below instead.

## The model in one paragraph

A claim is a single JSON file on the orphan **`claims`** branch at
`claims/<binary>/<decimal_rva>.json`. The branch is tiny — it only ever
holds the handful of in-flight + PR-pinned claims, never the full solved
set. The **solved set is not stored in the ledger at all**: the committed
`src/<binary>/_rosetta/FUN_<va>.cpp` tree on `develop` *is* the solved set
(a VA is "solved" iff that file exists). You claim a free VA, match it,
open a PR; merging the PR auto-releases the claim.

## Lifecycle

```
  free-set discovery
        │
        ▼
   /claim FUN_<va> <binary>   ── comment on the coordination issue
        │                        fires claim.yml → tools/claim.py decide
        ▼
   lease granted (60h)        ── claims/<bin>/<rva>.json, state=active
        │
        ├── heartbeat: re-post /claim before expiry → lease extended (idempotent)
        │
        ├── open a PR adding src/<bin>/_rosetta/FUN_<va>.cpp
        │        → claim-pr.yml pins it (state=pinned-by-PR, no expiry)
        │
        │   ┌─ PR merged  → reconcile.yml release-solved deletes the claim
        │   └─ PR closed unmerged → claim-pr.yml frees (deletes) the claim
        │
        └── lease lapses un-renewed → sweeper.yml: grace → quarantine → delete
```

### 1. Discover the free set

A VA is **free** iff it is none of:

- **solved** — `src/<binary>/_rosetta/FUN_<va>.cpp` exists on `develop`;
- **live-claimed** — a file exists at `claims/<binary>/<decimal_rva>.json`
  on the `claims` branch (sweeper deletes expired non-pinned claims, so
  the mere presence of a ledger file means "in flight — leave it");
- **open-PR** — its `_rosetta/FUN_<va>.cpp` is added by an open PR into
  `develop` (so two claimers don't both PR the same VA before the ledger
  catches up).

Subtract those three sets from the binary's function pool
(`config/<binary>.yaml`). To inspect the two GitHub-side sets manually:

```sh
# Solved set on develop (presence-of-file = solved):
git ls-tree -r --name-only origin/develop src/ffxivgame/_rosetta/

# Live claims on the claims branch:
git ls-tree -r --name-only origin/claims claims/ffxivgame/
```

The automated path (decomp-agents distributed mode) computes all three
sets for you — see [decomp-agents `README.md` § Distributed
mode](../../decomp-agents/README.md).

### 2. Claim it

Post a comment on the pinned coordination issue:

```
/claim FUN_004165b0 ffxivgame
```

- The VA token may be `FUN_<va>`, `0x<va>`, or bare `<va>` hex (4–8
  nibbles). The binary stem is optional and defaults to `ffxivgame`.
- `claim.yml` parses the VA from the (untrusted) comment body but binds
  the claim's **`owner` to the authenticated comment author** — you can't
  spoof someone else's claim.
- It runs `tools/claim.py decide`, which is a 3-way decision:
  - **granted** — the VA was free (or its lease had expired): a lease is
    written with a 60h TTL.
  - **extended** — you already hold a live lease (idempotent heartbeat;
    the lease is refreshed).
  - **rejected** — the VA is already solved, or held by another owner.
    The bot replies with the reason (and the holder, if relevant).
- The bot replies on the issue with the outcome. On a grant you'll be
  told to open a PR adding `src/<binary>/_rosetta/FUN_<va>.cpp` to pin it.

### 3. Match it

Do the work on a branch in your fork — see [CONTRIBUTING.md](../CONTRIBUTING.md)
and [docs/matching-workflow.md](matching-workflow.md). The deliverable is
exactly one file: `src/<binary>/_rosetta/FUN_<va>.cpp`, GREEN under
`make diff FUNC=FUN_<va>`.

**Heartbeat** while you work: the lease is 60h. If a match takes longer,
re-post `/claim FUN_<va> <binary>` to refresh it (an owner-held live lease
is an idempotent `extend`). Automated agents heartbeat once before each
long match phase.

### 4. Open the PR

Open a PR from your fork into `develop`, titled
`decomp: match <Sym> @0x<rva>`. The PR adds your one `_rosetta` file.

- `claim-pr.yml` (on `opened`/`reopened`) reads the PR's added
  `_rosetta/FUN_<va>.cpp` filenames *via the API* — it never checks out
  your PR head — and **pins** the matching claim: `state=pinned-by-PR`,
  no expiry. From now on the sweeper will never reclaim it, however long
  review takes.
- If the PR is **closed without merging**, `claim-pr.yml` (on `closed`,
  `merged=false`) **frees** the claim (deletes the ledger file) so the VA
  returns to the pool.

### 5. Merge → auto-release

On merge to `develop`, `reconcile.yml`:

1. recomputes `docs/reconcile-state.json` from the committed `_rosetta`
   tree (`make reconcile`),
2. regenerates the progress regions in `README.md` /
   `docs/decomp-status.md` / `PLAN.md` (`make update-docs`) and commits
   any doc churn back to `develop` (loop-guarded),
3. then runs `tools/claim.py release-solved`, which deletes every claim
   whose VA is now present in `src/<binary>/_rosetta` on `develop`.

`reconcile.yml` is the **sole releaser for merged VAs**, and it releases
*strictly after* the solved state is on `develop` — so a claim is only
dropped once the work is durably done. A merged-PR close in `claim-pr.yml`
is intentionally a no-op for exactly this reason.

## Ledger schema

One JSON object per file at `claims/<binary>/<decimal_rva>.json`:

```json
{
  "binary":           "ffxivgame",
  "rva":              196636,
  "va":               "0x0042fc1c",
  "owner":            "octocat",
  "claimed_at":       "2026-05-31T12:00:00+00:00",
  "lease_expires_at": "2026-06-02T00:00:00+00:00",
  "state":            "active"
}
```

- `rva` is decimal and matches the path; `va` is `rva + IMAGE_BASE`
  (`0x400000`) and matches the `_rosetta` filename.
- `lease_expires_at` is `null` when `state == "pinned-by-PR"`.
- `state ∈ {active, expiring, pinned-by-PR}`:
  - **active** — a live lease.
  - **expiring** — the sweeper saw the lease expire and granted a short
    grace window; a heartbeat within grace reclaims it (the owner's next
    `/claim` resets it to `active`).
  - **pinned-by-PR** — an open PR holds it; never swept, no expiry.

## Atomicity (compare-and-swap)

Two contributors can `/claim` the same VA at the same instant. The
serialiser is **not** an Actions `concurrency` queue (which only de-dupes
and cancels — it can't FIFO two racers). It is GitHub's **serialized
per-ref commit**: each workflow checks out the `claims` branch, mutates
the ledger with `tools/claim.py`, commits, and `git push`es. A
non-fast-forward push is *rejected* if another writer advanced the ref
between the fetch and the push — on rejection the workflow re-fetches,
hard-resets to the new tip, **re-runs `claim.py` against the now-newer
ledger**, and retries (bounded, with backoff). "Create at most once" falls
out for free: a given ledger path can be added at most once per ref state.

`tools/claim.py` itself is pure — it never touches git and never reads the
wall clock (`--now` is injected by the workflow) — so every decision is
deterministic and unit-testable.

## CLI reference (`tools/claim.py`)

The workflows shell out to this; you rarely run it by hand, but it's the
authoritative spec. All commands take `--ledger-dir` (default `claims`),
`--src-root` (default `src`), and `--now ISO8601` (default `$CLAIM_NOW`,
then wall clock).

| Subcommand | What it does | Who calls it |
|---|---|---|
| `decide --binary B --va 0x.. --owner LOGIN` | 3-way claim decision; mutates the ledger on grant/extend. Exit 0 = granted/extended, exit 2 = rejected. | `claim.yml` |
| `pin --binary B --va 0x.. [--va ..] --owner LOGIN` | Pin VA(s) to a PR (`state=pinned-by-PR`, no expiry); preserves an existing claim's original owner. | `claim-pr.yml` (opened/reopened) |
| `free --binary B --va 0x.. [--va ..]` | Delete claim file(s) — un-merged PR close. | `claim-pr.yml` (closed, unmerged) |
| `sweep` | Two-phase grace/quarantine of expired non-pinned claims. | `sweeper.yml` |
| `release-solved` | Delete every claim whose VA is now solved on `develop`. | `reconcile.yml` |

Knobs: `--ttl-hours` (env `CLAIM_TTL_HOURS`, default **60**) is the
no-PR-yet lease length; `--grace-hours` (env `CLAIM_GRACE_HOURS`, default
**6**) is the sweep grace window.

---

## Operating the claim system (maintainer setup)

The workflows are committed but **inert until a maintainer does a one-time
bootstrap**. Until then, `/claim` comments get a "claim system is not yet
wired" reply and nothing is recorded (the workflows skip-with-warning when
`CLAIMS_BOT_TOKEN` is unset). The four steps below light it up.

> **Status: this bootstrap has NOT been performed yet.** A maintainer must
> run steps 1–3 once on the canonical repo before claims go live.

### 1. Create the `CLAIMS_BOT_TOKEN` secret

The claim workflows push the orphan `claims` branch under a dedicated
identity. Create a **fine-grained personal access token** owned by a
maintainer:

- **Repository access:** only this repository.
- **Permissions:** **Contents → Read and write** (nothing else).
- No expiry, or a long one you'll rotate.

Save it as a **repository secret** named `CLAIMS_BOT_TOKEN`:

```sh
# from a clone of the canonical repo, with `gh` authenticated as a maintainer:
gh secret set CLAIMS_BOT_TOKEN --repo <owner>/meteor-decomp
# (paste the PAT when prompted)
```

The default `GITHUB_TOKEN` is intentionally *not* used for the ledger push
— the claim writes are attributed to the `claims-bot` identity, and using a
PAT keeps the compare-and-swap a real branch push.

### 2. Create the orphan `claims` branch

The ledger lives on a branch with **no shared history** with `develop` (it
holds only `claims/…json`, never source). Create it once:

```sh
git switch --orphan claims
git rm -rf . 2>/dev/null || true        # empty the worktree
git commit --allow-empty -m "claims: initialize empty ledger branch"
git push -u origin claims
git switch develop                       # back to normal work
```

The workflows checkout this branch shallowly, add/remove
`claims/<bin>/<rva>.json` files, and push. They never touch `develop`'s
history.

### 3. Open the pinned coordination issue

Open one issue whose comments drive `/claim`. Pin it so contributors can
find it:

```sh
gh issue create --repo <owner>/meteor-decomp \
  --title "Claim coordination — post /claim FUN_<va> <binary> here" \
  --body "Comment \`/claim FUN_<va> <binary>\` to reserve a function. See docs/claim-protocol.md."
gh issue pin <issue-number> --repo <owner>/meteor-decomp
```

Any `issue_comment` starting with `/claim` fires `claim.yml`. (Comments on
*any* issue work, since `claim.yml` triggers on `issue_comment` globally —
but a single pinned issue keeps coordination legible and discoverable. Put
its number in decomp-agents' `DECOMP_CLAIM_ISSUE`.)

### 4. Lease TTL / sweeper cron knobs

Defaults are sane; tune only if needed.

| Knob | Where | Default | Notes |
|---|---|---|---|
| Lease TTL | `CLAIM_TTL_HOURS` env / `--ttl-hours` (claim.py) | **60h** | No-PR-yet lease length. |
| Sweep grace | `CLAIM_GRACE_HOURS` env in `sweeper.yml` / `--grace-hours` | **6h** | Window between "marked expiring" and deletion. |
| Sweep cadence | `cron` in `sweeper.yml` | `13 */6 * * *` (every 6h) | **Keep the cron interval ≤ grace** so a claim survives at least one full grace window before phase-2 deletion. |

After the bootstrap, smoke-test by commenting `/claim FUN_00416000 ffxivgame`
on the coordination issue and confirming the bot replies and a
`claims/ffxivgame/89152.json` file lands on the `claims` branch.
