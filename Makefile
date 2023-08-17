yesterday != date -d yesterday +"%Y%m%d"

CXX ?= g++
CFLAGS = -O3 -std=c++17 -lpthread

main: src/main.cpp src/BooPHF.h
	$(CXX) -o $@ $< $(CFLAGS)


### NECESSARY INTERMEDIATE FILES: --------------------------------

data/planet.pbf:
	@echo "Missing data/planet.pbf: See https://wiki.openstreetmap.org/wiki/Planet.osm#Downloading for how to download an Osmium-compatible planet file." >&2; exit 1
	# You could add a command here to download data/planet.pbf with a BitTorrent client

data/commonswiki-all-titles.gz: FORCE
	curl -z data/commonswiki-all-titles.gz https://dumps.wikimedia.org/commonswiki/latest/commonswiki-latest-all-titles.gz -o data/commonswiki-all-titles.gz

data/commonswiki-cats.txt: data/commonswiki-all-titles.gz
	# namespace 14 is "Category:"
	zcat data/commonswiki-all-titles.gz | grep ^14 | cut -f2  > data/commonswiki-cats.txt

data/commonswiki-files.txt.gz: FORCE
	curl -z data/commonswiki-files.txt.gz https://dumps.wikimedia.org/other/mediatitles/$(yesterday)/commonswiki-$(yesterday)-all-media-titles.gz -o data/commonswiki-files.txt.gz

data/commonswiki-files.txt: data/commonswiki-files.txt.gz
	gunzip --keep data/commonswiki-files.txt.gz

data/planet-filtered.tsv: data/planet.pbf
	osmium tags-filter -R data/planet.pbf 'nwr/wikimedia_commons' -f xml | ./xml2tsv.py > data/planet-filtered.tsv

data/planet-filtered.geojson: data/planet.pbf
	osmium tags-filter -t data/planet.pbf 'nwr/wikimedia_commons' -o data/planet-filtered.pbf --overwrite
	osmium export data/planet-filtered.pbf -c src/config.json -o data/planet-filtered.geojson -f jsonseq --overwrite


### OUTPUT FILES: ------------------------------------------------

out/cats.tsv: main data/commonswiki-cats.txt data/planet-filtered.tsv
	# ./main.py Category: data/commonswiki-cats.txt data/planet-filtered.tsv > out/cats.tsv
	./main Category: $$(wc -l data/commonswiki-cats.txt) data/planet-filtered.tsv $$(nproc) > out/cats.tsv

out/cats.geojson: main data/commonswiki-cats.txt data/planet-filtered.geojson
	# ./main.py Category: data/commonswiki-cats.txt data/planet-filtered.geojson > out/cats.geojson
	./main Category: $$(wc -l data/commonswiki-cats.txt) data/planet-filtered.geojson $$(nproc) > out/cats.geojson

out/files.tsv: main data/commonswiki-files.txt data/planet-filtered.tsv
	./main File: $$(wc -l data/commonswiki-files.txt) data/planet-filtered.tsv $$(nproc) > out/files.tsv

out/files.geojson: main data/commonswiki-files.txt data/planet-filtered.geojson
	./main File: $$(wc -l data/commonswiki-files.txt) data/planet-filtered.geojson $$(nproc) > out/files.geojson

FORCE: ;
