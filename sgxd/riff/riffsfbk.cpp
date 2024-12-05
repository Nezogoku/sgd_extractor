#include <algorithm>
#include <vector>
#include "fourcc_type.hpp"
#include "chunk_type.hpp"
#include "riff_forms.hpp"
#include "riff_types.hpp"
#include "riff_func.hpp"
#include "riffsfbk_forms.hpp"
#include "riffsfbk_const.hpp"
#include "riffsfbk_types.hpp"
#include "riffsfbk_func.hpp"


///Unpacks SFBK info from SFBK data
void unpackRiffSfbk(unsigned char *in, const unsigned length,
                    const EndianType endian, const bool is_rv) {
    sf2_inf = {};
    if (!in || length < 4) return;

    const unsigned char *in_end = in + length;

    auto get_fcc = [&in]() -> unsigned {
        unsigned out = 0;
        for (int t = 0; t < 4; ++t) out = (out << 8) | *(in++);
        return out;
    };
    auto get_int = [&in, &endian]() -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < 4; ++i) {
            switch (endian) {
                case ENDIAN_BIG:
                    out = (out << 8) | *(in++);
                    continue;
                case ENDIAN_LITTLE:
                    out |= (unsigned)*(in++) << (8 * i);
                    continue;
            }
        }
        return out;
    };

    while (in < in_end) {
        fourcc t_fc;
        unsigned t_sz;
        chunk t_ch;

        t_fc = get_fcc();
        t_sz = get_int();
        if (in + t_sz > in_end) break;

        t_ch.setRev(is_rv);
        t_ch.setEnd(endian);
        t_ch.setFcc(t_fc);
        t_ch.setArr(in, t_sz); in += t_sz;
        if (t_sz % 2 && !in[0]) { t_ch.setPad(); in += 1; }
#ifdef UNPACKLIST_IMPLEMENTATION
        if (t_ch.getFcc() == LIST_INFO) {
            unpackList(t_ch);
            switch(riff_inf.list.getFcc.getInt()) {
#ifdef UNPACKINFO_IMPLEMENTATION
                case LIST_INFO:
                    unpackInfo(riff_inf.list);
                    sf2_inf.info.swap(riff_inf.info);
                    continue;
#endif
#ifdef UNPACKSDTA_IMPLEMENTATION
                case LIST_sdta:
                    unpackSdta(riff_inf.list);
                    continue;
#endif
#ifdef UNPACKPDTA_IMPLEMENTATION
                case LIST_pdta:
                    unpackPdta(riff_inf.list);
                    continue;
#endif
                default:
                    continue;
            }
        }
#endif
    }

    if (!sf2_inf.pdta.empty()) {
        if (!sf2_inf.pdta.phdr.empty()) {
            const auto &phdr = sf2_inf.pdta.phdr.back();
            std::string name(phdr.name, phdr.name + SFBK_NAME_MAX);
            name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
            if (
                phdr == phdrinfo{} ||
                name.empty() || name == "EOP" ||
                phdr.pbagid == sf2_inf.pdta.pbag.size()
            ) sf2_inf.pdta.phdr.pop_back();
        }
        if (!sf2_inf.pdta.pbag.empty()) {
            const auto &pbag = sf2_inf.pdta.pbag.back();
            if (
                pbag == baginfo{} ||
                pbag.genid == sf2_inf.pdta.pgen.size() ||
                pbag.modid == sf2_inf.pdta.pmod.size()
            ) sf2_inf.pdta.pbag.pop_back();
        }
        if (!sf2_inf.pdta.pmod.empty()) {
            const auto &pmod = sf2_inf.pdta.pmod.back();
            if (pmod == modinfo{}) sf2_inf.pdta.pmod.pop_back();
        }
        if (!sf2_inf.pdta.pgen.empty()) {
            const auto &pgen = sf2_inf.pdta.pgen.back();
            if (pgen == geninfo{}) sf2_inf.pdta.pgen.pop_back();
        }
        if (!sf2_inf.pdta.inst.empty()) {
            const auto &inst = sf2_inf.pdta.inst.back();
            std::string name(inst.name, inst.name + SFBK_NAME_MAX);
            name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
            if (
                inst == ihdrinfo{} ||
                name.empty() || name == "EOI" ||
                inst.ibagid == sf2_inf.pdta.ibag.size()
            ) sf2_inf.pdta.inst.pop_back();
        }
        if (!sf2_inf.pdta.ibag.empty()) {
            const auto &ibag = sf2_inf.pdta.ibag.back();
            if (
                ibag == baginfo{} ||
                ibag.genid == sf2_inf.pdta.igen.size() ||
                ibag.modid == sf2_inf.pdta.imod.size()
            ) sf2_inf.pdta.ibag.pop_back();
        }
        if (!sf2_inf.pdta.imod.empty()) {
            const auto &imod = sf2_inf.pdta.imod.back();
            if (imod == modinfo{}) sf2_inf.pdta.imod.pop_back();
        }
        if (!sf2_inf.pdta.igen.empty()) {
            const auto &igen = sf2_inf.pdta.igen.back();
            if (igen == geninfo{}) sf2_inf.pdta.igen.pop_back();
        }
        if (!sf2_inf.pdta.shdr.empty()) {
            const auto &shdr = sf2_inf.pdta.shdr.back();
            std::string name(shdr.name, shdr.name + SFBK_NAME_MAX);
            name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
            if (
                shdr == shdrinfo{} ||
                name.empty() || name == "EOS"
            ) sf2_inf.pdta.shdr.pop_back();
        }
    }
}

///Unpacks SFBK info from SFBK chunk
void unpackRiffSfbk(const chunk chnk) {
    unpackRiffSfbk(chnk.getArr().data(), chnk.size() - 8, chnk.getEnd(), chnk.getRev());
}


///Packs SFBK info into array
std::vector<unsigned char> packRiffSfbk() {
    if (sf2_inf.info.empty()) return {};

    chunk out, sfbk;

    //Initial setup of RIFF SFBK data
    sfbk.setFcc(RIFF_sfbk);

    //Set information chunk
    if (true) {
        auto has_ifil = []() -> bool {
            return std::find_if(
                sf2_inf.info.begin(), sf2_inf.info.end(),
                [](const chunk &c){return c.getFcc()==INFO_ifil;}
            ) != sf2_inf.info.end();
        };
        auto has_isng = []() -> bool {
            return std::find_if(
                sf2_inf.info.begin(), sf2_inf.info.end(),
                [](const chunk &c){return c.getFcc()==INFO_isng;}
            ) != sf2_inf.info.end();
        };
        auto has_inam = []() -> bool {
            return std::find_if(
                sf2_inf.info.begin(), sf2_inf.info.end(),
                [](const chunk &c){return c.getFcc()==INFO_INAM;}
            ) != sf2_inf.info.end();
        };

        chunk list, info;

        //Set information chunk
        info.setFcc(LIST_INFO);
        if (!has_ifil()) {
            chunk ifil;

            ifil.setRev(false);
            ifil.setEnd(ENDIAN_LITTLE);
            ifil.setFcc(INFO_ifil);
            ifil.setInt(0x02, 2);
            ifil.setInt(0x04, 2);

            info += ifil;
        }
        if (!has_isng()) {
            chunk isng;

            isng.setRev(false);
            isng.setEnd(ENDIAN_LITTLE);
            isng.setFcc(INFO_isng);
            isng.setZtr("EMU8000");

            info += isng;
        }
        if (!has_inam()) {
            chunk inam;

            inam.setRev(false);
            inam.setEnd(ENDIAN_LITTLE);
            inam.setFcc(INFO_INAM);
            inam.setZtr("UNKNOWN");

            info += inam;
        }
        for (const auto &i : sf2_inf.info) info += i;

        //Set list chunk
        list.setFcc(RIFF_LIST);
        list.setChk(info, false);

        sfbk += list;
    }

    //Set sample data chunk
    if (true) {
        chunk list, sdta;

        //Set sample data chunk
        sdta.setFcc(LIST_sdta);
        if (!sf2_inf.sdta.smpl.empty()) {
            chunk smpl;
            smpl.setFcc(SDTA_smpl);
            for (const auto &s : sf2_inf.sdta.smpl) smpl.setInt(s, 2);
            sdta += smpl;
        }
        if (!sf2_inf.sdta.sm24.empty()) {
            chunk sm24;
            sm24.setFcc(SDTA_sm24);
            sm24.setArr(sf2_inf.sdta.sm24);
            if (sf2_inf.sdta.sm24.size() % 2) sm24.setPad();
            sdta += sm24;
        }

        //Set list chunk
        list.setFcc(RIFF_LIST);
        list.setChk(sdta, false);

        sfbk += list;
    }

    //Set preset data chunk
    if (true) {
        const auto get_bag = [](const std::vector<baginfo> &bag, const unsigned &fcc) -> chunk {
            chunk out;

            out.setFcc(fcc);
            for (int b = 0; b <= bag.size(); ++b) {
                if (b == bag.size()) out.setPad(SFBK_BAG_SIZE);
                else {
                    out.setInt(bag[b].genid, 2);
                    out.setInt(bag[b].modid, 2);
                }
            }

            return out;
        };
        const auto get_mod = [](const std::vector<modinfo> &mod, const unsigned &fcc) -> chunk {
            chunk out;

            out.setFcc(fcc);
            for (int m = 0; m <= mod.size(); ++m) {
                if (m == mod.size()) out.setPad(SFBK_MOD_SIZE);
                else {
                    out.setInt(mod[m].modsrc, 2);
                    out.setInt(mod[m].moddst, 2);
                    out.setInt(mod[m].modamnt, 2);
                    out.setInt(mod[m].srcamnt, 2);
                    out.setInt(mod[m].srctrns, 2);
                }
            }

            return out;
        };
        const auto get_gen = [](const std::vector<geninfo> &gen, const unsigned &fcc) -> chunk {
            chunk out;

            out.setFcc(fcc);
            for (int g = 0; g <= gen.size(); ++g) {
                if (g == gen.size()) out.setPad(SFBK_GEN_SIZE);
                else {
                    out.setInt(gen[g].type, 2);
                    if (gen[g].type == GN_KEY_RANGE || gen[g].type == GN_VELOCITY_RANGE) {
                        out.setInt((gen[g].val >> 8) & 0xFF, 1);
                        out.setInt((gen[g].val >> 0) & 0xFF, 1);
                    }
                    else out.setInt(gen[g].val, 2);
                }
            }

            return out;
        };

        chunk list, pdta;

        //Set preset data chunk
        pdta.setFcc(LIST_pdta);
        if (true) {
            chunk phdr;
            phdr.setFcc(PDTA_phdr);
            for (int p = 0; p <= sf2_inf.pdta.phdr.size(); ++p) {
                if (p == sf2_inf.pdta.phdr.size()) {
                    phdr.setStr("EOP", SFBK_NAME_MAX);
                    phdr.setPad(SFBK_PHDR_SIZE - SFBK_NAME_MAX);
                }
                else {
                    const auto &phd = sf2_inf.pdta.phdr[p];
                    phdr.setStr(phd.name, SFBK_NAME_MAX);
                    phdr.setInt(phd.prstid, 2);
                    phdr.setInt(phd.bankid, 2);
                    phdr.setInt(phd.pbagid, 2);
                    phdr.setInt(phd.library, 4);
                    phdr.setInt(phd.genre, 4);
                    phdr.setInt(phd.morph, 4);
                }
            }
            pdta += phdr;
        }
        if (true) pdta += get_bag(sf2_inf.pdta.pbag, PDTA_pbag);
        if (true) pdta += get_mod(sf2_inf.pdta.pmod, PDTA_pmod);
        if (true) pdta += get_gen(sf2_inf.pdta.pgen, PDTA_pgen);
        if (true) {
            chunk inst;
            inst.setFcc(PDTA_inst);
            for (int i = 0; i <= sf2_inf.pdta.inst.size(); ++i) {
                if (i == sf2_inf.pdta.inst.size()) {
                    inst.setStr("EOI", SFBK_NAME_MAX);
                    inst.setPad(SFBK_IHDR_SIZE - SFBK_NAME_MAX);
                }
                else {
                    const auto &ihd = sf2_inf.pdta.inst[i];
                    inst.setStr(ihd.name, SFBK_NAME_MAX);
                    inst.setInt(ihd.ibagid, 2);
                }
            }
            pdta += inst;
        }
        if (true) pdta += get_bag(sf2_inf.pdta.ibag, PDTA_ibag);
        if (true) pdta += get_mod(sf2_inf.pdta.imod, PDTA_imod);
        if (true) pdta += get_gen(sf2_inf.pdta.igen, PDTA_igen);
        if (true) {
            chunk shdr;
            shdr.setFcc(PDTA_shdr);
            for (int s = 0; s <= sf2_inf.pdta.shdr.size(); ++s) {
                if (s == sf2_inf.pdta.shdr.size()) {
                    shdr.setStr("EOS", SFBK_NAME_MAX);
                    shdr.setPad(SFBK_SHDR_SIZE - SFBK_NAME_MAX);
                }
                else {
                    const auto &shd = sf2_inf.pdta.shdr[s];
                    shdr.setStr(shd.name, SFBK_NAME_MAX);
                    shdr.setInt(shd.smpbeg, 4);
                    shdr.setInt(shd.smpend, 4);
                    shdr.setInt(shd.loopbeg, 4);
                    shdr.setInt(shd.loopend, 4);
                    shdr.setInt(shd.smprate, 4);
                    shdr.setInt(shd.noteroot, 1);
                    shdr.setInt(shd.notetune, 1);
                    shdr.setInt(shd.smplink, 2);
                    shdr.setInt(shd.smptyp, 2);
                }
            }
            pdta += shdr;
        }

        //Set list chunk
        list.setFcc(RIFF_LIST);
        list.setChk(pdta, false);

        sfbk += list;
    }

    //Set RIFF data
    out.setFcc(FOURCC_RIFF);
    out.setChk(sfbk, false);

    return out.getAll();
}
