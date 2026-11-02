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
| `script/scrScriptProcess`, `script/scrTraceCode` | shared, all three games | the script VM; DDS defines the command table in C, its rows in DDS's `include/scr_command_table.h` |
| `script/scrCommonCommand` | shared, all three games | DDS's extra commands and its additions to `BLUR_ALLOFF`, `FILTER_ALLOFF` under the DDS macros |
| `script/scrParameterCommand` | SMT3 only | game code, no twins in DDS |

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

## Next

1. mdlEffect, after its DDS viewer functions have a unit of their own.
2. effect/.
