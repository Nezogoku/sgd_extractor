#include <algorithm>
#include <cstdio>
#include <string>
#define ALLSGXD_IMPLEMENTATION
#include "sgxd/sgxd_func.hpp"
#include "printpause.hpp"


void printOpt(const char *pName) {
    fprintf(stderr, "Usage: %s [-hdt] [<infile(s).sgd/sgh/sgb>]\n\n", pName);
    fprintf(stderr, "Infile:\n");
    fprintf(stderr, "   .sgd        Sony Game Data Archive\n");
    fprintf(stderr, "   .sgh        Sony Game Data Archive Header\n");
    fprintf(stderr, "                   Must be combined with .sgb file\n");
    fprintf(stderr, "   .sgb        Sony Game Data Archive Body\n");
    fprintf(stderr, "                   Must be combined with .sgh file\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -h          Prints this message\n");
    fprintf(stderr, "   -d          Toggles debug mode\n");
    fprintf(stderr, "   -t          Activates textual extraction mode\n");
}


int main(int argc, char *argv[]) {
    bool debug = false;

    std::string prgm = argv[0];
    prgm.erase(std::remove(prgm.begin(), prgm.end(), '\"'), prgm.end());
    prgm = prgm.substr(prgm.find_last_of("\\/") + 1);
    prgm = prgm.substr(0, prgm.find_last_of('.'));

    if (argc < 2) { printOpt(prgm.c_str()); }
    else {
        std::string sgh, sgb, tfle;
        auto get_sgd = [&](const char *s0, const char *s1 = 0) -> void {
            sgd_debug = debug;
            unpackSgxd(s0, s1);

            std::string pth = s0;
            pth = pth.substr(0, pth.find_last_of("\\/") + 1);
            extractSgxd(pth.c_str());
            
            tfle.clear(); sgh.clear(); sgb.clear();
        };

        for (int i = 1; i < argc; ++i) {
            tfle = argv[i];
            tfle.erase(std::remove(tfle.begin(), tfle.end(), '\"'), tfle.end());

            if (tfle == "-h") { printOpt(prgm.c_str()); break; }
            else if (tfle == "-d") { debug = !debug; continue; }
            else if (tfle == "-t") { sgd_text = true; continue; }

            if (tfle.rfind(".sgd") != std::string::npos) {
                if (debug) fprintf(stderr, "This is a game data archive file\n");
                get_sgd(tfle.c_str());
            }
            else if (tfle.rfind(".sgh") != std::string::npos) {
                if (debug) fprintf(stderr, "This is a game data archive header file\n");
                sgh = tfle;
                if (sgb.empty()) continue;
            }
            else if (tfle.rfind(".sgb") != std::string::npos) {
                if (debug) fprintf(stderr, "This is a game data archive header file\n");
                sgb = tfle;
                if (sgh.empty()) continue;
            }
            else {
                if (debug) fprintf(stderr, "This file is unknown\n");
                continue;
            }
            
            get_sgd(sgh.c_str(), sgb.c_str());
        }
    }

    fprintf(stdout, "\nEnd of operations\n");

    sleep(10);
    return 0;
}
