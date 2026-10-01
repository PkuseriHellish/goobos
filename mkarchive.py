#!/usr/bin/env python3
"""
Packs a directory into the tiny archive format archive.c parses.

Format (little-endian, no padding, no alignment -- just sequential bytes):

    magic   4 bytes   "SFS1"
    count   u32       number of entries
    per entry:
        name_len   u32   includes the trailing NUL
        name       name_len bytes, NUL-terminated
        data_len   u32
        data       data_len bytes, raw

Names are the file's path relative to the root directory, using forward
slashes. There's no real hierarchy on either side of this -- a name like
"docs/readme.txt" is just a string; the kernel never splits it or builds
a directory tree from it.

Usage:
    mkarchive.py <root-dir> <output-file>
"""
import os
import struct
import sys


def main():
    if len(sys.argv) != 3:
        sys.exit(f"usage: {sys.argv[0]} <root-dir> <output-file>")
    root, out_path = sys.argv[1], sys.argv[2]

    entries = []
    for dirpath, _dirnames, filenames in os.walk(root):
        for fname in sorted(filenames):
            full = os.path.join(dirpath, fname)
            rel = os.path.relpath(full, root).replace(os.sep, "/")
            with open(full, "rb") as f:
                entries.append((rel, f.read()))
    entries.sort(key=lambda e: e[0])  # stable output regardless of os.walk order

    with open(out_path, "wb") as out:
        out.write(b"SFS1")
        out.write(struct.pack("<I", len(entries)))
        for name, data in entries:
            name_bytes = name.encode("utf-8") + b"\0"
            out.write(struct.pack("<I", len(name_bytes)))
            out.write(name_bytes)
            out.write(struct.pack("<I", len(data)))
            out.write(data)

    total = sum(len(d) for _, d in entries)
    print(f"{out_path}: {len(entries)} entries, {total} bytes of content")
    for name, data in entries:
        print(f"  {name}  ({len(data)} bytes)")


if __name__ == "__main__":
    main()
