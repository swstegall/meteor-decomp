# Releasing

`meteor-decomp` uses [Semantic Versioning 2.0.0](https://semver.org/) driven
**entirely by git tags**. The annotated git tag **`vX.Y.Z` is the sole source of
truth** — there is **nothing in the tree to version**. This repo has no
`Cargo.toml` and no `VERSION` file, so a release writes **no commit** back to the
branch: it only creates and pushes a tag (and publishes a GitHub Release).

## Branching model

- **`develop`** is the default branch and the integration branch for day-to-day
  decomp work. It is **unprotected** — commit/push to it directly (agent
  orchestration included) or via PR, whatever fits the moment. **Nothing merged
  into `develop` produces a release or a tag.**
- **`master`** is the protected **release** branch. A release is cut by opening a
  PR from `develop` into `master`; when it merges, the push to `master` triggers
  `release.yml` (tag + GitHub Release). `master` requires a pull request (no
  direct pushes) but **no approving review**, so you can self-merge your own
  `develop` → `master` release PR.

Flow: work on `develop` → release PR `develop` → `master` → merge → automatic
tag + GitHub Release.

> Branch protection on `master` does **not** block the release automation: the
> workflow pushes only a **tag** (`refs/tags/vX.Y.Z`), and tag pushes are not
> gated by branch-protection rules — only commits to the branch are, which this
> workflow never makes. So the default `GITHUB_TOKEN` still suffices (see
> [Why no PAT](#why-no-pat)).

## How it works

`.github/workflows/release.yml` runs on every push/merge to **`master`** (i.e.
when a `develop` → `master` release PR merges) and:

1. reads the highest existing `vX.Y.Z` tag,
2. picks a bump level (see below),
3. creates an annotated `vX.Y.Z` tag on the merge commit and pushes **only that
   tag** (no commit, no file edits — nothing is rewritten in the tree), and
4. publishes a GitHub Release with auto-generated notes, best-effort appended
   with the current decomp progress (see [Progress tie-in](#progress-tie-in)).

After a release, `git describe --tags` on `master` reports the new `X.Y.Z`.

## Choosing the bump level

| Bump      | How to trigger                                                                 |
|-----------|--------------------------------------------------------------------------------|
| **Patch** | Default. Any merge/push to `master` with no release label → `Z` increments.     |
| **Minor** | Add the **`release:minor`** label to the PR before merging → `Y+1`, `Z=0`.      |
| **Major** | Add the **`release:major`** label to the PR before merging → `X+1`, `Y=0`, `Z=0`. |

The label is read from the PR associated with the merge commit; if both labels
are present, **`release:major` wins**. A direct push with no associated PR is
treated as a patch bump.

The `release:minor` and `release:major` labels must exist in the repo.

### Manual minor/major (alternative)

Because the next version is computed from the **highest tag**, you can also bump
out-of-band by pushing a tag yourself:

```sh
git tag -a v0.2.0 -m v0.2.0 && git push origin v0.2.0
```

The automation then continues patch-incrementing from there (`v0.2.1`, …). This
is handy for the first minor/major when no PR is involved.

## Why no PAT

Unlike the sibling `garlemald-server` / `garlemald-client` release workflows,
`master` here is **not branch-protected**, and this workflow pushes **only a
tag** — it never commits anything back to the branch. The default
`GITHUB_TOKEN` (`github.token`) can:

- push a tag to an unprotected branch, and
- create a GitHub Release.

So **no fine-grained `RELEASE_PAT` is required**. The checkout uses the default
token. (If `master` were ever branch-protected, note that tag pushes are not
gated by branch protection rules, so even then the default token would still
suffice for the tag — protection only matters for commits to the branch, which
this workflow never makes.)

## Why no loop guard

Because nothing is committed back to `master`, there is no re-trigger to guard
against:

- The workflow triggers on `push: branches: [master]`.
- Pushing a **tag** does **not** match that trigger, so creating `vX.Y.Z`
  cannot re-run this workflow.

So there is deliberately **no bot-identity `if` guard** and **no `[skip ci]`
marker** — both are only needed when a workflow pushes a *commit* back to the
branch it triggers on, which this one never does.

## Progress tie-in

The Release body is best-effort augmented with current decomp progress. The
workflow runs `tools/progress.py` (after `pip install pyyaml`), captures the
`overall (YAML status):` and `overall (_rosetta/*.cpp):` summary lines, and
**edits the just-created Release** to prepend a short "Decomp progress at
vX.Y.Z" block above GitHub's auto-generated notes.

The create-then-edit approach is used because `gh release create` cannot combine
`--generate-notes` with `--notes-file`; editing afterward keeps the generated
notes *and* the progress block. The whole progress attempt is wrapped so that a
failure (PyYAML missing, network blip, `progress.py` change, no YAML present)
**never** fails the release — the tag and Release publish regardless.

## Seeding

The sequence starts from an initial **`v0.1.0`** tag on `master` HEAD, created
once by a maintainer as a seed step **outside** this workflow:

```sh
git tag -a v0.1.0 -m v0.1.0 <master-HEAD-sha> && git push origin v0.1.0
```

The first merge/push to `master` after the release automation lands bumps it to
`v0.1.1` (or the labeled minor/major). The workflow never creates `v0.1.0`
itself.
