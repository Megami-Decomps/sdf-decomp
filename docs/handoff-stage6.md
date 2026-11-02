# Handoff: stage 6, DDS compiles shared sdf units

## Done

- Step 0, sdf-decomp caught up with smt3-decomp origin/main (0be37933): its
  three sdf commits replayed with author and date kept, 981c4d8 (58da2042),
  9076f4d (db882c2f), ebbd229 (cfd84fef). All 45 units, the vu sources and the
  14 headers equal smt3-decomp origin/main byte for byte. The ps2asm.h half of
  58da2042 stays in smt3-decomp.
- The SMT3 clone (`/home/wsl/projects/sdf-decomp-work/smt3-decomp`) is now
  0be37933 plus the consume commit, rebased (aa1165e0 before the submodule
  bump). From an empty build/: `[860/861] check config/smt3/checksum.sha1`,
  ninja exit 0, SHA-1 5facfbf7916836613449c922e26a4ec412207b32.
  `docs/smt3-decomp-consume.patch` is regenerated from it.
- Step 1, dds-decomp consumes the submodule (worktree
  `/home/wsl/projects/dds-decomp-sdf`, branch `sdf-shared`), 8badbeb49:
  - `config/sdf_shared.txt` lists moved units by name (`sdf/sdfRand`), both
    games. `tools/sources.py` maps `src/<v>/<unit>.c` to
    `external/sdf-decomp/src/<unit>.c` for those.
  - Every unit gets `-DSDF_DDS1` or `-DSDF_DDS2` (both games stayed
    byte-identical with it). Only a shared unit gets
    `-Iexternal/sdf-decomp/include`, ahead of `-Iinclude`: DDS has its own
    `libsdf.h`, `sdf_resource.h`, `sdfCorePacketCb_t.h` and
    `sdfCorePacketPointBuff_tag.h`, so a global sdf-decomp include would
    shadow them for every DDS unit.
  - `tools/splat_split.py` (smt3-decomp's) keeps splat from writing stubs for
    moved units. `cc.sh` compiles a shared unit under a scratch name as long as
    `external/sdf-decomp/src/<unit>.c`, the build's input, since ee-gcc output
    can depend on name lengths. check_unit, check_nonmatching, progress and
    production_report go through `sources.py`. check_unit needs the logical
    `src/<v>/<unit>.c` path even for a shared unit.
  - `.gitmodules` URL is the absolute local path; it must become
    `../sdf-decomp` once sdf-decomp is pushed.
  - Verified: configure --clean, ninja exit 0 with both check lines,
    `ninja dds1-dev dds2-dev`, run_tests.py 442 OK.

## Units

| unit | status |
|---|---|

## In progress

## Next

## Traps

- `configure.py` rewrites about 123 DDS sources (INCLUDE_RODATA and
  INCLUDE_SDATA spacing); never stage them.
- `include_rodata.py` and `include_sdata.py` walk only `src/<v>`, so they never
  rewrite a shared unit (good: their output is per game). A shared unit's
  INCLUDE_RODATA and INCLUDE_SDATA lines are kept by hand under `#ifdef`, and
  the per-symbol asm files those tools write for a unit are not written for a
  shared one. A unit that needs them needs those tools taught first.
