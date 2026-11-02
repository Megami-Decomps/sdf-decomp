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

- Stage 3, `tools/twins.py` (e3cee99). `table` and `contig` subcommands, stdlib
  only, `python3 -I`, `--smt3` and `--dds` default to the sibling checkouts.
  `--loose` reproduces the prototype scripts in
  `/home/wsl/projects/sdf-decomp-scratch/` exactly (`strict.py` table,
  `contig.py` flags, `units.py`'s 773). The default hash keeps struct offsets,
  as the plan says it does, and three plan numbers moved with it (a5101b0):
  - DDS1 twins in C: 767 of 777, not 773 (773 was the loose hash).
  - DDS-only functions: `sdfModel` has 2 and `sdfMat` 2. The loose hash paired
    fewer functions uniquely and so measured a shorter run.
  - Renames: 59, not 171. `units.py` compared against the first DDS1 twin by
    address; 111 of the 171 have a same-named twin too (small wrappers hash
    alike). 59 is the count with no same-named DDS1 twin.

## Current state (2026-11-02, stage 8 in progress)

- Stages 4 to 6: `docs/handoff-dds.md` (re-split, renames) and
  `docs/handoff-stage6.md` (shared units, traps, owed names). Stage 8:
  `docs/handoff-stage8.md`.
- 61 units are shared by all three games: SMT3's 45 `sdf/` units,
  `mc/mcManager`, `model/mdlManager`, `file/fileManager`, the three
  `script/` engine units, and `effect/` effPCPNeedle, polyManager, effPart,
  effPCPFlash, effPCPScatter, parManager, effMath, effPacket, effDebug and
  effSpline, as dds-decomp's `config/sdf_shared.txt` lists. `model/mdlEffect`
  is here for SMT3 only. dds-decomp branch `sdf-shared` in the worktree
  `/home/wsl/projects/dds-decomp-sdf` (5ee6b1f56). All three games
  byte-identical from clean builds of committed state.
- Branch `sdf-only` marks the end of stage 6 (the 45 `sdf/` units plus
  `mc/mcManager`) as a cut that can land on its own: sdf-decomp 575a81e,
  dds-decomp worktree 6b1c8ccb7.
- sdfVu1 and sdfDev9 hold DDS's former `hand/` segments as INCLUDE_ASM, as
  SMT3 does; `config/<v>/handwritten.txt` keeps those 19 functions out of
  DDS's game-code totals (`tools/progress.py`, `production_report.py`).
  SMT3 counts the same functions as game code and was left as it is.
- smt3-decomp main has no commits touching shared files since 98b9d355,
  the base of the current consume patch (rebased from 0be37933 on 2026-11-02;
  5353c126 and e7483423 touched polyManager and effEvent before they moved).
- sdf-decomp is pushed to github.com/Megami-Decomps/sdf-decomp (`main` and
  `sdf-only`, hashes unchanged, so existing pins stay valid). dds-decomp and
  smt3-decomp are not pushed: set dds-decomp's `.gitmodules` URL to
  `../sdf-decomp`, rebase onto their current main, rerun the checks, land both.
- Stopped 2026-11-02 on the user's request after effSpline, at a verified
  committed point. Next: the CI-equivalent checks, then effEvent (measured,
  plan in `docs/handoff-stage8.md`), effBlur_Filter, mdlEffect. `mc/slMain` and `script/scrParameterCommand` are game code and
  stay per game.

## Next

- Land the patch in smt3-decomp (a session there), after sdf-decomp has a remote.
  First replay into sdf-decomp the smt3-decomp commits since the seed that
  touch moved files. As of origin/main on 2026-11-02: 58da2042 (sdfVu1 inline
  asm into ps2asm.h macros), db882c2f (rotate comment signs in sdfVu0),
  cfd84fef (movieMemcpy at file scope under NON_MATCHING). Recheck with
  `git log b9136570..origin/main -- src/smt3/sdf <moved headers>`.
- `sdfGrOtTable` (0x002F5F18): splat declares 0xAA0, emits 0x5A0; smt3-port
  lost an interior alias at +0x600 to it. Size is being settled in
  smt3-port; the symbol stays in smt3-decomp's config, but the C that
  declares it is now here.
- Stage 4, DDS re-split: `twins.py contig -v` lists each flagged unit's DDS
  functions without an SMT3 twin, the candidates for the DDS-only code.
- Stage 5 can take its list from `twins.py`'s `Game.twin()`; no subcommand
  prints the rename pairs yet.

## Applying the patch in smt3-decomp

The patch holds everything but the deletions, which would make it 1.1 MB.
From smt3-decomp at b9136570 or later, with sdf-decomp reachable at `../sdf-decomp`
relative to smt3-decomp's remote (on GitHub: Megami-Decomps/sdf-decomp):

```sh
git am /path/to/sdf-decomp/docs/smt3-decomp-consume.patch
git rm -r -q src/smt3/sdf src/smt3/mc/mcManager.c src/smt3/model src/smt3/file/fileManager.c src/smt3/script/scrScriptProcess.c src/smt3/script/scrTraceCode.c src/smt3/script/scrCommonCommand.c src/smt3/effect/effPCPNeedle.c src/smt3/effect/polyManager.c src/smt3/effect/effPart.c src/smt3/effect/effPCPFlash.c src/smt3/effect/effPCPScatter.c src/smt3/effect/parManager.c src/smt3/effect/effMath.c src/smt3/effect/effPacket.c src/smt3/effect/effDebug.c src/smt3/effect/effSpline.c include/{libsdf.h,sdf_types.h,sdf_resource.h,sdf_lighting.h,sdf_movie_types.h,sdfGsMemIO.h,sdfDevCons.h,sdfCorePacketCb_t.h,sdfCorePacketPointBuff_tag.h,model.h,motion.h,draw.h,draw3d.h,billboard.h,fileManager.h,scr.h,effPCPData.h,effect.h}
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
  `external/sdf-decomp/src/sdf/<unit>.c`, and `src/smt3/mc/mcManager.c` to
  `external/sdf-decomp/src/mc/mcManager.c`. Unit names, objects
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
