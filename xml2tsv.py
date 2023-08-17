#!/usr/bin/env python3
import xml.etree.ElementTree as ET
import sys

FMTSTR = "%s/%s\t%s\n"

tree = ET.parse(sys.stdin.buffer)
for elem in tree.findall(".//*tag[@k='wikimedia_commons']/.."):
    wikimedia_commons = elem.find("tag[@k='wikimedia_commons']").get('v')
    sys.stdout.write(FMTSTR % (elem.tag[:1], elem.get('id'), wikimedia_commons))
