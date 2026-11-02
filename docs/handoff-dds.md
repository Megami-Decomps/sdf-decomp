# Handoff: stages 4 and 5 on the dds-decomp side

Work is in the worktree `/home/wsl/projects/dds-decomp-sdf`, branch `sdf-shared`
(from dds-decomp origin/main 648e39e3c). Nothing is pushed. `orig/dds1`,
`orig/dds2`, `tools/bin`, `tools/compilers`, `tools/glibc32` are symlinks to the
main checkout and show as untracked; never `git add -A` there. Python is
`/home/wsl/projects/dds-decomp/.venv/bin/python`.

## Done

Stage 4, on `sdf-shared`:

- 5eb4644a1 `tools/split_unit.py`: keeps an enum a part uses only through its
  constants, and recognises `INCLUDE_ASM(void, ...)`.
- 3efd70980 hand edit: `sdfSndMap` is `extern struct SndWork` in DDS1
  `code_002E9708` and DDS2 `code_003425B0`; their `extern u8 sdfSndMap[]`
  clashed with sdfSound's declaration once merged. Codegen unchanged.
- 33fd459fb `tools/resplit.py` plus `config/<v>/sdf_units.txt`, the input.
- 59fbe48c6 the generated re-split, both games: 46 units written, 12 removed
  per game. (An earlier 6500d16f3 left out eight units the tool had
  truncated and only built from the working tree; it was regenerated.)

Stage 5, on `sdf-shared`:

- ef754e276 22 SMT3 names per game, applied with `names.py apply` and one
  `rename_refs.py` run, plus two exclusions in `config/smt3_names_exclude.txt`.

Verified at 59fbe48c6 and again at ef754e276, each from a clean checkout: `ninja` exit 0 with both check lines, check_unit clean
(N match, 0 differ, no flag lines) on all 42 written units per game,
`run_tests.py` (442 tests), `check_nonmatching.py` (89 units), `ninja dds1-dev
dds2-dev`, the four reports, `progress.py`, and CI's report scope check.

In sdf-decomp: `tools/twins.py runs` (each SMT3 unit's DDS address range, by
sequence alignment of the hashes, with DDS-only functions inside and unassigned
functions between runs) and `renames` (stage 5 pairs).

### Units split

40 of SMT3's 45 per game: sdfDefaultMain, sdfGsMemory, sdfImageSet, sdfTex,
sdfGsMemIO, sdfDelayedResrc, sdfPacketBuff, sdfDraw, sdfPrim, sdfModel, sdfMat,
sdfMotion, sdfVu0, sdfVu1 (first part only, below), sdfCoreDraw, sdf2D, sdf3D,
sdf3DPrim, sdfBillBoard, sdf_PadManager, sdfDevCons, sdfPrint, sdfCd, sdfFile,
sdfFileIO, sdfHdd, sdfDev9 (first part only), sdfMisc, sdfMath, sdfQuat,
sdfRand, sdfSound, sdfDevFile, sdfAddrResolve, sdfIpu, sdfMovie, sdfPacDecode,
sdf_Decomp, sdfTmx, sdfSortOt.

### Not split

- sdfSystem, sdfChip, sdfMemory, sdfIop, sdfGraph (DDS1 0x002CF438-0x002D16F0,
  DDS2 0x003282E8-0x0032A5A0). `config/<v>/devbuild.json` replaces and moves
  six objects there (thread and memory state) with pinned symbol, relocation
  and GP-reference counts, and a replacement may not use INCLUDE_RODATA.
  Re-cutting them means rewriting that descriptor. Also DDS1 `code_002CF530`
  (inside sdfSystem) is built with `-fno-optimize-sibling-calls` while the rest
  of sdfSystem's run sibcalls; SMT3 matches its four functions without the flag
  by returning s32, so those need SMT3's C forms first.
- sdfVu1 and sdfDev9 are interrupted by hand-written assembly that DDS keeps as
  its own `hand/` segments (`hand/vu/sdfVu*` x4 inside sdfVu1,
  `hand/mem/sdfMemClearBlocks` inside sdfDev9). SMT3 holds the same functions
  as INCLUDE_ASM inside the C unit. Folding them would move hand-written code
  into the game-code totals, a dds-decomp policy call. The C after each hand
  segment keeps its `game/code_<vram>` name.

### Functions between two runs, decided

By the SMT3 function in the same slot (unaligned in `twins.py runs`) or, where
SMT3 has none, by what the function touches:

| DDS function | unit | why |
|---|---|---|
| sdfLoadModulesAndBindNamedResourceRpc | sdfIop | SMT3's only sdfIop function `SdfIopInit` sits there |
| sdfSetForcedAssetTextureMode | sdfMat | sets the sdata variable sdfMat's own functions read, already in sdfMat's sdata |
| sdfPadInitManager | sdf_PadManager | same name in SMT3, last function of the unit |
| sdfInitDeviceSemaphores | sdfCd | SMT3 `sdfCdInit` ends sdfCd |
| sdfPowerOffLoop, sdfDevInitializeHddPfsDebugMode | sdfHdd | SMT3 `sdfHddPowerOffTh`, `sdfHddInit` |
| sdfItemArrayGrow, sdfDevResizeBufferedRequest | sdfMisc | SMT3 `sdfItemArrayGrow`, `func_002BFF98` |
| dds3GetRandA, dds3GetRandFloatA, dds3GetRandIntA | not sdf | SMT3 `code_002C0BA8`, between sdfQuat and sdfRand |
| sndReserveIopWorkMemory, sndTryDispatchQueuedCommand | sdfSound | SMT3 `sdfSndIopAlloc`, `sdfSndCmdTrans` open sdfSound |
| sdfReadPackedResourceAndRelocateHeader | sdfDevFile | a load-and-relocate like sdfDevLoadMotion; sdfAddrResolve is one function |

### Renames applied (both games)

From `twins.py renames`, 58 identical twins plus 2 skipped per game. Applied:
sdfChipAllocZeroDirect, sdfMemAlloc2Direct, sdfMemSearchHandle,
sdfGrInitNewFrame, sdfGsMemCheckMerge, sdfDrawTagLinkRef, sdfGrSetFrameMaskRGB,
sdfGrSetFrameMaskReset, sdfModelOtInit, sdfModelGetExtraChunk, sdfDevSetPrintCB,
sdfFileCmdFileSize, sdfHddPowerOffHandler, p2sdPlaySeDeg, sdfFixupModelBinary,
sdfFixupMotionBinary, sdfIpuStartDecodeTask, movieAttachDecodeTask,
sdfPacNewBlock, sdfPacEndBlock, sdfPacTexListDecoderTmxBodyEndFunc,
sdfPacTexListDecoder1. Each name's note gives the SLUS_209.11 address and the
DDS name it replaced.

### Renames withheld

- SMT3 placeholder names (address tags such as `sdf_2A8258`, `p2sd_*`,
  `movie_2C5080`): 40 across both games.
- Already in `config/smt3_names_exclude.txt`: 22.
- `sdfAddHandler` to `DmacAddHandler`: the DDS name has `evidence` in
  `config/<v>/name_sources.txt`, which outranks an SMT3 name.
- `dds3EffectFileLoad`: two SMT3 twins, `sdfDevLoadTex` and `sdfDevLoadTexList`.
- `sdfChipCheck`: collides with `s32 sdfChipCheck(s32 address)` in
  `include/sdf_chip.h`.
- `sdfDrawTagLink2` (DDS1 `sdfDrawTagLinkRef2`): its declaration tips DDS1
  `game/code_002BC8F0` (sdfDrawTexturedQuad) into CONTEXT; excluded in both
  games so the twins keep one name.

## Next

Nothing on this side. `sdf-shared` sits on origin/main 648e39e3c, unpushed.

## Regenerating on a newer base

The generated commits must be redone, not rebased:

```sh
git rebase --onto origin/main 648e39e3c sdf-shared   # or cherry-pick the first three commits
git checkout -- src config && git clean -fq src      # drop the old generated output
.venv/bin/python configure.py --force-split           # the tool reads the old layout's asm
.venv/bin/python tools/resplit.py dds1 config/dds1/sdf_units.txt
.venv/bin/python tools/resplit.py dds2 config/dds2/sdf_units.txt
.venv/bin/python configure.py --force-split && ninja
```

Then check_unit every unit the yaml diff adds, and stage the yaml, the written
units, the deleted ones and every kept unit whose range the yaml diff changed
(resplit truncates those in place). Then confirm from a clean checkout
(`git checkout -- . && git clean -fq src`, configure, ninja). Do not stage the rest: `configure.py` on current main rewrites
about 178 other sources (INCLUDE_RODATA spacing, trailing newlines), which
must stay out of the commit. If resplit prints CONFLICT, two merged units
declare one name differently; fix the old unit by hand in a commit before the
generated one, as 3efd70980 did.

## Traps

- resplit reads `asm/<v>/<unit>.s` of the old units for INCLUDE_RODATA homing
  and data ownership. Run it after a configure of the old layout, never twice.
- A data definition after a unit's last function travels with that function in
  split_unit; resplit moves it to the first part that uses it
  (`sdfFileHostRootPath` would otherwise land in sdfDev9).
- Top-level `#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif` regions are
  carried whole; split_unit alone would scatter their lines.
- The renames: reset the tree, `names.py apply <v> <names.tsv>` per game,
  then one `rename_refs.py <dds1.tsv> <dds2.tsv>`, then `git status` before
  configure to get the file list. The name lists come from
  `twins.py renames` filtered by docs/smt3-names.md's rules.
- The dev build (`ninja dds1-dev dds2-dev`) is the check that catches a unit
  change touching its six objects; plain `ninja` passes.
