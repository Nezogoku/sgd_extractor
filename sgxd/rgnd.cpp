#include <algorithm>
#include <bitset>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>
#include "riff/riff_forms.hpp"
#include "riff/riffsfbk_forms.hpp"
#include "riff/riffsfbk_const.hpp"
#include "riff/riffsfbk_types.hpp"
#include "riff/riffsfbk_func.hpp"
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"


///Unpacks variable region definitions from RGND data
void unpackRgnd(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack RGND\n");
    
    sgd_inf.rgnd = {};
    if (!sgd_beg || !in || length < 8) return;

    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.rgnd;

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

    if (sgd_debug) fprintf(stderr, "    Read RGND Header\n");
    out.flag = get_int(4);
    out.rgnd.resize(get_int(4));
    signed rgnoffs[out.rgnd.size()] {};

    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);

    if (sgd_debug) fprintf(stderr, "    Read RGND Setup\n");
    for (int r = 0; r < out.rgnd.size(); ++r) {
        out.rgnd[r].resize(get_int(4));
        rgnoffs[r] = get_int(4);
    }

    if (sgd_debug) fprintf(stderr, "    Read RGND Region\n");
    for (int r = 0; r < out.rgnd.size(); ++r) {
        in = (unsigned char*)sgd_beg + rgnoffs[r];

        if (sgd_debug) fprintf(stderr, "        Current region: %d\n", r);
        for (auto &t : out.rgnd[r]) {
            t.flag = get_int(4);
            t.name = get_str(sgd_beg, get_int(4));
            t.rgnsiz = get_int(4);
            if (t.rgnsiz < 56) { in += 56 - t.rgnsiz - 12; continue; }
            else if (t.rgnsiz > 56) { in += t.rgnsiz - 56 - 12; continue; }
            t.voice = *(in++);
            t.excl = *(in++);
            t.bnkmode = *(in++);
            t.bnkid = *(in++);
            in += 4;
            t.effect = get_int(4);
            t.notelow = *(in++);
            t.notehigh = *(in++);
            in += 2;
            t.noteroot = *(in++);
            t.notetune = *(in++);
            t.notepitch = get_int(2);
            t.vol0 = get_int(2);
            t.vol1 = get_int(2);
            t.gendry = get_int(2);
            t.genwet = get_int(2);
            t.env0 = get_int(4);
            t.env1 = get_int(4);
            t.vol = *(in++);
            t.pan = *(in++);
            t.bendlow = *(in++);
            t.bendhigh = *(in++);
            t.smpid = get_int(4);

            if (sgd_debug) {
                fprintf(stderr, "            Current tone: %d\n", &t - out.rgnd[r].data());
                fprintf(stderr, "                Local flag: 0x%08X\n", t.flag);
                fprintf(stderr, "                Name: %s\n", t.name.c_str());
                fprintf(stderr, "                Region size: %d\n", t.rgnsiz);
                fprintf(stderr, "                Priority: %d\n", t.voice);
                fprintf(stderr, "                Group ID 1: %d\n", t.excl);
                fprintf(stderr, "                Group mode: %d\n", t.bnkmode);
                fprintf(stderr, "                Group ID 2: %d\n", t.bnkid);
                fprintf(stderr, "                Effect: %d\n", t.effect);
                fprintf(stderr, "                Note low: %d\n", t.notelow);
                fprintf(stderr, "                Note high: %d\n", t.notehigh);
                fprintf(stderr, "                Note root: %d\n", t.noteroot);
                fprintf(stderr, "                Note tune: %d\n", t.notetune);
                fprintf(stderr, "                Note pitch: %d\n", t.notepitch);
                fprintf(stderr, "                Volume 1: %d\n", t.vol0);
                fprintf(stderr, "                Volume 2: %d\n", t.vol1);
                fprintf(stderr, "                Generator dry: %d\n", t.gendry);
                fprintf(stderr, "                Generator wet: %d\n", t.genwet);
                fprintf(stderr, "                Envelope 1: %d\n", t.env0);
                fprintf(stderr, "                Envelope 2: %d\n", t.env1);
                fprintf(stderr, "                Volume: %d\n", t.vol);
                fprintf(stderr, "                Pan: %d\n", t.pan);
                fprintf(stderr, "                Bend low: %d\n", t.bendlow);
                fprintf(stderr, "                Bend high: %d\n", t.bendhigh);
                fprintf(stderr, "                Sample ID: %d\n", t.smpid);
            }
        }
    }
}

///Packs variable region definitions into soundfont data
std::vector<unsigned char> rgndToSfbk() {
    if (sgd_debug) fprintf(stderr, "    Extract SF2\n");
    
    sf2_inf = {};
    if (
        sgd_inf.file.empty() ||
        sgd_inf.rgnd.empty() ||
        sgd_inf.wave.empty()
    ) return {};

    auto get_crd = []() -> std::string {
        time_t t_tm;
        struct tm *t_dt;
        std::string out;
        out.resize(20);

        //Get current time in "Month Day, Year" format
        std::time(&t_tm);
        t_dt = std::localtime(&t_tm);
        std::strftime(out.data(), 20, "%B %e, %Y", t_dt);

        return out;
    };
    auto set_nam = []<typename... T>(char *out, const int length, const char *in, T&&... args) -> void {
        snprintf(out, length + 1, in, args...);
    };
    std::string sft = "";
    sft += PROGRAMME_IDENTIFIER;
    sft += ":";
    sft += PROGRAMME_IDENTIFIER;

    if (sgd_debug) fprintf(stderr, "        Set info to soundbank\n");
    //Version Level
    if (sgd_debug) fprintf(stderr, "            Set version\n");
    sf2_inf.info.emplace_back();
    sf2_inf.info.back().setFcc(INFO_ifil);
    sf2_inf.info.back().setInt(0x02, 2);
    sf2_inf.info.back().setInt(0x04, 2);
    //Sound Engine
    if (sgd_debug) fprintf(stderr, "            Set sound engine\n");
    sf2_inf.info.emplace_back();
    sf2_inf.info.back().setFcc(INFO_isng);
    sf2_inf.info.back().setZtr("EMU8000");
    //Title
    if (sgd_debug) fprintf(stderr, "            Set title\n");
    sf2_inf.info.emplace_back();
    sf2_inf.info.back().setFcc(INFO_INAM);
    sf2_inf.info.back().setZtr(sgd_inf.file);
    //ROM Name
    if (sgd_debug) fprintf(stderr, "            Set ROM name\n");
    sf2_inf.info.emplace_back();
    sf2_inf.info.back().setFcc(INFO_irom);
    sf2_inf.info.back().setZtr("1MGM");
    //ROM Version Level
    if (sgd_debug) fprintf(stderr, "            Set ROM version\n");
    sf2_inf.info.emplace_back();
    sf2_inf.info.back().setFcc(INFO_iver);
    sf2_inf.info.back().setInt(0x06, 2);
    sf2_inf.info.back().setInt(0x00, 2);
    //Creation Date
    if (sgd_debug) fprintf(stderr, "            Set creation date\n");
    sf2_inf.info.emplace_back();
    sf2_inf.info.back().setFcc(INFO_ICRD);
    sf2_inf.info.back().setZtr(get_crd());
    //Software Package
    if (sgd_debug) fprintf(stderr, "            Set software\n");
    sf2_inf.info.emplace_back();
    sf2_inf.info.back().setFcc(INFO_ISFT);
    sf2_inf.info.back().setZtr(sft);

    if (sgd_debug) fprintf(stderr, "        Set samples to soundbank\n");
    const int siz = sgd_inf.wave.wave.size();
    for (int w = 0; w <= siz; ++w) {
        const auto &wav = sgd_inf.wave.wave[w % siz];
        if (w != siz && wav.chns != 1) continue;

        auto &smp = sf2_inf.sdta.smpl;
        char nam[SFBK_NAME_MAX + 1] {};

        if (sgd_debug) fprintf(stderr, "            Set sample %d header\n", w);
        if (w == siz) set_nam(nam, SFBK_NAME_MAX, "triangle");
        else if (!wav.name.empty()) set_nam(nam, SFBK_NAME_MAX, wav.name.c_str());
        else set_nam(nam, SFBK_NAME_MAX, "smpl_%03d", w);
        sf2_inf.pdta.shdr.emplace_back(
            nam,
            (w == siz) ? 428157 : smp.size(),
            (w == siz) ? 430225 : smp.size() + wav.pcm.size(),
            (w == siz) ? 428493 : (smp.size() + (wav.loopbeg < 0) ? wav.loopsmp : wav.loopbeg),
            (w == siz) ? 430221 : (smp.size() + (wav.loopend < 0) ? wav.loopsmp : wav.loopend),
            (w == siz) ? 44100  : wav.smprate,
            60, 0, 0, (w != siz) ? ST_RAM_MONO : ST_ROM_MONO
        );

        if (sgd_debug) fprintf(stderr, "            Set sample %d\n", w);
        if (w != siz) smp.insert(smp.end(), wav.pcm.begin(), wav.pcm.end());
        smp.resize(smp.size() + SFBK_SMPL_PAD, 0);
    }

    if (sgd_debug) fprintf(stderr, "        Set instruments to soundbank\n");
    for (int r = 0, i = 0; r < sgd_inf.rgnd.rgnd.size(); ++r) {
        const auto &rgn = sgd_inf.rgnd.rgnd[r];
        if (rgn.empty()) continue;

        for (int t = 0; t < rgn.size(); ++t) {
            const auto &ton = sgd_inf.rgnd.rgnd[r][t];
            const auto &wav = sgd_inf.wave.wave;

            char nam[SFBK_NAME_MAX + 1] {};

            if (sgd_debug) fprintf(stderr, "            Set instrument %d header\n", i + t);
            if (!ton.name.empty()) set_nam(nam, SFBK_NAME_MAX, ton.name.c_str());
            else set_nam(nam, SFBK_NAME_MAX, "inst_%03d", i + t);
            sf2_inf.pdta.inst.emplace_back(nam, sf2_inf.pdta.ibag.size());

            if (sgd_debug) fprintf(stderr, "            Set instrument %d zone\n", i + t);
            sf2_inf.pdta.ibag.emplace_back(
                sf2_inf.pdta.igen.size(), sf2_inf.pdta.imod.size()
            );

            if (sgd_debug) fprintf(stderr, "            Set instrument %d generators\n", i + t);
            //Key range
            sf2_inf.pdta.igen.emplace_back(
                GN_KEY_RANGE, ton.notelow, ton.notehigh
            );
            //Pitch
            sf2_inf.pdta.igen.emplace_back(
                GN_MODULATION_ENV_FINE_PITCH, ton.notepitch
            );
            //Chorus if applicable
            if (ton.effect) sf2_inf.pdta.igen.emplace_back(
                GN_CHORUS_EFFECTS_SEND, ton.effect
            );
            //Reverb
            sf2_inf.pdta.igen.emplace_back(
                GN_REVERB_EFFECTS_SEND, (ton.genwet - ton.gendry) * 4096 / 1000.00
            );
            //Dry pan
            sf2_inf.pdta.igen.emplace_back(
                GN_DRY_PAN, ton.vol1 * 500 / 1024.00
            );
            //Envelope 1
            sf2_inf.pdta.igen.emplace_back(
                GN_VOLUME_ENV_ATTACK, ton.env0
            );
            //Envelope 2
            sf2_inf.pdta.igen.emplace_back(
                GN_VOLUME_ENV_HOLD, ton.env1
            );
            //Initial attenuation if applicable
            if (ton.vol0 != 4096) sf2_inf.pdta.igen.emplace_back(
                GN_INITIAL_ATTENUATION, (4096 - ton.vol0) * 1440 / 4096.00
            );
            //Coarse tune if applicable
            if (ton.noteroot < 0) sf2_inf.pdta.igen.emplace_back(
                GN_PITCH_COARSE_TUNE, ton.noteroot * -1
            );
            //Fine tune if applicable
            if (ton.notetune) sf2_inf.pdta.igen.emplace_back(
                GN_PITCH_FINE_TUNE, ton.notetune
            );
            //Sample mode
            sf2_inf.pdta.igen.emplace_back(
                GN_SAMPLE_MODE,
                (ton.smpid >= 0 && ton.smpid < siz) ? (
                    (wav[ton.smpid].loopbeg < 0 && wav[ton.smpid].loopbeg < 0) ? SM_NO_LOOP :
                    (wav[ton.smpid].loopbeg < 0) ? SM_DEPRESSION_LOOP : SM_CONTINUOUS_LOOP
                ) : SM_DEPRESSION_LOOP
            );
            //Exclusive class if applicable
            if (ton.excl) sf2_inf.pdta.igen.emplace_back(
                GN_SAMPLE_EXCLUSIVE_CLASS, ton.excl
            );
            //Root key
            sf2_inf.pdta.igen.emplace_back(
                GN_SAMPLE_OVERRIDE_ROOT,
                (ton.noteroot < 0) ? 127 : ton.noteroot
            );
            //Sample ID
            sf2_inf.pdta.igen.emplace_back(
                GN_SAMPLE_ID,
                (ton.smpid < 0) ? siz - 1 : ton.smpid
            );

            if (sgd_debug) fprintf(stderr, "            Set instrument %d modulators\n", i + t);
        }

        i += rgn.size();
    }

    if (sgd_debug) fprintf(stderr, "        Set presets to soundbank\n");
    for (int r = 0, i = 0; r < sgd_inf.rgnd.rgnd.size(); ++r) {
        const auto &rgn = sgd_inf.rgnd.rgnd[r];
        std::vector<unsigned char> bnk;
        if (rgn.empty()) continue;

        for (const auto &ton : rgn) {
            if (std::find(bnk.begin(), bnk.end(), ton.bnkid) != bnk.end()) continue;
            else bnk.push_back(ton.bnkid);
        }
        std::sort(bnk.begin(), bnk.end());

        for (int b = 0; b < bnk.size(); ++b) {
            char nam[SFBK_NAME_MAX + 1] {};

            if (sgd_debug) fprintf(stderr, "            Set bank %d preset %d header\n", b, r);
            set_nam(nam, SFBK_NAME_MAX, "prst_%03d_%04d", b, r);
            sf2_inf.pdta.phdr.emplace_back(nam, r, b, sf2_inf.pdta.pbag.size());

            if (sgd_debug) fprintf(stderr, "            Set bank %d preset %d zone\n", b, r);
            sf2_inf.pdta.pbag.emplace_back(
                sf2_inf.pdta.pgen.size(), sf2_inf.pdta.pmod.size()
            );

            if (sgd_debug) fprintf(stderr, "            Set bank %d preset %d generators\n", b, r);
            for (int t = 0; t < rgn.size(); ++t) {
                if (rgn[t].bnkid != b) continue;

                //Instrument ID
                sf2_inf.pdta.igen.emplace_back(GN_INSTRUMENT_ID, i + t);
            }

            if (sgd_debug) fprintf(stderr, "            Set bank %d preset %d modulators\n", b, r);
        }

        i += rgn.size();
    }

    return packRiffSfbk();
}

///Extracts variable region definitions into string
std::string extractRgnd() {
    if (sgd_debug) fprintf(stderr, "    Extract RGND info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0); snprintf(out.data() + s1, s0, in, args...);
    };

    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.rgnd.flag).to_string().c_str());
    set_fstr("Regions:\n");
    for (const auto &r : sgd_inf.rgnd.rgnd) {
        set_fstr("    Region: %d\n", &r - sgd_inf.rgnd.rgnd.data());
        for (const auto &t : r) {
            set_fstr("        Tone: %d\n", &t - r.data());
            set_fstr("            Tone Flags: %s\n", std::bitset<32>(t.flag).to_string().c_str());
            set_fstr("            Name: %s\n", (!t.name.empty()) ? t.name.c_str() : "(none)");
            set_fstr("            Region Size: %d\n", t.rgnsiz);
            set_fstr("            Priority: %d\n", t.voice);
            set_fstr("            Bank ID 1: %d\n", t.excl);
            set_fstr("            Bank Mode: %d\n", t.bnkmode);
            set_fstr("            Bank ID 2: %d\n", t.bnkid);
            set_fstr("            Effect: %d\n", t.effect);
            set_fstr("            Note Range %d to %d\n", t.notelow, t.notehigh);
            set_fstr("            Root Note: %d\n", t.noteroot);
            set_fstr("            Note Tune: %d\n", t.notetune);
            set_fstr("            Note Pitch: %d\n", t.notepitch);
            set_fstr("            Volume Range %d to %d\n", t.vol0, t.vol1);
            set_fstr("            Dry Generator: %d\n", t.gendry);
            set_fstr("            Wet Generator: %d\n", t.genwet);
            set_fstr("            Envelope 1: %d\n", t.env0);
            set_fstr("            Envelope 2: %d\n", t.env1);
            set_fstr("            Volume: %d\n", t.vol);
            set_fstr("            Pan: %d\n", t.pan);
            set_fstr("            Bend Range %d to %d\n", t.bendlow, t.bendhigh);
            set_fstr("            Sample ID: %d\n", t.smpid);
        }
    }

    return out;
}
