#include <bitset>
#include <cstdio>
#include <vector>
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"


///Unpacks variable audio buss from BUSS data
void unpackBuss(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack BUSS\n");
    
    sgd_inf.buss = {};
    if (!sgd_beg || !in || length < 8) return;
    
    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.buss;
    
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
    
    if (sgd_debug) fprintf(stderr, "    Read BUSS Header\n");
    out.flag = get_int(4);
    out.buss.resize(get_int(4));
    signed unitoffs[out.buss.size()] {};
    signed effectoffs[out.buss.size()] {};
    
    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);
    
    if (sgd_debug) fprintf(stderr, "    Read BUSS Setup\n");
    for (int b = 0; b < out.buss.size(); ++b) {
        out.buss[b].flag = get_int(4);
        out.buss[b].name = get_str(sgd_beg, get_int(4));
        out.buss[b].unit.resize(*(in++));
        out.buss[b].effect.resize(*(in++));
        out.buss[b].munit = *(in++);
        in += 5;
        unitoffs[b] = get_int(4);
        effectoffs[b] = get_int(4);
        out.buss[b].oper = get_str(sgd_beg, get_int(4));
        out.buss[b].oparam = get_str(sgd_beg, get_int(4));
        in += 16;
        
        if (sgd_debug) {
            fprintf(stderr, "        Current setup: %d\n", b);
            fprintf(stderr, "            Local flag: 0x%08X\n", out.buss[b].flag);
            fprintf(stderr, "            Name: %s\n", out.buss[b].name.c_str());
            fprintf(stderr, "            Module unit: %d\n", out.buss[b].munit);
            fprintf(stderr, "            Operation: %s\n", out.buss[b].oper.c_str());
            fprintf(stderr, "            Operation parameters: %s\n", out.buss[b].oparam.c_str());
        }
    }
    
    if (sgd_debug) fprintf(stderr, "    Read BUSS Unit\n");
    for (int b = 0; b < out.buss.size(); ++b) {
        in = (unsigned char*)sgd_beg + unitoffs[b];
        
        if (sgd_debug) fprintf(stderr, "        Current BUSS: %d\n", b);
        for (auto &u : out.buss[b].unit) {
            u.flag = get_int(4);
            u.name = get_str(sgd_beg, get_int(4));
            in += 8;
            
            if (sgd_debug) {
                fprintf(stderr, "            Current unit: %d\n", &u - out.buss[b].unit.data());
                fprintf(stderr, "                Local flag: 0x%08X\n", u.flag);
                fprintf(stderr, "                Name: %s\n", u.name.c_str());
            }
        }
    }
    
    if (sgd_debug) fprintf(stderr, "    Read BUSS Effect\n");
    for (int b = 0; b < out.buss.size(); ++b) {
        in = (unsigned char*)sgd_beg + effectoffs[b];
        
        if (sgd_debug) fprintf(stderr, "        Current BUSS: %d\n", b);
        for (auto &e : out.buss[b].effect) {
            e.flag = get_int(4);
            e.name = get_str(sgd_beg, get_int(4));
            e.module = get_str(sgd_beg, get_int(4));
            for (auto &a : e.assign) a = *(in++);
            e.numin = *(in++);
            e.numout = *(in++);
            in += 2;
            for (auto &bi : e.busin) bi = *(in++);
            for (auto &bo : e.busout) bo = *(in++);
            for (auto &gi : e.gainin) gi = get_int(2);
            for (auto &go : e.gainout) go = get_int(2);
            if (t_sz = get_int(4)) e.preset = (const char*)(sgd_beg + t_sz);
            in += 12;
            
            if (sgd_debug) {
                fprintf(stderr, "            Current effect: %d\n", &e - out.buss[b].effect.data());
                fprintf(stderr, "                Local flag: 0x%08X\n", e.flag);
                fprintf(stderr, "                Name: %s\n", e.name.c_str());
                fprintf(stderr, "                Module: %s\n", e.module.c_str());
                fprintf(stderr, "                Assigns:");
                for (const auto &a : e.assign) fprintf(stderr, " %d", a);
                fprintf(stderr, "\n                Bus in values:");
                for (int i = 0; i < e.numin; ++i) fprintf(stderr, " %d", e.busin[i]);
                fprintf(stderr, "\n                Bus out values:");
                for (int i = 0; i < e.numout; ++i) fprintf(stderr, " %d", e.busout[i]);
                fprintf(stderr, "\n                Gain in values:");
                for (int i = 0; i < e.numin; ++i) fprintf(stderr, " %d", e.gainin[i]);
                fprintf(stderr, "\n                Gain out values:");
                for (int i = 0; i < e.numout; ++i) fprintf(stderr, " %d", e.gainout[i]);
                fprintf(stderr, "\n                Preset size: %d\n", e.preset.size());
            }
        }
    }
}

///Extracts variable audio buss settings into string
std::string extractBuss() {
    if (sgd_debug) fprintf(stderr, "    Extract BUSS info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0); snprintf(out.data() + s1, s0, in, args...);
    };
    
    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.buss.flag).to_string().c_str());
    set_fstr("Busses:\n");
    for (const auto &b : sgd_inf.buss.buss) {
        set_fstr("    Buss: %d\n", &b - sgd_inf.buss.buss.data());
        set_fstr("    Buss Flags: %s\n", std::bitset<32>(b.flag).to_string().c_str());
        set_fstr("    Name: %s\n", (!b.name.empty()) ? b.name.c_str() : "(none)");
        set_fstr("    Module Unit: %d\n", b.munit);
        set_fstr("    Units:\n");
        for (const auto &u : b.unit) {
            set_fstr("        Unit %d\n", &u - b.unit.data());
            set_fstr("            Unit Flags: %s\n", std::bitset<32>(u.flag).to_string().c_str());
            set_fstr("            Name: %s\n", (!u.name.empty()) ? u.name.c_str() : "(none)");
        }
        set_fstr("    Effects:\n");
        for (const auto &e : b.effect) {
            set_fstr("        Effect %d\n", &e - b.effect.data());
            set_fstr("            Effect Flags: %s\n", std::bitset<32>(e.flag).to_string().c_str());
            set_fstr("            Name: %s\n", (!e.name.empty()) ? e.name.c_str() : "(none)");
            set_fstr("            Module: %s\n", (!e.module.empty()) ? e.module.c_str() : "(none)");
            set_fstr("            Assigned:");
            for (const auto &a : e.assign) set_fstr(" 0x%02X", a); out += "\n";
            set_fstr("            Bus In:");
            for (int i = 0; i < e.numin; ++i) set_fstr(" 0x%02X", e.busin[i]); out += "\n";
            set_fstr("            Bus Out:");
            for (int i = 0; i < e.numout; ++i) set_fstr(" 0x%02X", e.busout[i]); out += "\n";
            set_fstr("            Gain In:");
            for (int i = 0; i < e.numin; ++i) set_fstr(" 0x%02X", e.gainin[i]); out += "\n";
            set_fstr("            Gain Out:");
            for (int i = 0; i < e.numout; ++i) set_fstr(" 0x%02X", e.gainout[i]); out += "\n";
            set_fstr("            Size: %d\n", e.preset.size());
        }
        set_fstr("    Operation: %s\n", (!b.oper.empty()) ? b.oper.c_str() : "(none)");
        set_fstr("    Operation Parameters: %s\n", (!b.oparam.empty()) ? b.oparam.c_str() : "(none)");
    }
    
    return out;
}