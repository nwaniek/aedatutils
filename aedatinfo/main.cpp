#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include <string>

using namespace std;

/*
 * aedat files contain the following items. If a line begins with #, it is
 * expected to be a comment
 */
typedef struct {
	uint32_t data;
	uint32_t timestamp;
} aedat_item;


/*
 * configuration for aedatslice
 */
typedef struct {
	uint32_t start;
	uint32_t stop;
	string infile;
	string outfile;
} config_t;


static const char *usage =
"Usage: aedatinfo [-h] input-file\n"
"Positional Arguments:\n"
"  input-file   Input file to read data from.\n"
"Options:\n"
"  -h           Print this help\n"
"Files are expected to be in format AER-DAT 2.0.";


int parse_config(config_t &config, int argc, char *argv[])
{
	config.infile = "";
	config.outfile = "";

	int opt;
	while ((opt = getopt(argc, argv, "h")) != -1)
		return 1;

	if ((argc - optind) < 1) {
		cerr << "EE: insufficient arguments" << endl;
		return 1;
	}

	config.infile = string(argv[optind]);
	return 0;
}


void process(config_t &cfg)
{
	ifstream fi;
	fi.open(cfg.infile, ios_base::in | ios_base::binary);

	// skip all header lines, compute number of events stored in the file
	uint32_t pos = fi.tellg();
	while (fi.get() == '#') {
		string s;
		getline(fi, s);
		pos = fi.tellg();
	}
	fi.seekg(0, ios::end);
	uint32_t len = fi.tellg();
	fi.seekg(pos);
	uint32_t bytesPerEvent = 8;

	// TODO: UGLY! WASTE OF COMPUTING RESOURCES> YADDAYADDA!@!11!
	uint32_t nEvents = (len - pos) / bytesPerEvent;
	uint32_t tStart = -1;
	uint32_t tStop = -1;
	for (uint32_t i = 0; i < nEvents; i++) {
		// read into little endian (as we are on x86 usually...)
		char buf[4];

		// data
		buf[0] = fi.get();
		buf[1] = fi.get();
		buf[2] = fi.get();
		buf[3] = fi.get();

		// timestamp
		buf[0] = fi.get();
		buf[1] = fi.get();
		buf[2] = fi.get();
		buf[3] = fi.get();
		uint32_t time = (uint32_t)buf[0]
			      | (uint32_t)buf[1]<<8
			      | (uint32_t)buf[2]<<16
			      | (uint32_t)buf[3]<<24;
		if (i == 0) tStart = time;
		if (i == nEvents - 1) tStop = time;
	}

	cout << "Events: " << nEvents << "\n";
	cout << "tStart: " << tStart << "\n";
	cout << "tStop:  " << tStop << endl;

	fi.close();
}


int main(int argc, char *argv[])
{
	config_t cfg;
	if (parse_config(cfg, argc, argv)) {
		cout << usage << endl;
		return EXIT_FAILURE;
	}
	process(cfg);
	return EXIT_SUCCESS;
}
