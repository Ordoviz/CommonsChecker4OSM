.PHONY: geojson
geojson: out.geojson out-maproulette.geojson

.PHONY: tsv
tsv: out.tsv

planet.pbf:
	@echo "Missing planet.pbf: See https://wiki.openstreetmap.org/wiki/Planet.osm#Downloading for how to download an Osmium-compatible planet file." >&2; exit 1
	# You could add a command here to download planet.pbf with a BitTorrent client

commonswiki-all-titles.gz: FORCE
	curl -z commonswiki-all-titles.gz https://dumps.wikimedia.org/commonswiki/latest/commonswiki-latest-all-titles.gz -o commonswiki-all-titles.gz

commonscats-in-commons.txt: commonswiki-all-titles.gz
	# namespace 14 is "Category:"
	zcat commonswiki-all-titles.gz | grep ^14 | cut -f2  > commonscats-in-commons.txt

commonscats-in-osm.xml: planet.pbf
	osmium tags-filter -R planet.pbf 'nwr/wikimedia_commons=Category:*' -o commonscats-in-osm.xml --overwrite

planet-filtered.geojson: planet.pbf
	osmium tags-filter -t planet.pbf 'nwr/wikimedia_commons=Category:*' -o planet-filtered.pbf --overwrite
	osmium export planet-filtered.pbf -c config.json -o planet-filtered.geojson -f jsonseq --overwrite

out.tsv: commonscats-in-osm.xml commonscats-in-commons.txt
	./main.py out.tsv

out.geojson: planet-filtered.geojson commonscats-in-commons.txt
	./main.py out.geojson

out-maproulette.geojson: out.geojson
	./to_maproulette.py out.geojson > out-maproulette.geojson

FORCE: ;
