#include <bitset>
#include <cstdio>
#include <vector>
#include "sgxd_const.hpp"
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"


///Unpacks variable name definitions from NAME data
void unpackName(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack NAME\n");
    
    sgd_inf.name = {};
    if (!sgd_beg || !in || length < 8) return;

    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.name;

    auto get_int = [&in, &in_end](const unsigned length) -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < length; ++i) {
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

    if (sgd_debug) fprintf(stderr, "    Read NAME Header\n");
    out.flag = get_int(4);
    out.name.resize(get_int(4));

    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);

    if (sgd_debug) fprintf(stderr, "    Read NAME\n");
    for (auto &n : out.name) {
        n.reqsmp = get_int(2);
        n.reqseq = *(in++);
        n.type = *(in++);
        n.name = get_str(sgd_beg, get_int(4));

        if (sgd_debug) {
            fprintf(stderr, "        Current name: %d\n", &n - out.name.data());
            fprintf(stderr, "            Wave request ID: %d\n", n.reqsmp);
            fprintf(stderr, "            Sequence request ID: %d\n", n.reqseq);
            fprintf(stderr, "            Request type: 0x%02X\n", n.type);
            fprintf(stderr, "            Name: %s\n", n.name.c_str());
        }
    }
}

///Extracts variable name definitions into string
std::string extractName() {
    if (sgd_debug) fprintf(stderr, "    Extract NAME info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0 - 1); snprintf(out.data() + s1, s0, in, args...);
    };

    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.name.flag).to_string().c_str());
    set_fstr("Names:\n");
    for (const auto &n : sgd_inf.name.name) {
        set_fstr("    Name: %d\n", &n - sgd_inf.name.name.data());
        set_fstr("        Sample: %d\n", n.reqsmp);
        set_fstr("        Sequence: %d\n", n.reqseq);
        set_fstr("        Type: ");
        switch((n.type >> 4) & 0x0F) {
            case REQUEST_STREAM:    set_fstr("STREAM\n"); break;
            case REQUEST_NOTE:      set_fstr("NOTE\n"); break;
            case REQUEST_SEQUENCE:  set_fstr("SEQUENCE\n"); break;
            case REQUEST_SAMPLE:    set_fstr("SAMPLE\n"); break;
            case REQUEST_SETUP:     set_fstr("SETUP\n"); break;
            case REQUEST_EFFECT:    set_fstr("EFFECT\n"); break;
            case REQUEST_CONFIG:    set_fstr("CONFIGURATION\n"); break;
            default:                set_fstr("UNKNOWN\n"); break;
        }
        set_fstr("        Name: %s\n", (!n.name.empty()) ? n.name.c_str() : "(none)");
    }

    return out;
}
