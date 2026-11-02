# Handoff

## Done

- Stage 1, seed (65666fc). History is smt3-decomp's (at its b9136570) filtered
  to the sdf units and 14 engine headers, 225 commits, then the plan commit on
  top. `docs/layout.md` lists what moved and why. Files match smt3-decomp
  b9136570 byte for byte.
- Stage 2, SMT3 consumes it, verified in a clone. `docs/smt3-decomp-consume.patch`
  is the change. Result in the clone, from `ninja -t clean`:
  `[861/861] check config/smt3/checksum.sha1`, ninja exit 0, image SHA-1
  5facfbf7916836613449c922e26a4ec412207b32. All 45 sdf units' compiler
  output is identical to the baseline except the `.file` line. All 27
  `tools/test_*.py` pass, `check_nonmatching.py` (145 units, 10 of them sdf)
  and `check_irx_nonmatching.py` pass, `progress.py` output and both objdiff
  reports (`report.json`, `report.all.json`) equal a baseline clone's, and the
  CI report scope check passes.

## In progress

- Stage 3, `tools/twins.py`.

## Applying the patch in smt3-decomp

The patch holds everything but the deletions, which would make it 1.1 MB.
From smt3-decomp at b9136570 or later, with sdf-decomp reachable at `../sdf-decomp`
relative to smt3-decomp's remote (on GitHub: Megami-Decomps/sdf-decomp):

```sh
git am /path/to/sdf-decomp/docs/smt3-decomp-consume.patch
git rm -r -q src/smt3/sdf include/{libsdf.h,sdf_types.h,sdf_resource.h,sdf_lighting.h,sdf_movie_types.h,sdfGsMemIO.h,sdfDevCons.h,sdfCorePacketCb_t.h,sdfCorePacketPointBuff_tag.h,model.h,motion.h,draw.h,draw3d.h,billboard.h}
git commit --amend --no-edit
git submodule update --init
```

Then bump `external/sdf-decomp` to the pushed sdf-decomp tip (the patch pins
65666fc, a local hash). If smt3-decomp has changed `src/smt3/sdf` or those
headers since b9136570, those commits must first land in sdf-decomp. The
`.gitmodules` URL is relative (`../sdf-decomp`), so a local clone of
smt3-decomp finds `/home/wsl/projects/sdf-decomp` and GitHub finds the sibling
repo. Checked: applying the patch plus the `git rm` to b9136570 gives the
verified tree exactly.

What the patch does:

- `tools/sources.py` maps a unit's `src/smt3/sdf/<unit>.c` name to
  `external/sdf-decomp/src/sdf/<unit>.c`. Unit names, objects
  (`build/smt3/src/smt3/sdf/*.o`), asm paths, objdiff names and reports do not
  change.
- `tools/splat_split.py` runs `splat split` with the C segment's source path
  mapped during scan and split only. splat has one `src_path` and would
  otherwise recreate missing units as stubs. The yaml is unchanged.
- `configure.py` puts `-Iexternal/sdf-decomp/include` before `-Iinclude`, adds
  `-DSDF_SMT3` to every unit, and fails if the submodule is empty.
- The tools the build and CI run go through `sources.py`: `include_rodata`,
  `include_sdata`, `vu_include`, `progress`, `check_nonmatching`,
  `check_unit`, `check_inline_asm`, `check_drift`, `production_report`,
  `cc.sh` (which repeats the mapping in shell), and three tests.
- CI checks out submodules; README and `docs/building.md` say so.

## smt3-decomp follow-ups, not in the patch

- About 45 other tools still walk `src/` or `include/` directly and miss the
  submodule: the `hd_*`, `ida_*`, `dds_*` families, `m2ctx.py`,
  `decompile.py`, `perm_unit.py`, `split_unit.py`, `names.py`,
  `inline_strings.py`, `find_similar.py`, `type_match.py`,
  `merge_duplicates.py`, `check_names.py`, `drift_report.py`. List them with
  `grep -lE '"src"|src/smt3|/ "include"' tools/*.py | xargs grep -L sources`.
  Each should take `c_sources()` and `SHARED_INCLUDES`.
- `include/README.md` and `tools/header_layout.c` still document the moved
  headers from smt3-decomp; the evidence text belongs in sdf-decomp.
- 19 docs mention `src/smt3/sdf/`; the logical name still holds, the file does not.
- `include_rodata.py` and `include_sdata.py` rewrite `INCLUDE_RODATA` and
  `INCLUDE_SDATA` lines inside C units, so a configure in smt3-decomp can now
  write into the submodule. It is idempotent today (the submodule stays clean).

## smt3-port, when it follows (do not change from here)

smt3-port at 47b4296 reaches smt3-decomp only as `external/smt3-decomp`:

- The submodule needs `--recursive`; the nested `../sdf-decomp` URL resolves
  against smt3-decomp's GitHub URL.
- `tools/stage.py` copies `src/` and `include/` from smt3-decomp. It must also
  copy `external/sdf-decomp/src/sdf` to `stage/src/smt3/sdf` (keeping patch
  paths valid) and `external/sdf-decomp/include` into the stage, or add it as
  an include directory.
- `tools/census_compile.py` takes `-I$DECOMP/include` and walks
  `src/smt3`; it needs the sdf-decomp include ahead of it, the sdf units, and
  `-DSDF_SMT3`.
- `tools/gen_data_host.py` reads `src/smt3/sdf/vu/*.vsm`.
- `tools/make_patches.py` globs `src/**/*.c` under smt3-decomp.
- `tools/measure_hwaddr.py` and `measure_bootpath.py` name or walk
  `src/smt3/sdf` paths.

sdfport, for stage 7: `CMakeLists.txt` decides `SDFX_HAVE_DECOMP` from
`$SDFX_SMT3DECOMP_TREE/include/sdf_types.h`, which moves, so it would quietly
switch the walk, player and tests off. `cmake/render.cmake` adds
`$SDFX_SMT3DECOMP_TREE/include`; it also needs
`external/sdf-decomp/include`. `cmake/native.cmake` includes smt3-port's
`cmake/decomp.cmake`, which smt3-port no longer has at 47b4296.

## Work area

`/home/wsl/projects/sdf-decomp-work/`:

- `fr/` venv with git-filter-repo; `seed/` the filtered clone; `paths.txt` the
  path list. Rebuild the seed with `git clone --no-local` of smt3-decomp, then
  `fr/bin/git-filter-repo --paths-from-file paths.txt --path-rename src/smt3/sdf/:src/sdf/`
  plus one `--path-rename` per `src/smt3/code_*.c` line in `paths.txt`.
- `venv/` has smt3-decomp's requirements.
- `smt3-decomp/` the consuming clone, its change committed as 22d112af;
  `smt3-base/` an unmodified clone built for comparison. Both have their own
  copies of `orig/` and `tools/{bin,compilers,glibc32}`, not symlinks, so no
  build output can land in the main checkout.

## Traps

- filter-repo turns a `--no-local` clone's remote branches into local ones; only
  `main` was fetched here.
- A failed link leaves the old `build/smt3/SLUS_209.11`, which still passes
  `sha1sum`. Read ninja's exit status.
- Python 3.12 and 3.13 `rglob` and `os.walk` do not descend into a symlinked
  directory, so a `src/smt3/sdf` symlink to the submodule would build but
  silently drop the units from every tool that walks `src/`.
- ee-gcc 2.96 output can depend on the length of the file names cc1 is given
  (`tools/cc.sh` notes it). The build now passes
  `external/sdf-decomp/src/sdf/X.c` instead of `src/smt3/sdf/X.c`; all 45 units
  still compile identically, but a unit edited later could differ between
  `check_unit.py` (which compiles under the canonical name) and ninja. If that
  shows up, compile shared units under a path of the canonical length.
