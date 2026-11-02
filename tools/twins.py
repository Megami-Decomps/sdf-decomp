#!/usr/bin/env python3
"""Find SMT3 functions whose code is the same in DDS1 and DDS2.

Hashes mask jump targets and gp- and lui-relative immediates; --loose masks every immediate.
"""
import argparse
import collections
import difflib
import hashlib
import json
import re
import struct
import sys
from pathlib import Path

SIBLINGS = Path(__file__).resolve().parents[2]

GROUPS = ("sdf", "model", "file", "script", "effect", "newbattle")

J, JAL, LUI = 0x02, 0x03, 0x0F
GP = 28
# Opcodes whose low 16 bits --loose masks: lui, addiu, andi, ori, daddiu and
# the loads and stores.
LOOSE_IMM = {0x0F, 0x09, 0x0C, 0x0D, 0x19, 0x1E, 0x1F, 0x20, 0x21, 0x23, 0x24, 0x25,
             0x27, 0x28, 0x29, 0x2B, 0x2C, 0x2D, 0x31, 0x34, 0x35, 0x37, 0x39, 0x3C,
             0x3D, 0x3F}
LOADS_STORES = {0x1E, 0x1F, 0x20, 0x21, 0x23, 0x24, 0x25, 0x27, 0x28, 0x29, 0x2B, 0x2C,
                0x2D, 0x31, 0x34, 0x35, 0x37, 0x39, 0x3C, 0x3D, 0x3F}
ADDRESS_ARITH = {0x09, 0x0D, 0x19}  # addiu, ori, daddiu

SYMBOL = re.compile(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+);.*type:func")
SIZE = re.compile(r"size:0x([0-9A-Fa-f]+)")
SEGMENT = re.compile(r"\s*-\s*\[\s*(0x[0-9A-Fa-f]+)\s*,\s*(c|asm|hasm)\s*,\s*([^\],\s]+)")
INCLUDE_ASM = re.compile(r"INCLUDE_ASM\([^)]*\)")

Function = collections.namedtuple("Function", "name addr words unit hash")


def words_of(data):
    words = list(struct.unpack("<%dI" % (len(data) // 4), data[:len(data) // 4 * 4]))
    while words and words[-1] == 0:
        words.pop()
    return words


def mask_loose(words):
    out = []
    for word in words:
        op = word >> 26
        if op in (J, JAL):
            word &= 0xFC000000
        elif op in LOOSE_IMM:
            word &= 0xFFFF0000
        out.append(word)
    return out


def mask_strict(words):
    """Mask only immediates that hold an address: from lui, or off gp or a lui'd register.

    A register stays lui'd to the end of the function; that over-masks a reused
    register, never under-masks."""
    lui_regs = set()
    out = []
    for word in words:
        op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
        if op in (J, JAL):
            word &= 0xFC000000
        elif op == LUI:
            lui_regs.add(rt)
            word &= 0xFFFF0000
        elif op in LOADS_STORES or op in ADDRESS_ARITH:
            if rs == GP or rs in lui_regs:
                word &= 0xFFFF0000
        out.append(word)
    return out


def segments(elf):
    phoff, = struct.unpack_from("<I", elf, 0x1C)
    phentsize, phnum = struct.unpack_from("<HH", elf, 0x2A)
    out = []
    for i in range(phnum):
        kind, offset, vaddr, _, filesz, _ = struct.unpack_from("<6I", elf, phoff + i * phentsize)
        if kind == 1:
            out.append((vaddr, offset, filesz))
    return out


def read(elf, segs, addr, size):
    for vaddr, offset, filesz in segs:
        if vaddr <= addr and addr + size <= vaddr + filesz:
            return elf[offset + addr - vaddr:offset + addr - vaddr + size]
    return None


def symbols(path):
    rows = set()
    for line in path.read_text().splitlines():
        if m := SYMBOL.match(line):
            size = SIZE.search(line)
            rows.add((int(m[2], 16), m[1], int(size[1], 16) if size else None))
    rows = sorted(rows)
    out = []
    for i, (addr, name, size) in enumerate(rows):
        if size is None:
            size = rows[i + 1][0] - addr if i + 1 < len(rows) else 0
        out.append((addr, name, size))
    return out


def yaml_units(path):
    """(file offset, unit) of each code segment, in yaml order."""
    return [(int(m[1], 16), m[3]) for line in path.read_text().splitlines()
            if (m := SEGMENT.match(line))]


def asm_functions(repo, version):
    """Functions still behind INCLUDE_ASM, in src/<version> and any submodule's src/."""
    roots = [repo / "src" / version] + sorted(repo.glob("external/*/src"))
    names = set()
    for root in roots:
        for path in root.rglob("*.c"):
            for call in INCLUDE_ASM.findall(path.read_text(errors="replace")):
                names.add(call.split(",")[-1].strip(' )"'))
    return names


class Game:
    def __init__(self, repo, version, mask):
        spec = json.loads((repo / "config" / "versions.json").read_text())[version]
        serial = spec["serial"]
        elf = (repo / "orig" / version / serial).read_bytes()
        if hashlib.sha1(elf).hexdigest() != spec["elf_sha1"]:
            sys.exit(f"{repo}/orig/{version}/{serial} is not the retail executable")
        segs = segments(elf)
        text = next(s for s in segs if s[0] <= int(spec["entry"], 16) < s[0] + s[2])
        units = yaml_units(repo / "config" / version / f"{serial}.yaml")
        self.version = version
        self.asm = asm_functions(repo, version)
        self.functions = {}
        for addr, name, size in symbols(repo / "config" / version / "symbol_addrs.txt"):
            if size <= 0 or size > 0x40000:
                continue
            data = read(elf, segs, addr, size)
            if data is None:
                continue
            offset = text[1] + addr - text[0]
            unit = None
            for start, candidate in units:
                if start > offset:
                    break
                unit = candidate
            words = words_of(data)
            digest = hashlib.sha1(struct.pack("<%dI" % len(words), *mask(words))).hexdigest()
            self.functions[name] = Function(name, addr, len(words), unit, digest)
        self.by_hash = collections.defaultdict(list)
        for f in sorted(self.functions.values(), key=lambda f: f.addr):
            self.by_hash[f.hash].append(f)

    def twin(self, f):
        """The function here hashing like f, preferring one of the same name."""
        matches = self.by_hash.get(f.hash, [])
        return next((m for m in matches if m.name == f.name), matches[0] if matches else None)


def group_of(unit):
    return unit.split("/")[0] if unit else "?"


def load(args):
    mask = mask_loose if args.loose else mask_strict
    return (Game(args.smt3, "smt3", mask), Game(args.dds, "dds1", mask), Game(args.dds, "dds2", mask))


def table(args):
    smt3, dds1, dds2 = load(args)
    count = collections.defaultdict(collections.Counter)
    for f in smt3.functions.values():
        if f.words < 4:
            continue
        c = count[group_of(f.unit)]
        c["functions"] += 1
        c["bytes"] += f.words * 4
        twin1, twin2 = dds1.twin(f), dds2.twin(f)
        if twin1 and twin2:
            c["all3"] += 1
            c["all3 bytes"] += f.words * 4
        if f.name not in smt3.asm:
            c["smt3 c"] += 1
        if twin1:
            c["dds1 twins"] += 1
            c["dds1 c"] += twin1.name not in dds1.asm
            c["renames"] += twin1.name != f.name
    groups = args.group or GROUPS
    print("| SMT3 group | functions | identical in all 3 | bytes | SMT3 in C "
          "| DDS1 twins | DDS1 twins in C | DDS1 names differ |")
    print("|---|---|---|---|---|---|---|---|")
    for g in groups:
        c = count.get(g)
        if not c:
            sys.exit(f"no SMT3 functions in group {g}")
        print(f"| `{g}/` | {c['functions']} | {c['all3']} | {round(100 * c['all3 bytes'] / c['bytes'])}% "
              f"| {c['smt3 c']} | {c['dds1 twins']} | {c['dds1 c']} | {c['renames']} |")


def contig(args):
    smt3, *dds = load(args)
    units = collections.defaultdict(list)
    for f in sorted(smt3.functions.values(), key=lambda f: f.addr):
        if f.unit and f.unit.startswith(args.prefix):
            units[f.unit].append(f)
    if not units:
        sys.exit(f"no SMT3 units start with {args.prefix}")
    for game in dds:
        order = sorted(game.functions.values(), key=lambda f: f.addr)
        position = {f.name: i for i, f in enumerate(order)}
        print(f"== {game.version}")
        print("| unit | SMT3 | paired | unpaired | no twin | DDS span | DDS-only | order breaks |")
        print("|---|---|---|---|---|---|---|---|")
        flagged = []
        for unit, functions in units.items():
            paired, unpaired = [], 0
            no_twin = sum(1 for f in functions if f.words >= 4 and f.hash not in game.by_hash)
            for f in functions:
                matches = game.by_hash.get(f.hash, []) if f.words >= 4 else []
                if len(matches) == 1:
                    paired.append(position[matches[0].name])
                else:
                    unpaired += 1
            if not paired:
                print(f"| {unit} | {len(functions)} | 0 | {unpaired} | {no_twin} | | | |")
                flagged.append(f"{unit}: no unique twins")
                continue
            lo, hi = min(paired), max(paired)
            span = hi - lo + 1
            breaks = sum(1 for a, b in zip(paired, paired[1:]) if b < a)
            # Functions in the DDS run that no SMT3 function of the unit can account for.
            dds_only = span - len(paired) - unpaired
            print(f"| {unit} | {len(functions)} | {len(paired)} | {unpaired} | {no_twin} | {span} "
                  f"| {max(dds_only, 0)} | {breaks} |")
            if breaks or dds_only > 0:
                flagged.append(f"{unit}: {max(dds_only, 0)} DDS-only, {breaks} order breaks")
                if args.verbose and not breaks:
                    twins = {m.name for f in functions for m in game.by_hash.get(f.hash, [])}
                    for f in order[lo:hi + 1]:
                        if f.name not in twins:
                            print(f"    {f.name} 0x{f.addr:08X} {f.words * 4} bytes")
        print()
        print(f"{len(units) - len(flagged)} of {len(units)} units are unbroken runs with nothing extra")
        for line in flagged:
            print(f"  {line}")
        print()


def align(smt3_functions, game):
    """Map each DDS function name to the SMT3 function it aligns with, in address order.

    Hashes are aligned as sequences, so a small wrapper that hashes like many others
    still pairs by position."""
    order = sorted(game.functions.values(), key=lambda f: f.addr)
    matcher = difflib.SequenceMatcher(None, [f.hash for f in smt3_functions], [f.hash for f in order],
                                      autojunk=False)
    pairs = {}
    for a, b, n in matcher.get_matching_blocks():
        for i in range(n):
            pairs[order[b + i].name] = smt3_functions[a + i]
    return order, pairs


def runs(args):
    smt3, *dds = load(args)
    functions = sorted((f for f in smt3.functions.values() if f.unit and f.unit.startswith(args.prefix)),
                       key=lambda f: f.addr)
    if not functions:
        sys.exit(f"no SMT3 units start with {args.prefix}")
    for game in dds:
        order, pairs = align(functions, game)
        print(f"== {game.version}")
        # Each DDS function between the first and last aligned one, with its unit or None.
        placed = [i for i, f in enumerate(order) if f.name in pairs]
        rows = [(f, pairs[f.name].unit if f.name in pairs else None) for f in order[placed[0]:placed[-1] + 1]]
        current, start, extra, gap = None, None, [], []

        def close(end):
            if current:
                print(f"{current} 0x{start:08X} 0x{end:08X}")
                for f in extra:
                    print(f"    inside {f.name} 0x{f.addr:08X}")

        for i, (f, unit) in enumerate(rows):
            if unit is None:
                (gap if current is None else extra).append(f)
                continue
            if unit != current and current is None:
                current, start, extra, gap = unit, f.addr, [], []
            elif unit != current:
                # Unaligned functions after the previous unit's last aligned one are a gap.
                tail = []
                while extra and extra[-1].addr > last.addr:
                    tail.insert(0, extra.pop())
                close(tail[0].addr if tail else last.addr + last.words * 4)
                for g in tail + gap:
                    print(f"  between {g.name} 0x{g.addr:08X} {g.words * 4} bytes")
                current, start, extra, gap = unit, f.addr, [], []
            last = f
        close(last.addr + last.words * 4)
        missing = [f for f in functions if f not in pairs.values()]
        for f in missing:
            print(f"  unaligned SMT3 {f.unit} {f.name} 0x{f.addr:08X}")
        print()


def renames(args):
    smt3, *dds = load(args)
    functions = sorted((f for f in smt3.functions.values() if f.unit and f.unit.startswith(args.prefix)),
                       key=lambda f: f.addr)
    for game in dds:
        names = set(game.functions)
        # Among several same-hash twins, the one aligned with f in unit order.
        aligned = {s.name: game.functions[d] for d, s in align(functions, game)[1].items()}
        for f in functions:
            if f.words < 4:
                continue
            matches = game.by_hash.get(f.hash, [])
            if not matches or any(m.name == f.name for m in matches):
                continue
            if len(matches) > 1:
                if f.name not in aligned or aligned[f.name] not in matches:
                    print(f"{game.version}\t{f.name}\tskip: {len(matches)} twins")
                    continue
                matches = [aligned[f.name]]
            note = "\tcollision" if f.name in names else ""
            print(f"{game.version}\t{matches[0].name}\t{f.name}\t0x{matches[0].addr:08X}{note}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--smt3", type=Path, default=SIBLINGS / "smt3-decomp", help="smt3-decomp checkout")
    parser.add_argument("--dds", type=Path, default=SIBLINGS / "dds-decomp", help="dds-decomp checkout")
    parser.add_argument("--loose", action="store_true", help="mask every immediate, not only addresses")
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("table", help="per-group identity table")
    p.add_argument("--group", action="append", help=f"SMT3 top directory (default {', '.join(GROUPS)})")
    p.set_defaults(run=table)
    p = sub.add_parser("contig", help="whether each SMT3 unit is one unbroken run in DDS1 and DDS2")
    p.add_argument("--prefix", default="sdf/", help="SMT3 unit name prefix")
    p.add_argument("-v", "--verbose", action="store_true",
                   help="list the functions in a flagged unit's DDS run that have no SMT3 twin")
    p.set_defaults(run=contig)
    p = sub.add_parser("runs", help="each SMT3 unit's address range in DDS1 and DDS2, by sequence alignment")
    p.add_argument("--prefix", default="sdf/", help="SMT3 unit name prefix")
    p.set_defaults(run=runs)
    p = sub.add_parser("renames", help="DDS name and SMT3 name of each identical twin named differently")
    p.add_argument("--prefix", default="sdf/", help="SMT3 unit name prefix")
    p.set_defaults(run=renames)
    args = parser.parse_args()
    args.run(args)


if __name__ == "__main__":
    main()
