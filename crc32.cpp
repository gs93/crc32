#include <iostream>
#include <string>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/crc.hpp>

using namespace std;

const string colorReset     = "\x1b[39m", 
             colorGreen     = "\x1b[32m",
             colorRed       = "\x1b[31m",
             colorMagenta   = "\x1b[35m";

unsigned long calculateCrc32(ifstream &inf)
{
    const size_t maxRead = 1048576;
    char buffer[maxRead];
    boost::crc_32_type chksum;
    while (inf) {
        inf.read(buffer, maxRead);
        chksum.process_bytes(buffer, inf.gcount());
    }
    return chksum.checksum();
}

unsigned long getCrc32FromFilename(const string &file)
{
    const boost::regex crcRegex("([\\[\\(]+)([a-fA-F0-9]{8})([\\]\\)]+)");
    boost::match_results<string::const_iterator> what;
    if (boost::regex_search(file.begin(), file.end(), what, crcRegex)) {
        unsigned long crc;
        // 0: complete match, 1: first match, 2: second match, 3: third match
        istringstream(what[2]) >> hex >> crc; // convert hex string into dec ulong
        return crc;
    }
    return 0; // nothing found
}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        cout << argv[0] << " file [file2 [ ..." << endl;
        return 1;
    }

    ifstream inf;
    bool errorOccurred = false;
    for (int arg = 1; argv[arg]; arg++) {
        inf.open(argv[arg], ios::binary);
        if (inf) {
            cout << "           " << argv[arg];
            cout.flush();

            unsigned long calcCrc = calculateCrc32(inf),
                wantedCrc = getCrc32FromFilename(argv[arg]);

            cout << "\r";
            if (calcCrc == wantedCrc)
                cout << colorGreen;
            else if (wantedCrc == 0) // no crc found
                cout << colorMagenta;
            else
                cout << colorRed;

            cout.width(8);
            cout << hex << calcCrc << colorReset << "   " << argv[arg] << endl;
        } else {
            errorOccurred = true;
            cout << colorRed << "    fail" << colorReset << "   " << argv[arg] << endl;
        }
        inf.close();
    }
    return errorOccurred;
}
