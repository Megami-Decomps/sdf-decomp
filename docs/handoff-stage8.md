# Handoff: stage 8, model/, file/, script/

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
| `script/*` | not started | below |

dds-decomp commits, in order: e76182af8 (probe and resolve take `dir/unit`),
878bc0130 (model re-split), ed9e2911e (model names), 79f089aa1 (mdlManager
shared), be74ecd02 (file re-split), 44ee9e9f8 (`fileCallbackStatus`),
5834e25dd (fileManager shared). `config/sdf_shared.txt` lists 48 units.

## mdlEffect, deferred

SMT3's mdlEffect is 42 functions; DDS's span at the same place holds them plus
54 viewer functions (about 2000 lines) written against DDS's own headers.
Compiling the shared unit for DDS needs `newdata.h` (DDS has none) and pulling
DDS's viewer code in brings 15 struct redefinitions and 27 conflicting
prototypes against the engine headers. Sharing it means first moving the
viewer functions to their own DDS unit (they are not engine code by SMT3's
split) or reconciling those types. The DDS re-split at SMT3's boundary stays.

## Traps

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

## script/, measured, reverted

Stopped here at the user's request. scrScriptProcess and scrTraceCode are the
script VM; scrCommonCommand is mostly game commands (46 of SMT3's 67 functions
unpaired, 5 more between runs); scrParameterCommand is SMT3 game code with no
twins in DDS and stays in smt3-decomp.

SMT3's boundaries in DDS, from `twins.py runs --prefix script/`:

| unit | DDS1 | DDS2 |
|---|---|---|
| scrScriptProcess | 0x0010BB28 | 0x0010BD50 |
| scrTraceCode | 0x0010C120 | 0x0010C348 |
| scrCommonCommand | 0x0010D6B8 (end kept at 0x0010EEF0) | 0x0010D8E0 (end 0x0010F118) |

A re-split with those lists (`config/<v>/script_units.txt`, same format as
`model_units.txt`) ran cleanly after removing resplit's stale
`scrCommonCommand` row and restoring the `# Nocturne __FILE__` comments, but
was reverted before a build because of the dev build: it replaces DDS1
`game/code_0010C028` and `script/scrTraceCode` (DDS2 `code_0010C250` and
`scrTraceCode`), and the re-split folds the first five functions of
`code_0010C028` (scrSetUserData to scrScriptProcess) into scrScriptProcess.
The descriptor then has to replace scrScriptProcess whole (its 0x1118 bytes of
rodata include an `INCLUDE_RODATA`, which a replacement may not use, so make
that C first or retain the section), merge the old code_0010C028 and
scrTraceCode text and rodata moves into one scrTraceCode move (DDS1 text
0x0010C120 size 0x1598, rodata 0x0039F390 size 0x134), and reset the counts
with a loop that sets each count the verifier reports (the session used a
scratch `/tmp/s8b/devloop.py`, not kept).

Pairing (`twins.Game.twin()` over each SMT3 function): SMT3-only `scrPrintProcedureEnd`,
`scrAllReleaseScript`; DDS-only `scrProcDestroyTask`,
`scrDestroyAllNamedProcesses`, `scrStartProcessProcedure` in their slot.
Names to carry: `scrClearProcessGlobals` to `dds3GlobalWorkClearFlags`; SMT3's
`scr_109D80`, `scr_109DD0`, `scr_109E38` are placeholders, so alias them to
DDS's names in the shared unit.

## Next

1. script/: the dev build descriptor above, then the re-split, names and
   scrScriptProcess and scrTraceCode, one at a time.
2. mdlEffect, after its DDS viewer functions have a unit of their own.
3. effect/.
