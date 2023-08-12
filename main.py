#!/usr/bin/env python3
import xml.etree.ElementTree as ET
import gzip
import sys
import json

VALID_CATS = "commonscats-in-commons.txt"

def load_valid_categories():
    cats = set()
    try:
        with open(VALID_CATS) as f:
            for line in f:
                cats.add(line.rstrip('\n'))
    except FileNotFoundError:
        with gzip.open("commonswiki-latest-all-titles.gz", "rt") as f_in, open(VALID_CATS, "w") as f_out:
            for line in f_in:
                ns, category_name = line.split('\t', 1)
                if ns == '14': # "Category:" namespace
                    cats.add(category_name.rstrip('\n'))
                    f_out.write(category_name)
    return cats


def tsv(cats, outfile):
    with open(outfile, "w") as f:
        tree = ET.parse("commonscats-in-osm.xml")
        for elem in tree.findall(".//*tag[@k='wikimedia_commons']/.."):
            wikimedia_commons = elem.find("tag[@k='wikimedia_commons']").get('v').removeprefix("Category:")
            if wikimedia_commons.replace(' ', '_') not in cats:
                f.write(f"{elem.tag[:1]}/{elem.get('id')}\t{wikimedia_commons}\n")


def geojson(cats, outfile):
    with open(outfile, "wb") as f:
        for line in open("planet-filtered.geojson", "rb"):
            wikimedia_commons = json.loads(line[1:])["properties"]["wikimedia_commons"]
            if wikimedia_commons.removeprefix("Category:").replace(' ', '_') not in cats:
                f.write(line)


def usage():
    print("""Usage: ./main.py outfile.geojson
       ./main.py outfile.tsv""",
        file=sys.stderr)
    exit(1)


def main():
    if len(sys.argv) != 2:
        usage()

    outfile = sys.argv[1]
    if outfile.endswith(".tsv"):
        tsv(load_valid_categories(), outfile)
    elif outfile.endswith(".geojson"):
        geojson(load_valid_categories(), outfile)
    else:
        usage()


if __name__ == "__main__":
    main()
