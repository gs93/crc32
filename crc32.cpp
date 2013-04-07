#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <boost/regex.hpp>
#include <boost/crc.hpp>

using namespace std;

const string colorReset     = "\x1b[39m", 
             colorGreen     = "\x1b[32m",
             colorRed       = "\x1b[31m",
             colorMagenta   = "\x1b[35m";

mutex coutMtx;

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

bool checkFile(const string &file)
{
    ifstream inf;
    inf.open(file, ios::binary);
    if (inf) {
        unsigned long calcCrc = calculateCrc32(inf),
            wantedCrc = getCrc32FromFilename(file);

        lock_guard<mutex> lock(coutMtx);
        if (calcCrc == wantedCrc)
            cout << colorGreen;
        else if (wantedCrc == 0) // no crc found
            cout << colorMagenta;
        else
            cout << colorRed;

        cout.width(8);
        cout << hex << calcCrc << colorReset << "   " << file << endl;

        inf.close();
        return true;
    } else {
        lock_guard<mutex> lock(coutMtx);
        cout << colorRed << "    fail" << colorReset << "   " << file << endl;
        return false;
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        cout << argv[0] << " file [file2 [ ..." << endl;
        return 1;
    }

    list<thread *> threads;
    for (int arg = 1; argv[arg]; arg++)
        threads.push_back(new thread(checkFile, argv[arg]));

    for (thread *t : threads)
        t->join();

    return 0;
}
