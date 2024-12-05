#include <bitset>
#include <cstdio>
#include <vector>
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"


///Unpacks variable audio envelope definitions from ADSR data
void unpackAdsr(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack ADSR\n");
    
    sgd_inf.adsr = {};
    if (!sgd_beg || !in || length < 8) return;
    
    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.adsr;
    
    auto get_int = [&in, &in_end]() -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < 4; ++i) {
            if (in >= in_end) break;
            out |= (unsigned)*(in++) << (8 * i);
        }
        return out;
    };
    
    if (sgd_debug) fprintf(stderr, "    Read ADSR Header\n");
    out.flag = get_int();
    out.adsr.resize(get_int());
    
    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);
    
    if (sgd_debug) fprintf(stderr, "    Read ADSR\n");
    for (auto &a : out.adsr) {
        a = get_int();
        
        if (sgd_debug) {
            fprintf(stderr, "        Current thingy: %d\n", &a - out.adsr.data());
            fprintf(stderr, "            Thingy: 0x%08X\n", a);
        }
    }
}

///Extracts variable audio envelope definitions into string
std::string extractAdsr() {
    if (sgd_debug) fprintf(stderr, "    Extract ADSR info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0); snprintf(out.data() + s1, s0, in, args...);
    };
    
    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.adsr.flag).to_string().c_str());
    set_fstr("Envelopes:\n");
    for (const auto &a : sgd_inf.adsr.adsr) {
        set_fstr("    Envelope: %d\n", &a - sgd_inf.adsr.adsr.data());
        set_fstr("        ADSR: 0x%08X\n", a);
    }
    
    return out;
}