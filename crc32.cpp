#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <boost/crc.hpp>
#include <boost/regex.hpp>

using namespace std;

const string colorReset     = "\x1b[39m", 
             colorGreen     = "\x1b[32m",
             colorRed       = "\x1b[31m",
             colorMagenta   = "\x1b[35m";

unsigned long calculateCrc32(ifstream &inf) // {{{1
{
    const size_t maxRead = 1048576;
    char buffer[maxRead];
    boost::crc_32_type chksum;
    while (inf) {
        inf.read(buffer, maxRead);
        chksum.process_bytes(buffer, inf.gcount());
    }
    return chksum.checksum();
} // 1}}}

unsigned long getCrc32FromFilename(const string &file) // {{{1
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
} // 1}}}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        cout << argv[0] << " file [file2 [ ..." << endl;
        return 1;
    }

    ifstream inf;
    bool errorOccurred = false;
    #pragma omp parallel for private(inf) shared(errorOccurred)
    for (int arg = 1; arg < argc; arg++) {
        inf.open(argv[arg], ios::binary);
        if (inf) {
            unsigned long calcCrc = calculateCrc32(inf),
                wantedCrc = getCrc32FromFilename(argv[arg]);

            string col;
            if (calcCrc == wantedCrc) {
                col = colorGreen;
            } else if (wantedCrc == 0) { // no crc found
                col = colorMagenta;
            } else {
                col = colorRed;
            }

            #pragma omp critical
            {
                cout << col << std::setw(8) << hex << calcCrc << colorReset << "   " << argv[arg] << endl;
            }
        } else {
            #pragma omp critical
            {
                errorOccurred = true;
                cout << colorRed << "    fail" << colorReset << "   " << argv[arg] << endl;
            }
        }
        inf.close();
    }
    return errorOccurred;
}
