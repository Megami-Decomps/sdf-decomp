# sdf-decomp project notes

Shared matching C for the SDF engine, compiled by smt3-decomp and dds-decomp.
`docs/plan.md` is the plan and its rules; `docs/handoff.md` is where the last
session stopped.

- SMT3 is the authority for text, names, headers and unit split. DDS conforms.
- Per-game differences are `#ifdef SDF_SMT3` / `SDF_DDS1` / `SDF_DDS2`.
- A change lands only when all three games still build byte-identical.
- Style is smt3-decomp's: 4-space indent, `/* */` comments, `s32`/`u32`/`u8`,
  a function's brace on its own line.
- smt3-decomp and dds-decomp are not edited from this repo's sessions. Verify
  against local clones; changes to them land in their own repos.
