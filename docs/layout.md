# Layout

## Seed

The history is smt3-decomp's, filtered with git filter-repo to these paths:

| here | smt3-decomp |
|---|---|
| `src/sdf/*.c` | `src/smt3/sdf/*.c` (45 units) |
| `src/sdf/vu/*.vsm` | `src/smt3/sdf/vu/*.vsm` |
| `include/*.h` | `include/*.h` (the 14 engine headers below) |
| `src/sdf/code_002A99B0.c` | `src/smt3/code_002A99B0.c`, renamed to `sdfIop.c` in bb0144e |
| `src/sdf/code_002AA350.c` | `src/smt3/code_002AA350.c`, split, the larger part becoming `sdfImageSet.c` in 09207bc |

The path list is every name `git log --follow` reports for the current files.
Units that were split out of a larger `code_*.c` container (rather than
renamed) start their history at the split commit; the container's earlier
history holds other units too and was left out. Hashes are this repo's; smt3-decomp hashes in commit
messages refer to smt3-decomp's history, not this one.

## Headers

The rule: a header moves if it declares engine types or functions that the sdf
units need, so DDS can take the same declarations later. It stays in the game
repo if it is toolchain, SDK or game layer, and the game repo puts its own
`include/` on the path after this one's.

### Moved (engine)

| header | what |
|---|---|
| `libsdf.h` | core engine types, includes `sdf_types.h`, `sdf_resource.h` |
| `sdf_types.h` | vectors, matrices, basic engine types |
| `sdf_resource.h` | resource and memory handles |
| `sdf_lighting.h` | parallel light packet |
| `sdf_movie_types.h` | sdfIpu and sdfMovie control blocks |
| `sdfGsMemIO.h` | sdfGsMemIO transfer record |
| `sdfDevCons.h` | `sdfDevPrintf` |
| `sdfCorePacketCb_t.h`, `sdfCorePacketPointBuff_tag.h` | sdfVu1 packet callbacks |
| `draw.h` | sdfGr frame buffers, draw queue, packets |
| `draw3d.h` | sdf3D and sdfCoreDraw camera, light, fog packets |
| `motion.h` | sdfMotion |
| `model.h` | sdfModel; lines from `mdlObj_t` on are the game's `model/` group |
| `billboard.h` | sdfBillBoard VIF packets; also effect's `dds3Billboard*` and `bill*` types |
| `fileManager.h` | `fileRequest_t` and `fileLoad`, the engine's queued file loader (stage 8) |
| `scr.h` | the script VM's file tables and process work block (stage 8); SMT3's game commands include it too |

`model.h` and `billboard.h` are mixed. They moved whole because the sdf units
need them and `model/` and `effect/` are planned to move here later (plan
stage 8). If DDS needs only the engine half, split them then, in smt3-decomp's
favour (the split must keep SMT3 byte-identical, which it does, since moving a
declaration does not change code).

### Stay in the game repo

| header | why |
|---|---|
| `common.h`, `include_asm.h` | the game build's base types and `INCLUDE_ASM`; each game repo has its own |
| `ps2asm.h` | EE and VU0 instruction macros, used by 204 non-sdf units; sdfport owns its host copy |
| `fpu.h` | `fsqrtf` and friends, compiler convention shared by all game code |
| `eeregs.h`, `libdma.h`, `sif_rpc.h`, `sdk_*.h` | Sony SDK declarations; the SDK units live in the game repo |
| `basic.h`, `kernel.h`, `pipe.h` | dds3 scene graph, process kernel and SMT3 pipe objects, game layer |

The game-layer headers are used only by `sdfSystem.c` (pipe objects, config
backup) and `sdfDefaultMain.c` (`dds3KernelMain`), which hold SMT3 game code.
Those parts become `#ifdef SDF_SMT3` when DDS consumes the units.

Stage 8's units include `kernel.h` (`mdlEffect.c`, `fileManager.c`, `scr.h`)
and `newdata.h` (`mdlEffect.c`, the two `script/` units) from the game repo.
They are game layer and stay. DDS's `kernel.h` does not declare
`dds3ProcessID_t`; `fileManager.c` and `scr.h` take it from DDS's `kwln.h`
under the DDS macros. DDS has no `newdata.h`: the `script/` units declare
DDS's view of the global work's script variables, at SMT3's offsets.

### Left behind, unused

`sdf_model_packet_callback.h`, `sdf_packet_alloc.h` and `sdf_packet_list.h`
declare sdf functions but nothing in smt3-decomp includes them. They stay in
smt3-decomp; deleting or moving them is smt3-decomp's call.

## Include order

The game repo compiles `src/sdf/*.c` with `-I<sdf-decomp>/include` ahead of its
own `include/`. No header name exists in both, so the order only matters if one
is added later.
