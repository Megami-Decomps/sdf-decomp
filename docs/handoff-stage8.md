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

## script/, measured

From `/tmp/s8b/runs-*.txt` (`twins.py runs`), not yet re-split:
scrParameterCommand has no unique twins; scrCommonCommand has 46 unpaired
functions plus 5 between runs. DDS already has `script/scrScriptProcess`,
`scrTraceCode` and `scrCommonCommand` units under those names. The headers they
use (`kernel.h`, `scr.h`, `newdata.h`, `basic.h`) differ in DDS.

## Next

1. script/: re-split one unit at a time (scrScriptProcess and scrTraceCode
   first), carry names, probe with `tools/sdf_probe.py script/<unit>`.
2. mdlEffect, after its DDS viewer functions have a unit of their own.
3. effect/.
