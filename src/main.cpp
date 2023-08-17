#include "BooPHF.h"
#include <cstddef>
#include <cstdio>

#define NEEDLE "wikimedia_commons\":\""

using namespace std;

typedef boomphf::SingleHashFunctor<uint64_t> hasher_t;
typedef boomphf::mphf<  uint64_t, hasher_t > boophf_t;

// OSM tag values have at most 255 characters of max 4 bytes each (in UTF-8 encoding).
// TSV input format also needs space for "n/9723028490\t".
char buf[256 * 5];

uint64_t hash_str(char *str) {
	// requires c++17
	// probability of collision is about 0.0003 after 100 million hashes
	return hash<string_view>{}(str);
}

/**
 * Copies src to buf until first unescaped quote and unescapes other quotes on the way.
 * Returns start of buf.
 */
char * decodeJSONstring(char *src) {
	int i, j;
	for (i = 0, j = 0; src[i] != '"'; i++) {
		if (src[i] == '\\' && src[i+1] == '"')
			i++;
		buf[j++] = src[i];
	}
	buf[j++] = '\n'; // add newline because hashed keys end with newline
	buf[j]   = '\0';
	return buf;
}

/**
 * Replace all occurrence of oldChar with newChar
 */
void replaceAll(char *str, char oldChar, char newChar) {
	int i = 0;

	while (str[i] != '\0') {
		if (str[i] == oldChar) str[i] = newChar;
		i++;
	}
}

class InvalidCommonsTagDetector {
public:
	InvalidCommonsTagDetector(ulong nelem, FILE *keyfile, const char *prefix, int nthreads)
		: prefix(prefix)
		, prefixlen(strlen(prefix))
	{
		uint64_t *keys      = (uint64_t *) calloc(nelem, sizeof(uint64_t));
		          checkbyte = (uint8_t  *) malloc(nelem);

		ulong i = 0;
		while (fgets(buf, sizeof buf, keyfile) && i < nelem) {
			keys[i] = hash_str(buf);
			i++;
		}
		if (i != nelem) {
			fprintf(stderr, "keyfile only has %lu lines, not %lu as you specified\n", i, nelem);
			nelem = i;
		}
		fclose(keyfile);
	
		auto data_iterator = boomphf::range(static_cast<const uint64_t*>(keys), static_cast<const uint64_t*>(keys+nelem));
	
		bphf = new boomphf::mphf<uint64_t,hasher_t>(nelem,data_iterator,nthreads,gammaFactor);
	
		for (i = 0; i < nelem; i++) {
			checkbyte[bphf->lookup(keys[i])] = (uint8_t) keys[i];
		}
		free(keys);
	}

	/**
	 * Returns whether str is a bad title (starts with prefix was not among
	 * the keys during hash function construction)
	 */
	bool detect(char *str) {
		if (memcmp(str, prefix, prefixlen))
			return false;
		str += prefixlen;
		replaceAll(str, ' ', '_'); // MediaWiki page title dump uses underscores
		uint64_t hash = hash_str(str);
		uint64_t idx = bphf->lookup(hash);
		return idx == ULONG_MAX || checkbyte[idx] != (uint8_t) hash;
	}


	~InvalidCommonsTagDetector()
	{
		free(checkbyte);
		delete bphf;
	}

private:
	const double gammaFactor = 9.0; // lowest bit/elem is achieved with gamma=1, higher values lead to faster construction and query
	boophf_t * bphf;
	uint8_t  * checkbyte;
	const char *prefix;
	const size_t prefixlen;
};

int main(int argc, char* argv[]) {
	if (argc < 5 || argc > 6) {
		fprintf(stderr,
			"Usage: %s <prefix> <lines in keyfile> <keyfile> <osmfile> [nthreads]\n"
			"\n"
			"Arguments:\n"
			"  <prefix>:  prefix (e.g. 'Category:' or 'File:')\n"
			"  <keyfile>: list of MediaWiki page titles (one per line)\n"
			"  <osmfile>: .tsv or .geojson file generated with make\n"
			"  [nthreads]: how many threads to use (defaults to 1)\n"
			"\n"
			"Example:\n"
			"   %s File: $(wc -l data/commonswiki-files.txt) data/planet-filtered.tsv $(nproc)\n"
			, argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	const char *prefix = argv[1];
	const ulong nelem = stoi(argv[2]);
	const char *keyfilename = argv[3];
	const char *osmfilename= argv[4];

	const uint nthreads = (argc == 6) ? stoi(argv[5]) : 1;

	FILE *keyfile = fopen(keyfilename, "r");
	if (!keyfile) {
		perror(keyfilename);
		return EXIT_FAILURE;
	}
	FILE *osmfile = fopen(osmfilename, "r");
	if (!osmfile) {
		perror(osmfilename);
		return EXIT_FAILURE;
	}


	const char *osmfilesuffix = osmfilename + strlen(osmfilename) - 4;
	if (!memcmp(osmfilesuffix, ".tsv", 4)) {
		InvalidCommonsTagDetector detector(nelem, keyfile, prefix, nthreads);
		while (fgets(buf, sizeof buf, osmfile)) {
			char *commonsname = buf;
			char *osmId = strsep(&commonsname, "\t");
			if (commonsname && detector.detect(commonsname))
				cout << osmId << "\t" << commonsname;
		}
	} else if (!memcmp(osmfilesuffix, "json", 4)) {
		InvalidCommonsTagDetector detector(nelem, keyfile, prefix, nthreads);
		char* line = NULL;
		size_t len = 0;
		ssize_t nread;
		while ((nread = getline(&line, &len, osmfile)) != -1) {
			// jsonline has this format:
			// {"type":"Feature","geometry":{...},"properties":{"@type":"node","@id":417089768,"wikimedia_commons":"File:Haltestelle_\"Rathaus\"_Hof_20200104_03.jpg"}}
			char *commonsname = strstr(line, NEEDLE);
			if (commonsname && detector.detect(decodeJSONstring(commonsname + strlen(NEEDLE)))) {
				line[nread-1] = '\0'; // strip newline
				// MapRoulette-friendly output format:
				printf("\x1e{\"type\":\"FeatureCollection\",\"features\":[%s]}\n", line + 1);
			}
		}
		free(line);
	} else {
		fprintf(stderr, "%s is not a .tsv or .geojson file\n", osmfilename);
		return EXIT_FAILURE;
	}
	fclose(osmfile);

	return EXIT_SUCCESS;
}
