#include <bitset>
#include <cstdio>
#include <vector>
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"


///Unpacks variable waveform markers from WMKR data
void unpackWmkr(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack WMKR\n");
    
    sgd_inf.wmkr = {};
    if (!sgd_beg || !in || length < 8) return;
    
    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.wmkr;
    
    auto get_int = [&in, &in_end]() -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < 4; ++i) {
            if (in >= in_end) break;
            out |= (unsigned)*(in++) << (8 * i);
        }
        return out;
    };
    auto get_str = [](const unsigned char *in, const unsigned adr) -> std::string {
        std::string out;
        if (!adr);
        else { out = (const char*)(in + adr); if (!out[0]) out.clear(); }
        return out;
    };
    
    if (sgd_debug) fprintf(stderr, "    Read WMKR Header\n");
    out.flag = get_int();
    out.wmkr.resize(get_int());
    signed mkroffs[out.wmkr.size()] {};
    
    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);
    
    if (sgd_debug) fprintf(stderr, "    Read WMKR Setup\n");
    for (int w = 0; w < out.wmkr.size(); ++w) {
        out.wmkr[w].resize(get_int());
        mkroffs[w] = get_int();
    }
    
    if (sgd_debug) fprintf(stderr, "    Read WMKR Definition\n");
    for (int w = 0; w < out.wmkr.size(); ++w) {
        in = (unsigned char*)sgd_beg + mkroffs[w];
        
        if (sgd_debug) fprintf(stderr, "        Current WMKR: %d\n", w);
        for (auto &m : out.wmkr[w]) {
            m.labl = get_str(sgd_beg, get_int());
            m.pos = get_int();
            m.siz = get_int();
            
            if (sgd_debug) {
                fprintf(stderr, "                Current channel: %d\n", &m - out.wmkr[w].data());
                fprintf(stderr, "                    Label: %s\n", m.labl.c_str());
                fprintf(stderr, "                    Position: %d\n", m.pos);
                fprintf(stderr, "                    Size: %d\n", m.siz);
            }
        }
    }
}

///Extracts variable waveform markers into string
std::string extractWmkr() {
    if (sgd_debug) fprintf(stderr, "    Extract WMKR info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0); snprintf(out.data() + s1, s0, in, args...);
    };
    
    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.wmkr.flag).to_string().c_str());
    set_fstr("Markers:\n");
    for (const auto &g : sgd_inf.wmkr.wmkr) {
        set_fstr("    Group: %d\n", &g - sgd_inf.wmkr.wmkr.data());
        for (const auto &m : g) {
            set_fstr("        Marker: %d\n", &m - g.data());
            set_fstr("            Label: %s\n", (!m.labl.empty()) ? m.labl.c_str() : "(none)");
            set_fstr("            Position: %d\n", m.pos);
            set_fstr("            Size: %d\n", m.siz);
        }
    }
    
    return out;
}