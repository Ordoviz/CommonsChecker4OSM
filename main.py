#!/usr/bin/env python3
import sys
import json

class InvalidCommonsTagDetector:
    def __init__(self, prefix, keyfile):
        self.prefix = prefix
        with open(keyfile) as f:
            self.titles = {line for line in f}

    def detect(self, str):
        return (str.startswith(self.prefix) and
                str.removeprefix(self.prefix).replace(' ', '_') not in self.titles)


def print_tsv(detector: InvalidCommonsTagDetector, tsvfile):
    for line in open(tsvfile, "r"):
        fields = line.split('\t')
        if len(fields) != 2:
            sys.stderr.write("malformed input line: " + line)
            continue
        if detector.detect(fields[1]):
            sys.stdout.write(line)


def print_geojson(detector: InvalidCommonsTagDetector, geojsonfile):
    for line in open(geojsonfile, "rb"):
        if detector.detect(json.loads(line[1:])["properties"]["wikimedia_commons"] + '\n'):
            # MapRoulette-friendly output format:
            sys.stdout.buffer.write(b'\x1e{"type":"FeatureCollection","features":[%s]}\n'
                    % line[1:].rstrip(b'\n'))


def usage():
    print("""Usage: ./main.py <prefix> <keyfile> <osmfile>

Arguments:
  <prefix>: namespace prefix (e.g. "Category:" or "File:")
  <keyfile>:  list of MediaWiki page titles (one per line)
  <osmfile>:  .tsv or .geojson file generated with make

Example:
   ./main.py Category: data/commonswiki-cats.txt data/planet-filtered.tsv""",
        file=sys.stderr)
    exit(1)


def main():
    if len(sys.argv) != 4:
        usage()

    prefix = sys.argv[1]
    keyfile = sys.argv[2]
    osmfile = sys.argv[3]

    if osmfile.endswith(".tsv"):
        detector = InvalidCommonsTagDetector(prefix, keyfile)
        print_tsv(detector, osmfile)
    elif osmfile.endswith(".geojson"):
        detector = InvalidCommonsTagDetector(prefix, keyfile)
        print_geojson(detector, osmfile)
    else:
        usage()


if __name__ == "__main__":
    main()
