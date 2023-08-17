# CommonsChecker4OSM

Finds OpenStreetMap elements tagged with `wikimedia_commons=Category:*` (or `wikimedia_commons=File:*`) where the category (or file) does not exist on Wikimedia Commons.

## How does it work

CommonsChecker4OSM filters a list of OSM elements tagged wtih `wikimedia_commons=*` against a list of valid Commons categories or files. It is basically an `fgrep -f` implementation that processes a 5 gigabyte pattern file efficiently using the [BBHash](https://github.com/rizkg/BBHash) minimal perfect hashing library. A Makefile is provided to download and prepare the input files. A Python script [main.py](./main.py) is included that may help you to understand the equivalent C++ code, but it loads the entire pattern file into RAM.

## How to use

If you want to find invalid categories, you should wait until Wikimedia releases a new data dump (usually at the third day of every month) to minimize false positives. You can get notified by subscribing to [the RSS feed](https://dumps.wikimedia.org/commonswiki/latest/commonswiki-latest-all-titles.gz-rss.xml). If you want to find invalid files, you do not need to wait—the data dump is released every day.

CommonsChecker4OSM has only been tested on Linux. It should work on Windows, but preparing the input files will be annoying. Step-by-step guide:

1. Install [Osmium Tool](https://osmcode.org/osmium-tool/)
2. Clone this repository
3. Inside cloned repo, create `data` and `out` folders
3. Download required files into the `data` directory:
   * `planet.pbf`: an Osmium-compatible [OpenStreetMap planet](https://wiki.openstreetmap.org/wiki/Planet.osm)
4. Choose an output format (see below). Run either `make out/cats.tsv`, `make out/cats.geojson`, `make out/files.tsv`, or `make out/files.geojson` (output filename must be one of these hardcoded strings).

## Output formats

### Tab-separated values (TSV)

Output contains OSM identifier (node, way, or relation ID) and `wikimedia_commons` value, separated by a tab. This is the fastest format to produce.

```console
$ make out/cats.tsv
# osmium takes 10min for entire planet
$ cat out/cats.tsv
n/1573735855	Category:Conservatoire_National_de_V%C3%A9hicules_Historiques
n/286133524	Category:ref:sprockhoff No. 465
n/3022117073	Category:Wildwiesenwarte;Category:Views from the Wildwiesenwarte
n/306593910	Category:Prince George pub, Brighton Good pictures  Advanced...
n/6426478285	Category:Dorfkirche_Mechow_(Kyritz)?uselang=de
w/297069904	Category:https://commons.wikimedia.org/wiki/Category:Gr%C3%BCner_Graben_14_(G%C3%B6rlitz)
w/320276921	Category: Ballyellen Upper Lock
w/474166824	Category:Nages-et-Solorgues#/media/File:Fontaine_Ranquet.jpg
r/12931220	Category:Brandenburger Straße 36;Riedelsberger Weg 2 (Bayreuth)
```

### Line-by-Line GeoJSON

[This format](https://learn.maproulette.org/documentation/line-by-line-geojson/) can be used to create a challenge on maproulette.org. It might lack a few categories that are present in the TSV format.

```console
$ make out/cats.geojson
# osmium takes 22min for entire planet
$ cat cats.geojson
{"type":"FeatureCollection","features":[{"type":"Feature","geometry":{"type":"Point","coordinates":[-2.0835284,53.3600557]},"properties":{"@type":"node","@id":29947059,"wikimedia_commons":"Category:Help Category:Middlewood railway station"}}]}
{"type":"FeatureCollection","features":[{"type":"Feature","geometry":{"type":"LineString","coordinates":[[10.1212064,54.3247979],[10.120334100000001,54.3242942],[10.1192733,54.3236981],[10.1199922,54.3233703],[10.1204628,54.3231298],[10.1209243,54.3228965],[10.1222211,54.3236181],[10.1212064,54.3247979]]},"properties":{"@type":"way","@id":9408975,"wikimedia_commons":"Category:Wilhelmplatz (Kiel)"}}]}
```

## License

CommonsChecker4OSM and BBHash are licensed under the MIT license:

Copyright (c) 2015 Guillaume Rizk (BBHash)
Copyright (c) 2023 Lennard Hofmann (CommonsChecker4OSM)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
