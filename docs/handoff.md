# Handoff

## Done

- Stage 1, seed. History is smt3-decomp's (at its b9136570) filtered to the sdf
  units and 14 engine headers, 225 commits, then the plan commit on top.
  `docs/layout.md` lists what moved and why. Files match smt3-decomp b9136570
  byte for byte.

## In progress

- Stage 2: smt3-decomp clone consuming this repo.

## Next

- Stage 2, then stage 3 (`tools/twins.py`).

## Work area

`/home/wsl/projects/sdf-decomp-work/` holds the filter-repo venv (`fr/`), the
filtered clone (`seed/`) and the path list (`paths.txt`). Rebuilding the seed:
`git clone --no-local` smt3-decomp, then
`fr/bin/git-filter-repo --paths-from-file paths.txt --path-rename src/smt3/sdf/:src/sdf/`
plus one `--path-rename` per `src/smt3/code_*.c` line in `paths.txt`.

## Traps

- filter-repo turns a `--no-local` clone's remote branches into local ones; only
  `main` was fetched here.
