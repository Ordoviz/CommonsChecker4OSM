# CommonsCatOSM

Finds OpenStreetMap elements tagged with `wikimedia_commons=Category:*` where the category does not exist on Wikimedia Commons.

## How to use

To minimize false positives, wait until Wikimedia releases a new data dump (usually at the third day of every month). You can get notified by subscribing to [the RSS feed](https://dumps.wikimedia.org/commonswiki/latest/commonswiki-latest-all-titles.gz-rss.xml).

1. Install [Osmium Tool](https://osmcode.org/osmium-tool/)
2. Clone this repository
3. Download required files into the same directory:
   * `planet.pbf`: an Osmium-compatible [OpenStreetMap planet](https://wiki.openstreetmap.org/wiki/Planet.osm)
   * [commonswiki-latest-all-titles.gz](https://dumps.wikimedia.org/commonswiki/latest/commonswiki-latest-all-titles.gz) (1.1 GB as of 2023)
4. Choose an output format (see below). If you use a Unix-like operating system, you can run `make tsv` or `make geojson`. Otherwise, run the commands given below.

## Output formats

### Tab-separated values (TSV)

Output contains invalid category names and the OpenStreetMap ID of the node, way, or relation with that category. This is the fastest format to produce. 

```console
# Make sure you have 10min to spare
$ osmium tags-filter -R planet.pbf 'nwr/wikimedia_commons=Category:*' -o commonscats-in-osm.xml
$ python main.py out.tsv
$ cat out.tsv
n/1573735855	Conservatoire_National_de_V%C3%A9hicules_Historiques
n/286133524	ref:sprockhoff No. 465
n/3022117073	Wildwiesenwarte;Category:Views from the Wildwiesenwarte
n/306593910	Prince George pub, Brighton Good pictures  Advanced...
n/6426478285	Dorfkirche_Mechow_(Kyritz)?uselang=de
w/297069904	https://commons.wikimedia.org/wiki/Category:Gr%C3%BCner_Graben_14_(G%C3%B6rlitz)
w/320276921	 Ballyellen Upper Lock
w/474166824	Nages-et-Solorgues#/media/File:Fontaine_Ranquet.jpg
r/12931220	Brandenburger Straße 36;Riedelsberger Weg 2 (Bayreuth)
```

### Line-by-Line GeoJSON

[This format](https://learn.maproulette.org/documentation/line-by-line-geojson/) can be used to create a challenge on maproulette.org. It might lack a few categories that are present in the TSV format.

```console
# Make sure you have 3GB RAM and 25min to spare
$ osmium tags-filter -t planet.pbf 'nwr/wikimedia_commons=Category:*' -o planet-filtered.pbf
$ osmium export planet-filtered.pbf -c config.json -o planet-filtered.geojson -f jsonseq
$ python main.py out.geojson
$ cat out.geojson
{"type":"Feature","geometry":{"type":"Point","coordinates":[-2.0835284,53.3600557]},"properties":{"@type":"node","@id":29947059,"wikimedia_commons":"Category:Help Category:Middlewood railway station"}}
{"type":"Feature","geometry":{"type":"LineString","coordinates":[[10.1212064,54.3247979],[10.120334100000001,54.3242942],[10.1192733,54.3236981],[10.1199922,54.3233703],[10.1204628,54.3231298],[10.1209243,54.3228965],[10.1222211,54.3236181],[10.1212064,54.3247979]]},"properties":{"@type":"way","@id":9408975,"wikimedia_commons":"Category:Wilhelmplatz (Kiel)"}}
$ python to_maproulette.py out.geojson
{"type":"FeatureCollection","features":[{"type":"Feature","geometry":{"type":"Point","coordinates":[-2.0835284,53.3600557]},"properties":{"@type":"node","@id":29947059,"wikimedia_commons":"Category:Help Category:Middlewood railway station"}}]}
{"type":"FeatureCollection","features":[{"type":"Feature","geometry":{"type":"LineString","coordinates":[[10.1212064,54.3247979],[10.120334100000001,54.3242942],[10.1192733,54.3236981],[10.1199922,54.3233703],[10.1204628,54.3231298],[10.1209243,54.3228965],[10.1222211,54.3236181],[10.1212064,54.3247979]]},"properties":{"@type":"way","@id":9408975,"wikimedia_commons":"Category:Wilhelmplatz (Kiel)"}}]}
```
