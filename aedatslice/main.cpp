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
"Usage: aedatslice [-h] input-file start stop output-file\n"
"Positional Arguments:\n"
"  input-file   Input file to read data from.\n"
"  start        Timestamp when to start cutting\n"
"  stop         Timestamp when to stop cutting\n"
"  output-file  Filename into which the slice is stored\n"
"Options:\n"
"  -h           Print this help\n"
"All files are expected to be in format AER-DAT 2.0.";


int parse_config(config_t &config, int argc, char *argv[])
{
	config.start = 0;
	config.stop = 0;
	config.infile = "";
	config.outfile = "";

	int opt;
	while ((opt = getopt(argc, argv, "h")) != -1)
		return 1;

	if ((argc - optind) < 4) {
		cerr << "EE: insufficient arguments" << endl;
		return 1;
	}

	config.infile = string(argv[optind]);
	try {
		config.start = boost::lexical_cast<uint32_t>(argv[optind + 1]);
		config.stop  = boost::lexical_cast<uint32_t>(argv[optind + 2]);
	}
	catch (boost::bad_lexical_cast&) {
		cerr << "EE: start/stop need to be in integer format" << endl;
		return 1;
	}
	config.outfile = string(argv[optind+3]);

	if (!config.infile.compare(config.outfile)) {
		cerr << "EE: input and output files are the same" << endl;
		return 1;
	}

	return 0;
}


inline void
write_bigendian(std::ofstream &f, uint32_t i)
{
	uint8_t buf[4];
	buf[0] = (i & 0xff000000) >> 24;
	buf[1] = (i & 0x00ff0000) >> 16;
	buf[2] = (i & 0x0000ff00) >> 8;
	buf[3] = (i & 0x000000ff);
	f.write((char*)&buf, sizeof(buf));
}


void process(config_t &cfg)
{
	ifstream fi;
	ofstream fo;

	fi.open(cfg.infile, ios_base::in | ios_base::binary);
	fo.open(cfg.outfile, ios_base::out | ios_base::binary);

	const char header[] =
		"#!AER-DAT2.0\r\n"
		"# This is a raw AE data file created by aedatslice (https://github.com/rochus/aedatutils)\r\n"
		"# Data format is int32 address, int32 timestamp (8 bytes total), repeated for each event\r\n"
		"# Timestamps tick is 1 us\r\n";

	fo.write(header, strlen(header));

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

	uint32_t eventsWritten = 0;
	uint32_t nEvents = (len - pos) / bytesPerEvent;
	for (uint32_t i = 0; i < nEvents; i++) {
		// read into little endian (as we are on x86 usually...)
		char buf[4];

		// data
		buf[0] = fi.get();
		buf[1] = fi.get();
		buf[2] = fi.get();
		buf[3] = fi.get();
		uint32_t data = (uint32_t)buf[0]
			      | (uint32_t)buf[1]<<8
			      | (uint32_t)buf[2]<<16
			      | (uint32_t)buf[3]<<24;

		// timestamp
		buf[0] = fi.get();
		buf[1] = fi.get();
		buf[2] = fi.get();
		buf[3] = fi.get();
		uint32_t time = (uint32_t)buf[0]
			      | (uint32_t)buf[1]<<8
			      | (uint32_t)buf[2]<<16
			      | (uint32_t)buf[3]<<24;

		// store the thing if the time is in the slice
		if (time >= cfg.start && time <= cfg.stop) {
			write_bigendian(fo, data);
			write_bigendian(fo, time);
			++eventsWritten;
		}

		// finish if we exceed the given time-slice
		if (time > cfg.stop) break;
	}

	cout << "Events read:    " << nEvents << endl;
	cout << "Events written: " << eventsWritten << endl;

	fi.close();
	fo.close();
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
