# Handoff: stage 8, model/, file/, script/, effect/

dds-decomp work is on `sdf-shared` in `/home/wsl/projects/dds-decomp-sdf`; the
SMT3 check is the clone `/home/wsl/projects/sdf-decomp-work/smt3-decomp`, its
one consume commit amended per unit and exported to
`docs/smt3-decomp-consume.patch`. Nothing is pushed.

## Units

| unit | state | notes |
|---|---|---|
| `model/mdlManager` | shared, all three games | DDS-only `mdlBlendEntryPitchYawAndUpdate` (both) and `mdlSetViewerSlotResourceHandles` (DDS2) under the DDS macros |
| `model/mdlEffect` | SMT3 only | DDS re-split to its boundary; not shared, below |
| `file/fileManager` | shared, all three games | DDS2 prints in `pacLoadEx` and `fileLoadEx`; DDS defines its sdata and bss in C, SMT3 uses `INCLUDE_SDATA`/`INCLUDE_RODATA` |
| `script/scrScriptProcess`, `script/scrTraceCode` | shared, all three games | the script VM; DDS defines the command table in C, its rows in DDS's `include/scr_command_table.h` |
| `script/scrCommonCommand` | shared, all three games | DDS's extra commands and its additions to `BLUR_ALLOFF`, `FILTER_ALLOFF` under the DDS macros |
| `script/scrParameterCommand` | SMT3 only | game code, no twins in DDS |
| `effect/effPCPNeedle` | shared, all three games | DDS names aliased in the unit; SMT3's two `printf`s under `SDF_SMT3` |
| `effect/polyManager` | shared, all three games | DDS names aliased in the unit; the OT slots and `D_002F64B8`'s offset per game; DDS's `dds3SpPoly_RollRingDraw` body (keeps a one-frame record running) under the DDS macros |
| `effect/effPart` | shared, all three games | SMT3's `event.h`, `nb.h`, `effBattle.h` under `SDF_SMT3`, `effPCPData.h` for DDS; the model id counter is u16 in DDS and DDS2 skips ids in use; `D_002F69B8` is DDS's `kwlnSceneSurfaceSets[10]` |
| `effect/effPCPFlash`, `effect/effPCPScatter` | shared, all three games | DDS names aliased in the units; Flash's OT slot per game (`sdfGrOtTable[45]`, DDS `[56]`) |
| `effect/parManager` | shared, all three games | DDS names aliased; SMT3's two NON_MATCHING drafts (`dds3Particle_SparkBuffInit`, `dds3Particle_SparkOffsetBuffInit`) are DDS's matching C, compiled for DDS by `#if defined(NON_MATCHING) \|\| defined(SDF_DDS1) \|\| defined(SDF_DDS2)`; `effect.h` moved here |
| `effect/effMath` | shared, all three games | SMT3's text; DDS's `sdfViewMatrix`, `sdfViewTargetVector` and `effMathResetBezierSlot` (SMT3's `func_001AC068`) aliased |
| `effect/effPacket` | shared, all three games | DDS's four textured quad and triangle writers under the DDS macros; the draw bank's head and the crossfade's second OT slot per game; DDS keeps the end message as INCLUDE_RODATA |

dds-decomp commits, in order: e76182af8 (probe and resolve take `dir/unit`),
878bc0130 (model re-split), ed9e2911e (model names), 79f089aa1 (mdlManager
shared), be74ecd02 (file re-split), 44ee9e9f8 (`fileCallbackStatus`),
5834e25dd (fileManager shared). `config/sdf_shared.txt` lists 48 units.

## mdlEffect, deferred

SMT3's mdlEffect is 42 functions. In DDS1 the first 36 run 0x002192C0 to
0x0021A608, then 54 viewer functions to 0x0021E450, then SMT3's last six
(`mdlSetDebugMatrix` to `soften_bias_Process`) to 0x0021F4B8; DDS2 has the
same shape. The viewer code sits inside the unit, so it cannot be a DDS unit
of its own: an object is one contiguous run. Sharing mdlEffect means the
viewer functions go into the shared file under the DDS macros (or into a DDS
file the shared one includes there), which brings the 15 struct
redefinitions and 27 conflicting prototypes against the engine headers that
the viewer code's DDS headers carry, plus `newdata.h`, which DDS lacks. The
DDS re-split at SMT3's outer boundary stays.

## Traps

- SMT3's `tools/test_find_similar.py` fails after a unit's source is removed
  from the clone: `find_similar.py` caches on the newest source mtime, which a
  deletion does not change. `rm -rf build/find_similar` and rerun.

- `tools/resplit.py` can leave stale yaml rows (the old unit's start rows) and
  print CONFLICT for a duplicate declaration; fix the yaml by hand. It also
  adds `INCLUDE_RODATA` for strings the C already emits; delete those lines.
- Renaming `fileRequestFreeIfComplete` to `fileHandleFree`, or DDS2's
  `fileCreatePacLoadWork` to `pacLoadEx`, flips `code_002DE248`
  (sdfDrawTexturedQuad) to 2 differ. They are `#define` aliases in the shared
  units instead.
- DDS's mdlManager had no rodata row: its `"pfs0:/model/work"` string sat at
  the end of the previous unit. Added rows at DDS1 0x2ACA68 and DDS2 0x321FD8.
- fileManager is in the dev build (`config/<v>/devbuild.json`). The re-split
  needed its replacement and move entries merged (one object now), the
  retained `.bss` dropped, and the counts reset; `docs/development-build.md`
  in dds-decomp has the new totals. A replacement may not use
  `INCLUDE_RODATA`, which is why DDS keeps fileManager's data in C.
- A resplit-deleted unit can come back as a stub file from configure; `git rm`
  it with the re-split.
- smt3-decomp's `tools/check_drift.py` scans the shared files, DDS-only code
  included: a DDS-side type there may not reuse an SMT3 type name or leave an
  `unk` member (the script units' global work view is `scrGlobalWork_t`,
  0327a5c). Run all of SMT3's `tools/test_*.py`, not only `test_sources.py`.

## script/

dds-decomp commits: 930aea9db (dev build scan fix), 9a4c55be9 (re-split at
SMT3's boundaries, DDS1 0x0010BB28, 0x0010C120, 0x0010D6B8, end 0x0010EEF0;
DDS2 0x0010BD50, 0x0010C348, 0x0010D8E0, end 0x0010F118), 514a1257e (names),
738416bee (shared). sdf-decomp: c1647f2 (history of the two units and
`scr.h`), 89bec34, d895e308. `config/sdf_shared.txt` lists 50 units.

- The command table (`scrCommandTable`, 0x1118 bytes) is in the rodata of
  DDS's scrScriptProcess object, not scrTraceCode's as SMT3's split assumes:
  cc1 gives scrTraceCode's jump tables `.align 4`, and the table sits at
  DDS1 0x0039E288, 8-aligned. In SMT3 either placement fits.
- The dev build replaced the old `game/code_0010C028` and scrTraceCode; it now
  replaces scrScriptProcess and scrTraceCode. A replacement may not use
  `INCLUDE_RODATA`, and the table cannot be retained (its entries point at
  commands in other replaced objects), so DDS defines it in C:
  `scrCommandTable[] = { SCR_COMMAND_TABLE_ENTRIES }` in the shared unit, the
  rows per game in DDS's `scr_command_table.h`. The definition has to be in the
  unit's text: include_rodata reads definitions there, and a clean configure
  fails otherwise.
- `tools/dev_elf.py`'s stale HI16/LO16 scan paired a `lui` with a use after
  its register was overwritten (DDS2 `kwlnInitializeFrameDrawPackets`); the
  window now ends at any write to the register (930aea9db, with a test).
- Names carried: `dds3GlobalWorkClearFlags`, `scrPrintProcedureEnd` (DDS's
  `scrProcDestroyTask`, same function without the printf),
  `scrAllReleaseScript` (DDS's `scrDestroyAllNamedProcesses`, which skips the
  running script's own process), `scrGetData`, `scrCommandTable`. Aliased in
  the shared unit for DDS: `datDebugPrintf`, `scrPrintProcedureStart` (DDS
  `evtPrintDeveloperConsoleMessage`, `bfContextCreate`, in DDS's
  `game/code_00107FD8`), `scr_109D80`, `scr_109DD0`, `scr_109E38`,
  `func_00109FD0` (DDS's `scrStartProcessProcedure`, not DDS-only).
- `scr.h` moved to sdf-decomp (SMT3's game commands include it too); it takes
  `kwln.h` under the DDS games for `dds3ProcessID_t`. DDS has no `newdata.h`:
  the two units declare a view of the global work with the script variables
  at 0x40, as in both games. dds-decomp got SMT3's `sdk_libgcc.h`.

scrCommonCommand: `twins.py contig` reports 21 of 67 paired because the
identical `_ON`/`_OFF`/`_SWAP` wrappers are not unique twins; by position 60
of SMT3's 67 have a same-named DDS twin in order. dds-decomp 541786aa5 carries
SMT3's names for the other 7 (`PUT`, `PUTS`, `FADE_IN`, `FADE_OUT`, `SCR_RUN`,
`BLUR_ALLOFF`, `FILTER_ALLOFF`), c701a78cb compiles it from sdf-decomp
(6e00347). In the shared unit: SMT3's `printf` calls and its `D_0052F5E8`
string are under `SDF_SMT3`; `D_005298C9` is now `dds3PadBitTable[1]` (the
same address in all three games); DDS's two background fades, the five game
commands at the end and `scrCommand_SCR_EXISTS` are under the DDS macros, with externs
written in the unit rather than DDS's headers. `config/sdf_shared.txt` lists
51 units.

## effect/

Measured with `twins.py contig --prefix effect/` and the twin ranges (DDS1
addresses; DDS2 has the same shape). "Clean" means the DDS run holds exactly
SMT3's functions in order.

| unit | SMT3 | DDS1 run | state |
|---|---|---|---|
| effPCPNeedle | 17 | 0x001760F8..0x00177120 | shared |
| polyManager | 87 | 0x0015B228..0x0015F810 | shared |
| effPart | 40 | 0x00162970..0x001632E0 | shared |
| effPCPFlash | 148 | 0x0016A088..0x001708A0 | shared |
| effPCPScatter | 90 | 0x001708A0..0x001760F8 | shared |
| parManager | 126 | 0x00153680..0x0015B228 | shared |
| effMath | 11 | 0x0018DBD8..0x0018E218 | shared |
| effPacket | 7 | 0x0015F810..0x001606C0 | shared |
| effSpline | 11 | 0x00192CC8..0x001937E0 | 4 DDS-only, inside `game/code_00192488` |
| effDebug | 12 | 0x0018DF90..0x0018F3A0 | 9 extra, overlaps effMath's run |
| effEvent | 84 | 0x0018F3A0..0x00192CC8 | 82 twins in 93; SMT3 has 1 INCLUDE_ASM |
| effBlur_Filter | 30 | 0x00186BC8..0x00188068 | 18 twins in 27; SMT3 has 1 INCLUDE_ASM |
| effManager, effPCPBezier, effPCPThunder, billManager, effModelTrackPoly, effMagatuhi, effPCPBoss | | | runs widened by small functions that twin far away; pair by position (`/tmp/s8/py/mclist.py` shape) before cutting |
| effPCPMisc | 387 | 0x00177120..0x00184420 | DDS has 369 after the effPCPNeedle re-split |
| effBattle, effBattleMisc, effBattleSystem, effBattleLight | | | battle code, few or no twins; per game |

Where SMT3 still has INCLUDE_ASM and DDS has C, the shared unit should take
DDS's C for that function (it may match SMT3 too) rather than lose it.

The SMT3 clone was rebased onto smt3-decomp 98b9d355 first: 5353c126
(`sdfGrOtTable`'s interior symbols as `PS2_SYMBOL_AT`) and e7483423 touched
polyManager and effEvent. The history merges now reset to 98b9d355.

dds-decomp 0e33cdee7 re-split parManager, polyManager, effPart, effPCPFlash
and effPCPScatter in one run (`config/<v>/effect_units.txt`); each unit's
function count now equals SMT3's. resplit.py's join needed hand fixes:
conflicting prototypes kept from both halves (parManager, polyManager), a K&R
definition whose body it copied into the next unit without its header
(DDS2 `effPCPFlashPolyRingExpHCreate`), a stale `code_00175B00` row, and it
rewrote effPCPThunder's head and dropped two size checks, so effPCPThunder was
restored from HEAD minus the moved function. `/tmp/s8/hist.sh <unit>...`
merges a unit's history, `/tmp/s8/move2.sh dir/unit` moves one,
`/tmp/s8/py/pair.py dir/unit` pairs function names by position from the built
objects (SMT3 clone, DDS1, DDS2).

polyManager: dds-decomp 6d17bdf0b (`dds3SpPoly_RollRingDraw`, DDS's
`polyUpdateRotatingBandCells`), 7248d2491; sdf-decomp eaba360, ea9e6dd.
`func_00168A50` is DDS's `polyBandLayoutRing`, a `#define`; DDS has a
different function at 0x00168A50, which check_unit reported as a 104-word DIFF
until the alias was in.

effPart: dds-decomp d7cd1eb74 (`dds3EffectPartGetHandle`,
`dds3EffectPartGetType`, `dds3EffectPartModelCreate`; DDS1 and DDS2 had named
the last one differently, `effParamCreateInitWork` and
`effParamCreateViewerWork`; `eff_param.h` changed and its 24 includers stayed
clean), 7a60736bb; sdf-decomp 8873049, ef6044c.

effPCPFlash, effPCPScatter: names already agreed. dds-decomp f4b87312c,
5f453c1a1; sdf-decomp a081a76, 46cc20f, c54a62f, and the Scatter commit
after it. SMT3's OT slot 45 (0x5A0) is DDS's 56 (0x700) in polyManager and
Flash alike.

parManager: dds-decomp 3749cb8af (game_source), ecd892f7c
(`dds3ParticleExpSetLoop`, `dds3Particle_EffectSet`, `dds3Particle_EffectReset`
in `par_kind_api.h`, 6 includers clean), 70b6dc6f1; sdf-decomp f3200fe
(`effect.h`'s history), merge of parManager's, 53e4f84. SMT3's drafts of the
two Spark functions are 22 words off in SMT3 and match DDS exactly: with
DDS's names (`effDefaultRandomState`, `effEmitterDelayRandomState`) gcse
orders the two RNG pseudos the other way (DDS's old PARK note on the
function, which hashed SYMBOL_REF names). They stay INCLUDE_ASM in SMT3.

`game_source` (both repos, with a test) reads `#if defined(NON_MATCHING) ||
defined(SDF_DDS1) || defined(SDF_DDS2)` as plain C for a game it names and
as `#ifdef NON_MATCHING` for the others, so every tool sees a draft in SMT3
and C in DDS, and the text is written once. SMT3's `check_nonmatching.py`
now picks and counts drafts on SMT3's view (in the consume patch).

Recipe, as effPCPNeedle went (dds-decomp 68256150f, 23f77cb01, 26cf8c8b3;
sdf-decomp 6a79c02, 4e084ea, 8ebfaf4):

1. Add the boundaries to `config/<v>/effect_units.txt`, run
   `tools/resplit.py <v> config/<v>/effect_units.txt`, drop the stale yaml
   row and restore the `# Nocturne __FILE__` comments, `configure.py
   --force-split`, build, `check_unit` the cut units, commit.
2. Merge the unit's history: `git clone --no-local` of smt3-decomp, reset to
   the consume patch's base, `fr/bin/git-filter-repo --path
   src/smt3/effect/<unit>.c --path-rename src/smt3/effect/:src/effect/`,
   then `git merge --allow-unrelated-histories`. Headers only SMT3 has
   (`effPCPData.h`) come the same way.
3. `tools/sdf_probe.py effect/<unit>`, `tools/sdf_resolve.py effect/<unit>`.
   Rename DDS functions to SMT3's with `tools/sdf_rename.py` when they are the
   same function; `eff.h` is in 67 units per game, so `check_unit` them all
   after a rename there (all 135 were clean for effPCPNeedle). Data and
   widely used names stay DDS's, aliased by `#define` at the top of the unit.

effMath: dds-decomp 35eef0cef (re-split; the "extra" function was SMT3's
`func_001AC068` past SMT3's measured run, so DDS's run is the whole unit),
aa3077da4; sdf-decomp 81061c0 (history), 5798765. SMT3's
`dds3ScreenParam.matrix` and `.offset` are DDS's `sdfProjectionMatrix` and
`D_00324660` by address, so the struct needed no alias. Matched on the first
build in all three games.

effPacket, effDebug, effEvent, effSpline: dds-decomp 78069a46f re-split all
four in one run (DDS1 0x0015F810, end 0x001606C0; 0x0018E218, 0x0018F3A0,
0x00192CC8, end 0x00193900; DDS2 0x00167400, 0x001682B0; 0x00195E50,
0x00196FD8, 0x0019A900, 0x0019B538). Nothing needed fixing by hand but the
`# Nocturne __FILE__` comments. DDS's counts: effPacket 11 (SMT3 7),
effDebug 16, effEvent 93 (SMT3 84), effSpline 18; all C.

effPacket: dds-decomp 5dae51e8f carries `dds3CrossfadeEnd` (was
`dds3EndCrossfade`), `dds3CrossfadeRunFlag`, `dds3CrossfadeFrame`,
`dds3CrossfadeFrameMax`, `dds3RenderDrawEnv` and `dds3DrawEnv` (DDS1
`D_00325870`, whose interior `kwlnFrameDrawPacketRecords` stays), ed7b35eca;
sdf-decomp 294ce25 (history), a30397b. DDS's draw bank is 0x1F40 to SMT3's
0x1BE0, all of it before the frame mask source, and the crossfade draws into
OT slot 82 where SMT3 uses 70. The probe's PAD line was real: compiling the
end message as a literal drops 8 bytes of DDS's rodata and shifts the next
unit's, so DDS prints it from its INCLUDE_RODATA symbol.

## Next

1. effect/: effDebug, effEvent, effSpline (re-split already), then
   effBlur_Filter.
2. mdlEffect, by bringing its viewer code in under the DDS macros.
