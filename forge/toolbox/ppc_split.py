#!/usr/bin/env python3
"""ppc_split.py - the forge's split stage for a Wii PowerPC executable.

Takes a DOL (Wii main.dol), disassembles it with Capstone (PowerPC, 32-bit,
big-endian), and emits the normalized Molerat foreign IR
(`molerat.foreign-ir/0.1`) that `effect molerat gather` already parses.

This is a REAL splitter - a linear sweep plus recursive traversal from the entry
point, following direct branch and call targets - and it is honest about what it
is not: it does not decompile, it does not follow indirect branches, and it names
each function by address until a better name exists. Depth: bytes.

Usage:
  python ppc_split.py <main.dol> --out <ir.json> [--limit 64] [--max-ops 512]
"""
import argparse
import json
import struct
import sys

try:
    from capstone import Cs, CS_ARCH_PPC, CS_MODE_32, CS_MODE_BIG_ENDIAN
    from capstone.ppc import PPC_OP_IMM
except ImportError as e:  # a tool is missing or moved: say exactly what happened
    print(json.dumps({"ok": False,
                      "error": "cannot import capstone (%s): run `python -m pip install capstone`" % e}))
    sys.exit(2)


def u32(data, off):
    return struct.unpack(">I", data[off:off + 4])[0]


def parse_dol(path):
    with open(path, "rb") as f:
        data = f.read()
    text_off = [u32(data, 0x00 + 4 * i) for i in range(7)]
    text_addr = [u32(data, 0x48 + 4 * i) for i in range(7)]
    text_size = [u32(data, 0x90 + 4 * i) for i in range(7)]
    entry = u32(data, 0xE0)
    sections = []
    for off, addr, size in zip(text_off, text_addr, text_size):
        if size > 0:
            sections.append((addr, off, size))
    return data, entry, sections


def addr_to_off(addr, sections):
    for saddr, off, size in sections:
        if saddr <= addr < saddr + size:
            return off + (addr - saddr), saddr + size  # (file offset, section end addr)
    return None, None


def is_text(addr, sections):
    for saddr, _, size in sections:
        if saddr <= addr < saddr + size:
            return True
    return False


def classify(mnemonic, op_str, target):
    """One PowerPC instruction -> one Molerat op dict."""
    m = mnemonic
    if m == "bl":
        return {"kind": "call", "value": target or op_str}
    if m in ("blr", "bctr", "bctrl"):
        return {"kind": "return", "value": m}
    if m == "b":
        return {"kind": "branch", "dst": target or op_str}
    if m.startswith("bc"):
        return {"kind": "branch", "dst": target or op_str}
    if m.startswith("st"):
        return {"kind": "store", "dst": op_str}
    if m.startswith("l") and ("z" in m or "w" in m or "h" in m or "b" in m):
        return {"kind": "load", "src": op_str}
    if m.startswith("add") or m == "subf":
        return {"kind": "add" if m.startswith("add") else "sub", "value": op_str}
    if m.startswith("cmp"):
        return {"kind": "cmp", "value": op_str}
    if m == "mr":
        return {"kind": "move", "value": op_str}
    return {"kind": "insn", "value": m + (" " + op_str if op_str else "")}


def disasm_function(md, data, start_addr, sections, max_ops):
    off, sec_end = addr_to_off(start_addr, sections)
    if off is None:
        return None, [], []
    chunk = data[off:min(off + 4096, len(data))]
    ops = []
    calls = []
    branch_targets = []
    for insn in md.disasm(chunk, start_addr):
        if len(ops) >= max_ops:
            break
        target = None
        for op in insn.operands:
            if op.type == PPC_OP_IMM:
                target = "0x%08X" % op.imm
        ops.append(classify(insn.mnemonic, insn.op_str, target))

        if insn.mnemonic == "bl" and target and is_text(int(target, 16), sections):
            calls.append(target)
            branch_targets.append(int(target, 16))
        elif insn.mnemonic == "b" and target and is_text(int(target, 16), sections):
            branch_targets.append(int(target, 16))
            break  # unconditional: the function ends here
        elif insn.mnemonic in ("blr", "bctr", "bctrl"):
            break  # return
        elif insn.mnemonic.startswith("bc") and target and is_text(int(target, 16), sections):
            branch_targets.append(int(target, 16))
    return ops, calls, branch_targets


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dol")
    ap.add_argument("--out", required=True)
    ap.add_argument("--limit", type=int, default=64)
    ap.add_argument("--max-ops", type=int, default=512)
    args = ap.parse_args()

    try:
        data, entry, sections = parse_dol(args.dol)
    except Exception as e:
        print(json.dumps({"ok": False, "error": "cannot parse DOL: %s" % e}))
        return 2

    md = Cs(CS_ARCH_PPC, CS_MODE_32 | CS_MODE_BIG_ENDIAN)
    md.detail = True   # operands are needed for branch targets

    functions = []
    seen = set()
    worklist = [entry]

    while worklist and len(functions) < args.limit:
        addr = worklist.pop(0)
        if addr in seen:
            continue
        seen.add(addr)
        ops, calls, targets = disasm_function(md, data, addr, sections, args.max_ops)
        if not ops:
            continue
        for t in targets:
            if t not in seen and len(seen) < args.limit * 4:
                worklist.append(t)
        functions.append({
            "name": "sub_%08X" % addr,
            "addr": "0x%08X" % addr,
            "blocks": [{"id": "e", "ops": ops}],
            "calls": calls,
            "reads": [],
            "writes": [],
            "evidence": ["capstone PPC disassembly (linear sweep + traversal)"],
        })

    ir = {
        "schema": "molerat.foreign-ir/0.1",
        "target": {
            "id": "rmgp01",
            "title": "Super Mario Galaxy",
            "platform": "wii",
            "origin": "capstone PPC disassembly of main.dol (split stage)",
            "depth": "bytes",
        },
        "functions": functions,
        "residual": [
            "linear sweep + recursive traversal; indirect branches and paired-single instructions are not followed",
            "names are addresses until a decompilation names them",
            "bounded to %d functions" % args.limit,
        ],
    }

    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(ir, f, indent=1)

    print(json.dumps({
        "ok": True,
        "out": args.out,
        "functions": len(functions),
        "entry": "0x%08X" % entry,
        "tool": "capstone",
    }))
    return 0


if __name__ == "__main__":
    sys.exit(main())
