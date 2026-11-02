# Handoff: stage 6, DDS compiles shared sdf units

## Done

- Step 0, sdf-decomp caught up with smt3-decomp origin/main (0be37933): its
  three sdf commits replayed with author and date kept, 981c4d8 (58da2042),
  9076f4d (db882c2f), ebbd229 (cfd84fef). All 45 units, the vu sources and the
  14 headers equal smt3-decomp origin/main byte for byte. The ps2asm.h half of
  58da2042 stays in smt3-decomp.
- The SMT3 clone (`/home/wsl/projects/sdf-decomp-work/smt3-decomp`) is now
  0be37933 plus the consume commit, rebased and amended (01d13e0d, pin not
  bumped). From an empty build/: `[860/861] check config/smt3/checksum.sha1`,
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

- Conditional-aware scanning, both repos (sdf-decomp 9928d13 for the
  consume patch, dds-decomp c6ad04af2). `tools/sources.py` has
  `game_source(text, version)`: it blanks the branches a game's build skips
  under `#if`/`#ifdef` on `SDF_SMT3`/`SDF_DDS1`/`SDF_DDS2`, keeps offsets,
  keeps every branch of any other conditional, and raises on a condition that
  mixes game macros with other terms. Used by configure (eeasm inputs, the
  SMT3 split digest, the DDS dev replacements), include_rodata, include_sdata,
  check_unit, progress and SMT3's check_nonmatching drafts. `test_sources.py`
  in both.
- include_rodata and include_sdata, both repos, walk shared units too and
  never write them: they compute the placement on the game's view and fail
  configure if it differs from the file (whitespace aside; the rodata pass
  ignores INCLUDE_SDATA lines, which the sdata pass places next). DDS's two
  also follow object-like `#define ALIAS name` lines, so a function naming
  `D_00551870` counts as naming `sdfPacketBuffers`.

## Units moved

Each is in `config/sdf_shared.txt`, DDS's copies deleted, check_unit clean in
both games, all three games byte-identical.

| unit | sdf-decomp | dds-decomp | what the shared file needed |
|---|---|---|---|
| sdfAddrResolve | 56101ca | 9f64e077a | drop unused `ps2asm.h` include |
| sdf2D | 80fdb48 | 8605346ab | same |
| sdfMath | dea183c | 9eba07f73 | same, and `#define D_0052D944 sdfNormalizedAsinSamples` for DDS |
| sdfPrim | 0ff30f4 | 184d8dae5 | same, and `D_00529388` defined to DDS1 `D_003980E0`, DDS2 `D_0040B290` |
| sdfPacketBuff, sdfDelayedResrc | 6e1669c | 546ff6a5a | `D_00551870` defined to `sdfPacketBuffers`, `D_005520A8` to `sdfPendingQueueSlots`; DDS's INCLUDE_SDATA lines at the end under `#if defined(SDF_DDS1) \|\| defined(SDF_DDS2)` |

Also dds-decomp d00cc006d: `include_sdata.py` writes a shared unit's
per-symbol asm files and leaves its source alone.

Final state, verified: DDS from `configure.py --clean`, ninja exit 0 with
both check lines (DDS1 6d18898e2724bf1d145392766e8ba1e678487419, DDS2
9be91ee1b4a535a4cb6ec89237b5a6ba41be2add), `ninja dds1 dds2 dds1-dev
dds2-dev`, the four reports, progress.py unchanged (DDS1 9501/9573, DDS2
10767/10855), CI's report scope check, run_tests.py 442 OK,
check_nonmatching 89 units 0 failed. SMT3 clone: ninja exit 0,
5facfbf7916836613449c922e26a4ec412207b32, 27 tools/test_*.py pass,
check_nonmatching exit 0 (145 units).

## Units not moved

Probe (each unit listed in `sdf_shared.txt`, unused `ps2asm.h` dropped,
check_unit in both games), with what blocks it:

| unit | probe | blocker |
|---|---|---|
| sdfGsMemory | 14 match, sdfGsErrorHandler UNDEF | SMT3 INCLUDE_ASM name; DDS needs the rename or a name for it |
| sdfGsMemIO | 2 DIFF (D_00529388), NOASM, UNDEF | SMT3 INCLUDE_SDATA of `D_00551870` belongs to SMT3's split |
| sdfRand | 2 DIFF, 4 UNDEF | `sdfDefaultRandCB`, `sdfCdClock` (SMT3 names, DDS D_), `D_00529BB0`, `D_005519B8`; DDS defines the clock cache in C |
| sdfMisc, sdfHdd, sdfDevFile, sdfPacDecode, sdfMat, sdf_PadManager | MISSING/ORDER | DDS-only functions between or inside the run need `#ifdef` bodies |
| sdfDefaultMain, sdfImageSet, sdfTex, sdfDraw, sdfModel, sdfMotion, sdfVu0, sdfCoreDraw, sdf3D, sdf3DPrim, sdfBillBoard, sdfDevCons, sdfPrint, sdfCd, sdfFile, sdfFileIO, sdfQuat, sdfSound, sdfIpu, sdfMovie, sdf_Decomp, sdfTmx, sdfSortOt | does not compile for DDS | missing `ps2asm.h` (14 use its macros), `sif_rpc.h`, `sdk_libmrpc.h`, `libdma.h`, `pipe.h`; SMT3 local SDK prototypes (`SleepThread`, `sceGetstat`) conflict with DDS `sdk_libkernl.h`; SMT3 `common.h` macros (`FAKEMATCH_*`, `PS2_SYMBOL_AT`, `QUAD_ZERO`) DDS's `common.h` lacks |

Not attempted: sdfSystem, sdfChip, sdfMemory, sdfIop, sdfGraph, sdfVu1,
sdfDev9 (stage 4 did not split them for DDS).

`/tmp/s6-names.py <unit>` (not committed) printed, from check_unit -v, each
SMT3 name's DDS address and symbol: HI16/LO16 pairs give the address plus the
field offset, so check the base (sdfPrim's first guess was the object at +8).

## Next

1. Data names: where SMT3 has a real name and DDS a placeholder (`sdfCdClock`,
   `sdfDefaultRandCB`), rename in DDS (names.py, rename_refs) rather than
   define. Where SMT3 has a placeholder and DDS a name (`D_0052D944`), the
   name is owed to smt3-decomp (list it in its upstream file); the `#define`
   goes when SMT3 takes it.
2. DDS headers: a DDS `ps2asm.h` (copy of smt3-decomp's, a dds-decomp
   decision), the SDK headers above, and SMT3's local SDK prototypes guarded
   or matched to DDS's.

## Owed to smt3-decomp

SMT3 placeholders that DDS names. A shared unit maps each with a `#define`
under the DDS games; the define goes when smt3-decomp takes the name.

| SMT3 | DDS name | unit |
|---|---|---|
| `D_0052D944` | `sdfNormalizedAsinSamples` | sdfMath |
| `D_00551870` | `sdfPacketBuffers` | sdfPacketBuff |
| `D_005520A8` | `sdfPendingQueueSlots` | sdfDelayedResrc |

## Traps

- `git rm` stages the deletion: commit the DDS tool change before it, or
  the deletions ride along in the wrong commit.
- `configure.py` rewrites about 123 DDS sources (INCLUDE_RODATA and
  INCLUDE_SDATA spacing); never stage them.
- A unit can pass check_unit in both games and still fail the link: check_unit
  compares text, and data DDS defines (INCLUDE_SDATA, C definitions) that the
  SMT3 file only declares shows up as `undefined reference` at ld. Always run
  the full build.
- Defines are per game when DDS names are placeholders (they differ between
  DDS1 and DDS2); DDS real names are usually the same in both.
- Verification: `/tmp/s6-verify.sh` copied sdf-decomp's working `src/` and
  `include/` into both submodules, ran ninja in the SMT3 clone and configure
  plus ninja in the DDS worktree (`CLEAN=1` for --clean). After committing,
  each submodule was reset and checked out at the sdf-decomp commit.
- The SMT3 clone's `external/sdf-decomp` is checked out at the sdf-decomp tip
  but its superproject commit still pins ebbd229; that clone is for
  verification only.
