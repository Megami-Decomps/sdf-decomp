# Plan

sdf-decomp holds the matching C for SDF, the engine under Shin Megami Tensei III:
Nocturne (SLUS_209.11), Digital Devil Saga (SLUS_209.74) and Digital Devil Saga 2
(SLUS_211.52). smt3-decomp and dds-decomp compile it as a submodule. Each game
repo keeps its own splat yaml, symbol table, checksum and build.

## Rules

- **SMT3 is the authority.** Its names come from real symbols (SMT3HD). The
  shared text, names, headers and unit split are smt3-decomp's; DDS conforms.
  Layouts and sizes still come from each game's retail code.
- **Differences are `#ifdef`s** on one game macro: `SDF_SMT3`, `SDF_DDS1` or
  `SDF_DDS2`, exactly one defined per build. A per-game struct layout goes in the
  header, a per-game function or datum around its definition. A DDS-only
  function keeps its DDS name inside its `#ifdef`.
- **Every game stays byte-identical.** A change lands only when all three ELFs
  still match their retail SHA-1. A unit that has not been moved yet stays in
  its game repo.
- **History comes along.** The seed is smt3-decomp's history for the moved
  paths, so blame and authorship survive.

## Measurements (2026-11-02)

Functions hashed from the three retail ELFs, with jump targets and gp- and
`lui`-relative immediates masked and struct offsets kept:

| SMT3 group | functions | identical in all 3 | bytes |
|---|---|---|---|
| `sdf/` | 822 | 777 | 91% |
| `model/` | 112 | 112 | 100% |
| `file/` | 31 | 29 | 94% |
| `script/` | 163 | 144 | 86% |
| `effect/` | 1581 | 1135 | 68% |
| `newbattle/` | 1560 | 59 | 1% |

SMT3's 45 `sdf/` units map onto DDS1 and DDS2 as unbroken runs in the same
order. Five units have one or two DDS-only functions inside the run
(`sdfSystem`, `sdfMat`, `sdfDevCons`, `sdfMovie`, `sdfPacDecode`); a few
functions are SMT3-only (`sdfHdd`, `sdfMisc`). `sdfDevFile` only looks
scattered, because tiny wrappers hash equal to unrelated functions.

Matching is not the payoff: SMT3 has 792 of 822 sdf functions in C, and DDS1
has 773 of their twins in C. The payoff is one set of names and types, and a
path for smt3-port and sdfport to reach DDS.

## Stages

1. **Seed.** Filter smt3-decomp's history to `src/smt3/sdf/` and the headers
   those units need, laid out as `src/sdf/` and `include/`. Headers the rest of
   the game also includes (`common.h`, the SDK headers) stay in the game repo
   and are found on its include path; record which.
2. **SMT3 consumes it.** smt3-decomp builds `src/sdf/` from the submodule with
   `-DSDF_SMT3` and stays byte-identical. Verified first in a local clone; the
   change to smt3-decomp itself lands there.
3. **Measurement tool.** `tools/twins.py`: the three-ELF identity table and the
   per-unit contiguity check above, rerunnable.
4. **DDS re-split.** dds-decomp's yaml for both games takes SMT3's 45 sdf unit
   boundaries. A dds-decomp change, byte-identical.
5. **DDS rename.** The 171 identical twins whose DDS names differ from SMT3's
   take SMT3's names (dds-decomp `tools/rename_refs.py`).
6. **DDS consumes it, one unit at a time.** Replace the unit's DDS source with
   the shared one under `-DSDF_DDS1`/`-DSDF_DDS2`, adding `#ifdef`s until both
   DDS games checksum. Units with no DDS-only code first.
7. **Pins follow.** smt3-port and sdfport pick up the new layout.
8. **Second round:** `model/`, `file/`, `script/`, then `effect/`.

Nothing is pushed until a GitHub home is chosen (suggested
`Megami-Decomps/sdf-decomp`).
