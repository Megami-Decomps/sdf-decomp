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

## Units moved

Each is in `config/sdf_shared.txt`, DDS's copies deleted, check_unit clean in
both games, all three games byte-identical.

| unit | sdf-decomp | dds-decomp | what the shared file needed |
|---|---|---|---|
| sdfAddrResolve | 56101ca | 9f64e077a | drop unused `ps2asm.h` include |
| sdf2D | 80fdb48 | 8605346ab | same |
| sdfMath | dea183c | 9eba07f73 | same, and `#define D_0052D944 sdfNormalizedAsinSamples` for DDS |
| sdfPrim | 0ff30f4 | 184d8dae5 | same, and `D_00529388` defined to DDS1 `D_003980E0`, DDS2 `D_0040B290` |

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
| sdfPacketBuff, sdfDelayedResrc | text clean once one SMT3 placeholder is defined to its DDS name (`D_00551870` to `sdfPacketBuffers`, `D_005520A8` to `sdfPendingQueueSlots`) | DDS owns their .sdata (INCLUDE_SDATA lines); see Next 1 |
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

1. Conditional-aware INCLUDE scanning. A shared file's per-game
   `INCLUDE_ASM`/`INCLUDE_RODATA`/`INCLUDE_SDATA` lines sit under
   `#if defined(SDF_DDS1) || ...`, but both configures scan them as text:
   SMT3's ninja failed with `asm/smt3/nonmatchings/sdf/sdfDelayedResrc/sdfDelayedResrcSemaId.s
   missing` when DDS's lines were added. Needed in both repos: resolve the
   `SDF_*` conditionals before scanning (configure's `include_directives`,
   include_rodata, include_sdata, check_unit, progress, eeasm inputs). For
   SMT3 that is a change to the consume patch. Then move sdfPacketBuff and
   sdfDelayedResrc (both already probe clean with the defines above; their
   DDS INCLUDE_SDATA lists are in the deleted DDS files).
2. `include_rodata.py` still walks only `src/<v>`: a shared unit with DDS
   rodata gets no per-symbol files. Teach it as `include_sdata.py` was.
3. Data names: where SMT3 has a real name and DDS a placeholder (`sdfCdClock`,
   `sdfDefaultRandCB`), rename in DDS (names.py, rename_refs) rather than
   define. Where SMT3 has a placeholder and DDS a name (`D_0052D944`), the
   name is owed to smt3-decomp (list it in its upstream file); the `#define`
   goes when SMT3 takes it.
4. DDS headers: a DDS `ps2asm.h` (copy of smt3-decomp's, a dds-decomp
   decision), the SDK headers above, and SMT3's local SDK prototypes guarded
   or matched to DDS's.

## Traps

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
