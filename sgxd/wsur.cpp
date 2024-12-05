#include <bitset>
#include <cstdio>
#include <vector>
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"


///Unpacks variable waveform surround definitions from WSUR data
void unpackWsur(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack WSUR\n");
    
    sgd_inf.wsur = {};
    if (!sgd_beg || !in || length < 8) return;
    
    const unsigned char *in_end = in + length;
    auto &out = sgd_inf.wsur;
    
    auto get_int = [&in, &in_end](const unsigned length) -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < length; ++i) {
            if (in >= in_end) break;
            out |= (unsigned)*(in++) << (8 * i);
        }
        return out;
    };
    
    if (sgd_debug) fprintf(stderr, "    Read WSUR Header\n");
    out.flag = get_int(4);
    out.wsur.resize(get_int(4));
    signed suroffs[out.wsur.size()] {};
    
    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);
    
    if (sgd_debug) fprintf(stderr, "    Read WSUR Setup\n");
    for (int w = 0; w < out.wsur.size(); ++w) {
        out.wsur[w].resize(get_int(4));
        suroffs[w] = get_int(4);
    }
    
    if (sgd_debug) fprintf(stderr, "    Read WSUR Definition\n");
    for (int w = 0; w < out.wsur.size(); ++w) {
        in = (unsigned char*)sgd_beg + suroffs[w];
        
        if (sgd_debug) fprintf(stderr, "        Current WSUR: %d\n", w);
        for (auto &c : out.wsur[w]) {
            c.flag = *(in++);
            c.chnid = *(in++);
            c.angle = get_int(2);
            c.dist = get_int(2);
            c.level = get_int(2);
            
            if (sgd_debug) {
                fprintf(stderr, "                Current channel: %d\n", &c - out.wsur[w].data());
                fprintf(stderr, "                    Local flag: 0x%02X\n", c.flag);
                fprintf(stderr, "                    Channel ID: %d\n", c.chnid);
                fprintf(stderr, "                    Angle: %d\n", c.angle);
                fprintf(stderr, "                    Distance: %d\n", c.dist);
                fprintf(stderr, "                    Level: %d\n", c.level);
            }
        }
    }
}

///Extracts variable waveform surround definitions into string
std::string extractWsur() {
    if (sgd_debug) fprintf(stderr, "    Extract WSUR info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0); snprintf(out.data() + s1, s0, in, args...);
    };
    
    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.wsur.flag).to_string().c_str());
    set_fstr("Definitions:\n");
    for (const auto &d : sgd_inf.wsur.wsur) {
        set_fstr("    Definition: %d\n", &d - sgd_inf.wsur.wsur.data());
        for (const auto &c : d) {
            set_fstr("        Channel: %d\n", &c - d.data());
            set_fstr("            Channel Flags: %s\n", std::bitset<8>(c.flag).to_string().c_str());
            set_fstr("            Channel ID: %d\n", c.chnid);
            set_fstr("            Angle: %d\n", c.angle);
            set_fstr("            Distance: %d\n", c.dist);
            set_fstr("            Level: %d\n", c.level);
        }
    }
    
    return out;
}