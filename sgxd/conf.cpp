#include <bitset>
#include <cstdio>
#include <vector>
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"


///Unpacks variable configurations from CONF data
void unpackConf(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack CONF\n");
    
    sgd_inf.conf = {};
    if (!sgd_beg || !in || length < 8) return;
    
    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.conf;
    
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
    
    if (sgd_debug) fprintf(stderr, "    Read CONF Header\n");
    out.flag = get_int();
    out.conf.resize(get_int());
    
    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);
    
    if (sgd_debug) fprintf(stderr, "    Read CONF Text\n");
    for (auto &c : out.conf) {
        c.flag = get_int();
        c.name = get_str(sgd_beg, get_int());
        c.text = get_str(sgd_beg, get_int());
        
        if (sgd_debug) {
            fprintf(stderr, "        Current configuration: %d\n", &c - out.conf.data());
            fprintf(stderr, "            Local flag: 0x%08X\n", c.flag);
            fprintf(stderr, "            Name: %s\n", c.name.c_str());
            fprintf(stderr, "            Size: %d\n", c.text.size());
        }
    }
}

///Extracts variable configurations into string
std::string extractConf() {
    if (sgd_debug) fprintf(stderr, "    Extract CONF info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0 - 1); snprintf(out.data() + s1, s0, in, args...);
    };
    
    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.conf.flag).to_string().c_str());
    set_fstr("Configurations:\n");
    for (const auto &c : sgd_inf.conf.conf) {
        set_fstr("    Configuration: %d\n", &c - sgd_inf.conf.conf.data());
        set_fstr("        Configuration Flags: %s\n", std::bitset<32>(c.flag).to_string().c_str());
        set_fstr("        Name: %s\n", (!c.name.empty()) ? c.name.c_str() : "(none)");
        set_fstr("        Size: %d\n", c.text.size());
    }
    
    return out;
}