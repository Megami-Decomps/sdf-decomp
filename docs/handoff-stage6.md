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
- Data names (dds-decomp ac466b040): DDS1 `D_00398938`/DDS2 `D_0040BAE8` to
  `sdfDefaultRandCB`, DDS1 `D_003BD480`/DDS2 `D_00438B70` to `sdfCdClock`.
  `names.py apply` needs a symbol_addrs row, so a splat-only label first got a
  bare `D_X = 0xX;` row above `// BEGIN shared`, then `names.py apply <v>`
  by address and `rename_refs.py` with the D_ names. check_unit on the ten
  units that name them: unchanged (code_002E7C20 and its DDS2 twin keep their
  earlier `.lit4` DATA line).
- dds-decomp e3fc94014: the placement tools also resolve a C definition's name
  through the unit's `#define` (sdfRand defines `D_005519B8` for DDS).
- DDS headers (dds-decomp c07ed90ed): `include/ps2asm.h`, `sif_rpc.h`,
  `sdk_libmrpc.h`, `libdma.h` copied from smt3-decomp 0be37933 (no later
  upstream change to them), and `common.h` replaced by SMT3's, which is DDS's
  plus `QUAD_ZERO`, `PS2_SYMBOL_AT` and the `FAKEMATCH_*` macros. check_unit
  on all 536 DDS units (both games, shared ones included) gave the same
  result line before and after; the four that differ did before too
  (devbuild in both games, code_002E7C20 and code_00340AC8's `.lit4` DATA).
  `pipe.h` is not copied: only sdfSystem includes it, and it includes
  `model.h` and `libsdf.h`.
- Overlap, not merged: DDS's `sdf_vu0.h` defines 96 macros, 95 of them also in
  `ps2asm.h` with identical bodies; `pcp_vu0.h` defines 123, 37 shared, 34
  identical. 121 DDS1 units include one of the two. A unit including both
  `ps2asm.h` and either would see the three differing redefinitions.
- More data names: DDS1 `D_00398580`/DDS2 `D_0040B730` to
  `sdf3DPrimIdentityTm`, DDS1 `D_00398500`/DDS2 `D_0040B6B0` to
  `sdf3DPrimDefaultFaceIndex` (5b6517731).

- SDK prototypes (step 1 of the last Next): SMT3's units declare SDK
  functions locally (`extern void SleepThread(void)`, `extern void
  EIntr(void)`, `sceSifSetRpcQueue` returning s32) where DDS's
  `sdk_libkernl.h` has one canonical prototype each. The shared units define
  `SDK_SKIP_<name>` under the DDS games before the include (sdf-decomp
  41f34ae, 4f6da3c); sdfPrint, sdfSound and sdfTmx also declare SMT3's
  `sceSifRpcFunc` typedef and sdfCd `sceCdlFILE` there. All 9 units that
  failed to compile now do. Conforming DDS's SDK headers was tried and
  reverted: see Traps (code_002DE248).
- dds-decomp 144675144 takes SMT3's `eeregs.h` (macros only: `HW_FIELD_*`,
  `EE_UNCACHED*`, D0 registers).
- dds-decomp ec8afa786, `tools/c_aliases.py`: `function_aliases` also
  follows `#define smt3name ddsname`, so check_unit, include_rodata and
  include_sdata see a shared function under the DDS name it compiles to.
  `test_c_aliases.py`.
- Renames to SMT3's names, both DDS games, through `names.py apply` and
  `rename_refs.py` (bb4f38c26: 23 symbols, 9ee9c2b95: `sdfCoreDrawReset`,
  81988f325: `Mult`, `ToMatrix`, `FromEuler`). Some DDS names were wrong:
  DDS's `sdfCreateSemaphoreFromOptions` is the empty `sdfDummyFunc`.
  `sdf_chip.h` kept a stale `sdfChipCheck(s32)` prototype; it now matches
  `sdf_chip_range_api.h`.

## Units moved

22 units, as listed in dds-decomp's `config/sdf_shared.txt` (the earlier
report of 11 was wrong; there were 9 before this session). Each has DDS's
copies deleted, check_unit clean in both games, all three games byte-identical.

| unit | sdf-decomp | dds-decomp | what the shared file needed |
|---|---|---|---|
| sdfAddrResolve | 56101ca | 9f64e077a | drop unused `ps2asm.h` include |
| sdf2D | 80fdb48 | 8605346ab | same |
| sdfMath | dea183c | 9eba07f73 | same, and `#define D_0052D944 sdfNormalizedAsinSamples` for DDS |
| sdfPrim | 0ff30f4 | 184d8dae5 | same, and `D_00529388` defined to DDS1 `D_003980E0`, DDS2 `D_0040B290` |
| sdfRand | e38c6d7 | 65a71918c | unused `ps2asm.h` dropped; `D_00529BB0`, `D_005519B8` defined per game to DDS placeholders, and DDS defines the clock cache (`sdfClock_t D_005519B8 = {0};`) in the unit |
| sdfBillBoard | none | 5368a2aa7 | nothing once DDS had `ps2asm.h` |
| sdf3DPrim | none | f813b3fc0 | the two data renames above |
| sdfPacketBuff, sdfDelayedResrc | 6e1669c | 546ff6a5a | `D_00551870` defined to `sdfPacketBuffers`, `D_005520A8` to `sdfPendingQueueSlots`; DDS's INCLUDE_SDATA lines at the end under `#if defined(SDF_DDS1) \|\| defined(SDF_DDS2)` |
| sdfSortOt | 41f34ae | 476696ca4 | `SDK_SKIP_SleepThread` for DDS |
| sdfGsMemory | none | 17ab14a35 | `sdfGsErrorHandler` rename (bb4f38c26) |
| sdfImageSet | 013e0c8 | 50dd0162f | `SDK_SKIP_SleepThread`, `sdfChipCheck` rename, `D_005293A0` per game (DDS1 `D_00398100`, DDS2 `D_0040B2B0`) |
| sdfCoreDraw | 3320a1d | 05bbc529b | renames (bb4f38c26, 9ee9c2b95 `sdfCoreDrawReset`), DDS's three INCLUDE_SDATA lines at the end under the DDS games |
| sdfGsMemIO | d8a7357 | a3a084442 | `D_00529388` as sdfPrim; SMT3's 27 INCLUDE_SDATA lines under `#ifdef SDF_SMT3` (DDS places that sdata in other units) |
| sdfQuat | cb30f8a | c5884f320 | four constants per game, `#define sdf_2C0510 effMiscQuatLengthSqVU`, renames `Mult`, `ToMatrix`, `FromEuler` (81988f325) |
| sdfDraw | 53d9599 | a337a9853 | `D_0055188A`, `D_00551888` defined to DDS names, `#define sdfGrOtClose sdfFlushPoolNodes` (see Traps), DDS's INCLUDE_SDATA lines at the end, one per game |
| sdfDevCons | 62ea4f6 | a337a9853 | `#define func_002BD300 sdfPktSetCmd`; DDS's `sdfDevConsFontUvTable` and `sdfDevConsColorTable` definitions under the DDS games |
| sdfIpu | 1a3eb57 | e639d4d22 | SDK skips, `D_00552158` and `D_00552144` defined, `#define sdfIpuRunDecode sdfAdvanceStreamPlayback`, DDS's own `sdfIpuRunDecode` body (pause checks) under the DDS games, its INCLUDE_SDATA line per game |
| sdf_Decomp | 5a2fdfe | 3ffd46925 | `D_00594780`, `D_00552170` per game; `sdfDrawFaceListParam` defined as `sdf3DScreenParam + 0x70` for DDS, which has no symbol there |
| sdfPrint | 43887e8 | 43397b6bb | rename to `sdfPrintIopInit` (4844a7423); DDS's body under the DDS games (it retries the bind instead of printing); `D_0058F310`, `D_00551908` defined; SMT3's INCLUDE_RODATA and INCLUDE_SDATA under `SDF_SMT3`, DDS's two per game |
| sdfMovie | 346ea33 | c72f8d6c6 | DDS-only `sdfSetStreamFrameDrawMode`, `sdfSetGridScaledDrawBounds`, `sdfMovieQueueFrameSprite` (INCLUDE_ASM plus DDS's NON_MATCHING draft) after `movieWaitSema`; the draw-mode check in `release_movieobj2` under the DDS games; sdata lists per game |
| sdfTmx | 1180181 | 00bf3e064 | renames `mcModuleNames`, `sdfFileRpcBuf`, `sdfFileRpcState`, `sdfFileRpcThreadId` (4651fd56b); DDS compiles the two module paths from literals, so `D_0054A360`, `D_0054A380` are defined to them; rodata lists per game |

Also dds-decomp d00cc006d: `include_sdata.py` writes a shared unit's
per-symbol asm files and leaves its source alone.

Final state, verified from committed state (sdf-decomp cb30f8a in both
submodules, dds-decomp c5884f320): SMT3 clone `configure.py --clean`, ninja
exit 0, `[860/861] check config/smt3/checksum.sha1`,
5facfbf7916836613449c922e26a4ec412207b32. DDS `configure.py --clean`, ninja
exit 0 with both check lines, DDS1 6d18898e2724bf1d145392766e8ba1e678487419,
DDS2 9be91ee1b4a535a4cb6ec89237b5a6ba41be2add. check_unit over every DDS unit
(shared ones by logical path) after each rename batch: only the four known
differences (devbuild in both games, and the orphans code_002E7C20 and
code_00340AC8, see Traps).

## Units not moved

Probe after the headers landed (unit listed in `sdf_shared.txt`
temporarily, check_unit in both games; DDS1 and DDS2 agree unless noted):

| unit | probe | first blockers |
|---|---|---|
| sdfVu0 | 34 match, 9 differ | MISSING DDS functions (`sdfVuLoadSecondaryRotationMatrixY`), NOASM |
| sdfTex | 29 match, 17 differ | MISSING DDS function names (`sdfTexDestroy`, `sdfTexReleaseReference...`) |
| sdf3D | 7 match, 17 differ | UNDEF `vu1Tex0Slot0..2` and more names |
| sdfMisc | 1 match, 6 differ | ORDER, MISSING DDS-only functions (`sdfDevCreateBufferedRequest`, `sdfDevResizeBufferedRequest`) |
| sdfHdd | 1 match, 13 differ | UNDEF `sdfDev9Ready`, `D_0058FFC0`, `D_005519A0` |
| sdfDevFile | 5 match, 10 differ | MISSING `sndLoadNamedOffsetResourceList`, UNDEF `D_0054A2D8`, NOASM |
| sdfPacDecode | 28 match, 10 differ | ORDER, MISSING DDS-only `sdfPacResetOutputToAllocationEntry` |
| sdfMat | 35 match, 22 differ | UNDEF sdfTex function names (`sdfTexDup`, `sdfTexFreeDelayed`, `sdfTexSetClutColor`) |
| sdf_PadManager | 6 match, 9 (DDS2 10) differ | `D_005298E8` is DDS `sdfPadButtonPressure`, `D_0058F0C0` DDS `sdfPadPortBuffers` (owed to SMT3); UNDEF `sdfPadManagerMode`, `D_005298A0`, `D_005518D8`, `D_005518E0`; MISSING `sdfPadBuildButtonStates` |
| sdfDefaultMain | 0 match, 12 differ | UNDEF `sdfPadManagerMode`, `SdfIopInit`, `sdfCdTrayMode` |
| sdfModel | 46 match, 17 differ | UNDEF `ToMatrix`, `MultMatrixRev`, `sdfItemArrayNew` |
| sdfMotion | 85 match, 26 differ | UNDEF `ToMatrix`, `sdfMotModelVisibleTable`, `sdfMotModelLinerScaleTable` |
| sdfFileIO | 21 match, 17 differ | UNDEF `sdfFileCmdFreeList`, `sdfFileCmdFreeCount`, `sdfFileCmdDefaultDevice` |
| sdfFile | 11 match, 18 differ | UNDEF `sdfFileDefaultSrcType`, `sdfFileCmdFreeList`, `sdfFileCmdFreeCount` |
| sdfCd | 6 (DDS2 7) match, 31 differ | UNDEF `sdfCdTrayMode`, `sdfCdImageSearchFileDirect_ent`, `sdfCdImageFileName` |
| sdfSound | 29 match, 114 differ | UNDEF `sdfSndSendCmd_cmd_id`, `sdfSndMakeFilename`, `sdfFileRpcResult` |

UNDEF of an SMT3 real name usually means a DDS rename is owed (step 1 below);
UNDEF of a function name means stage 5 withheld or never paired it.
Not attempted: sdfSystem, sdfChip, sdfMemory, sdfIop, sdfGraph, sdfVu1,
sdfDev9 (stage 4 did not split them for DDS).

`/tmp/s7/probe.sh <unit>...` and `/tmp/s7/names.sh <unit>` (not committed)
wrap the probe and `/tmp/s6-names.py`, which printed, from check_unit -v, each
SMT3 name's DDS address and symbol: HI16/LO16 pairs give the address plus the
field offset, so check the base (sdfPrim's first guess was the object at +8).

## Next

1. Data names, per unit as it moves: where SMT3 has a real name and DDS a
   placeholder, rename in DDS as above rather than define. Where SMT3 has a
   placeholder and DDS a name, define it and add it to "Owed to smt3-decomp".
2. SDK prototypes: read the parse errors in sdfPrint, sdfCd, sdfSound and
   sdfTmx. Prefer making DDS's `sdk_libkernl.h` prototype match SMT3's where
   the retail call sites allow it (check_unit every unit including it, as
   for common.h); otherwise `#define SDK_SKIP_<name>` under the DDS games.
3. Move units in roughly the table's order: the ones blocked only by names
   first (sdfQuat, sdfGsMemIO, sdfDraw, sdfCoreDraw, sdfDevCons), then the
   ones with DDS-only functions, which need their DDS bodies under `#ifdef`.

## Owed to smt3-decomp

SMT3 placeholders that DDS names. A shared unit maps each with a `#define`
under the DDS games; the define goes when smt3-decomp takes the name.

| SMT3 | DDS name | unit |
|---|---|---|
| `D_0052D944` | `sdfNormalizedAsinSamples` | sdfMath |
| `D_00551870` | `sdfPacketBuffers` | sdfPacketBuff |
| `D_005520A8` | `sdfPendingQueueSlots` | sdfDelayedResrc |
| `D_005298E8` | `sdfPadButtonPressure` | sdf_PadManager (not moved) |
| `D_0058F0C0` | `sdfPadPortBuffers` | sdf_PadManager (not moved) |
| `sdf_2C0510` | `effMiscQuatLengthSqVU` | sdfQuat |
| `func_002BD300` | `sdfPktSetCmd` | sdfDevCons |
| `D_00551888` | `sdfGrFrameComplete` | sdfDraw |
| `D_0055188A` | `sdfGrBufferSlotIndex` | sdfDraw |
| `D_00552158` | `sdfIpuInputStream` | sdfIpu |
| `D_0058F310` | `sdfDevRpcClient` | sdfPrint |
| `movie_2C5080` | `sdfPrintTextLine` | sdfMovie |
| `release_movieobj2` | `sdfMovieDrawFrame` | sdfMovie (IDB name with a one-argument `dds3MovieTable_t` prototype; the function takes the movie and an OT) |

Also owed: SMT3's units declare SDK functions locally with types that differ
from the SDK (`void SleepThread`, `void EIntr`, `s32 sceSifSetRpcQueue`).

## Traps

- `INCLUDE_SDATA` and `INCLUDE_RODATA` take the symbol name literally (the
  asm path is built from it), so a `#define` alias does not reach them: list
  the DDS names per game.

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
- `/tmp/s7-verify.sh` builds all three games from `--clean` with the
  submodules as checked out (no copying).
- Verification: `/tmp/s6-verify.sh` copied sdf-decomp's working `src/` and
  `include/` into both submodules, ran ninja in the SMT3 clone and configure
  plus ninja in the DDS worktree (`CLEAN=1` for --clean). After committing,
  each submodule was reset and checked out at the sdf-decomp commit.
- The SMT3 clone's `external/sdf-decomp` is checked out at the sdf-decomp tip
  but its superproject commit still pins ebbd229; that clone is for
  verification only.
- Never add an identifier (typedef, struct tag, prototype) to a DDS header
  most units include: DDS2 `game/code_002DE248.c` includes `sdk_ps2.h` and
  everything else, and its `sdfDrawTexturedQuad` changes register allocation
  when cc1's identifier table grows (a bare `struct zzqq;` in
  `sdk_libcdvd.h` was enough; a comment or macro was not). dds-decomp's
  `rename_refs.py` says the same. Put DDS-only declarations in the shared
  unit under the DDS games instead.
- `game/code_002E7C20.c` (DDS1) and `game/code_00340AC8.c` (DDS2) are
  orphans: no yaml entry, not built. They are old copies of sdfQuat and the
  source of their check_unit difference.
- A rename needs `configure.py` before check_unit sees the new name in the
  split asm.
- `names.py apply` renames in every DDS source, including files configure
  has just rewritten; stage such a file as HEAD plus the renames, not as the
  working copy (`/tmp/s8/fixmixed.py`).
- SMT3's `check_inline_asm.py` rejects `__asm__` in a shared unit even inside
  a DDS-only branch; alias a function with `#define` instead.
- DDS1 `game/code_001DEFE0.c` (`btlBuildPresetCameraPose`) shifts when an
  identifier in a header it includes changes length: renaming
  `sdfFlushPoolNodes` to `sdfGrOtClose` and `sdfAdvanceStreamPlayback` to
  `sdfIpuRunDecode` (both in `sdf.h` or `sdf_packet_list.h`) failed DDS1.
  New names of 16 or more characters were fine. DDS keeps its names and the
  shared unit aliases them with `#define`.
