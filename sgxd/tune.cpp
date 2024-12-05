#include <bitset>
#include <cstdio>
#include <vector>
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"


///Unpacks variable tuning definitions from TUNE data
void unpackTune(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack TUNE\n");
    
    sgd_inf.tune = {};
    if (!sgd_beg || !in || length < 8) return;
    
    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.tune;
    
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
    
    if (sgd_debug) fprintf(stderr, "    Read TUNE Header\n");
    out.flag = get_int();
    out.tune.resize(get_int());
    
    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);
    
    if (sgd_debug) fprintf(stderr, "    Read TUNE\n");
    for (auto &t : out.tune) {
        t.flag = get_int();
        t.labl = get_int();
        t.type = get_int();
        t.defn = get_int();
        in += 4;
        t.data.resize(get_int());
        t.name = get_str(sgd_beg, get_int());
        if (t_sz = get_int()) for (auto &d : t.data) d = *(sgd_beg + t_sz++);
        
        if (sgd_debug) {
            fprintf(stderr, "        Current tuning: %d\n", &t - out.tune.data());
            fprintf(stderr, "            Local flag: 0x%08X\n", t.flag);
            fprintf(stderr, "            Name: %s\n", t.name.c_str());
            fprintf(stderr, "            Label: %d\n", t.labl);
            fprintf(stderr, "            Type: %d\n", t.type);
            fprintf(stderr, "            Definition: %d\n", t.defn);
            fprintf(stderr, "            Size: %d\n", t.data.size());
        }
    }
}

///Extracts variable tuning definitions into string
std::string extractTune() {
    if (sgd_debug) fprintf(stderr, "    Extract TUNE info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0); snprintf(out.data() + s1, s0, in, args...);
    };
    
    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.tune.flag).to_string().c_str());
    set_fstr("Definitions:\n");
    for (const auto &t : sgd_inf.tune.tune) {
        set_fstr("    Tune: %d\n", &t - sgd_inf.tune.tune.data());
        set_fstr("        Tuning Flags: %s\n", std::bitset<32>(t.flag).to_string().c_str());
        set_fstr("        Label: %d\n", t.labl);
        set_fstr("        Type: %d\n", t.type);
        set_fstr("        Definition: %d\n", t.defn);
        set_fstr("        Name: %s\n", (!t.name.empty()) ? t.name.c_str() : "(none)");
        set_fstr("        Size: %d\n", t.data.size());
    }
    
    return out;
}