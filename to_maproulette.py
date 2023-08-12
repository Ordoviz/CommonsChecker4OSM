#!/usr/bin/env python3
import sys

infile = sys.argv[1]
fmtstr = b'\x1e{"type":"FeatureCollection","features":[%s]}\n'

for line in open(infile, "rb"):
    sys.stdout.buffer.write(fmtstr % line[1:].rstrip(b'\n'))
