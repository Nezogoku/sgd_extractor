#ifndef RIFFSFBK_TYPES_HPP
#define RIFFSFBK_TYPES_HPP

#include <compare>
#include <vector>
#include "chunk_type.hpp"
#include "riffsfbk_const.hpp"

///Modulator Fields
struct modinfo {
    ~modinfo() = default;
    modinfo(
        const unsigned short ms = 0, const unsigned short md = 0,
        const short ma = 0,
        const unsigned short sa = 0, const unsigned short st = 0) :
        modsrc(ms), moddst(md), modamnt(ma), srcamnt(sa), srctrns(st) {}
    modinfo(const modinfo &mod) = default;
    modinfo(modinfo &&mod) = default;

    modinfo& operator=(const modinfo &mod) = default;
    modinfo& operator=(modinfo &&mod) = default;
    
    auto operator<=>(const modinfo &mod) const {
        if (auto cmp = modsrc <=> mod.modsrc; cmp != 0) return cmp;
        if (auto cmp = moddst <=> mod.moddst; cmp != 0) return cmp;
        if (auto cmp = modamnt <=> mod.modamnt; cmp != 0) return cmp;
        if (auto cmp = srcamnt <=> mod.srcamnt; cmp != 0) return cmp;
        if (auto cmp = srctrns <=> mod.srctrns; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator<(const modinfo &mod) const = default;
    bool operator>(const modinfo &mod) const = default;
    bool operator==(const modinfo &mod) const = default;
    bool operator<=(const modinfo &mod) const = default;
    bool operator!=(const modinfo &mod) const = default;
    bool operator>=(const modinfo &mod) const = default;

    unsigned short modsrc;  // SfModulatorSource
    unsigned short moddst;  // SfGenerator
    short modamnt;
    unsigned short srcamnt; // SfModulatorSource
    unsigned short srctrns; // SfModulatorTransform
};

///Generator Fields
struct geninfo {
    ~geninfo() = default;
    geninfo(const unsigned short ty = 0, const unsigned v0 = -1, const unsigned v1 = -1) :
        type(ty), val(0) {
            if ((int)v0 < 0 && (int)v1 < 0);
            else if ((int)v0 < 0 && (int)v1 >= 0) val = v1 & 0xFFFF;
            else if ((int)v0 >= 0 && (int)v1 < 0) val = v0 & 0xFFFF;
            else val = (v0 & 0xFF) << 8 | (v1 & 0xFF);
        }
    geninfo(const geninfo &gen) = default;
    geninfo(geninfo &&gen) = default;

    geninfo& operator=(const geninfo &gen) = default;
    geninfo& operator=(geninfo &&gen) = default;
    
    auto operator<=>(const geninfo &gen) const {
        if (auto cmp = type <=> gen.type; cmp != 0) return cmp;
        if (auto cmp = val <=> gen.val; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator<(const geninfo &gen) const = default;
    bool operator>(const geninfo &gen) const = default;
    bool operator==(const geninfo &gen) const = default;
    bool operator<=(const geninfo &gen) const = default;
    bool operator!=(const geninfo &gen) const = default;
    bool operator>=(const geninfo &gen) const = default;

    unsigned short type;
    unsigned short val;
};

///Zone Fields
struct baginfo {
    ~baginfo() = default;
    baginfo(const unsigned short gn = 0, const unsigned short md = 0) :
        genid(gn), modid(md) {}
    baginfo(const baginfo &bag) = default;
    baginfo(baginfo &&bag) = default;

    baginfo& operator=(const baginfo &bag) = default;
    baginfo& operator=(baginfo &&bag) = default;

    unsigned short genid;
    unsigned short modid;

    auto operator<=>(const baginfo &bag) const {
        if (auto cmp = genid <=> bag.genid; cmp != 0) return cmp;
        if (auto cmp = modid <=> bag.modid; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator<(const baginfo &bag) const = default;
    bool operator>(const baginfo &bag) const = default;
    bool operator==(const baginfo &bag) const = default;
    bool operator<=(const baginfo &bag) const = default;
    bool operator!=(const baginfo &bag) const = default;
    bool operator>=(const baginfo &bag) const = default;
};

///Preset Header Fields
struct phdrinfo {
    ~phdrinfo() = default;
    phdrinfo(
        const char *nm = 0, const unsigned short pi = 0, const unsigned short bi = 0,
        const unsigned short pb = 0, const unsigned lb = 0, const unsigned gn = 0,
        const unsigned mp = 0) :
        prstid(pi), bankid(bi), pbagid(pb),
        library(lb), genre(gn), morph(mp) {
            for (auto &n : name) { if (!nm || !nm[0]) break; n = *(nm++); }
        }
    phdrinfo(const phdrinfo &phd) = default;
    phdrinfo(phdrinfo &&phd) = default;

    phdrinfo& operator=(const phdrinfo &phd) = default;
    phdrinfo& operator=(phdrinfo &&phd) = default;

    auto operator<=>(const phdrinfo &phd) const {
        if (auto cmp = pbagid <=> phd.pbagid; cmp != 0) return cmp;
        if (auto cmp = bankid <=> phd.bankid; cmp != 0) return cmp;
        if (auto cmp = prstid <=> phd.prstid; cmp != 0) return cmp;
        if (auto cmp = library <=> phd.library; cmp != 0) return cmp;
        if (auto cmp = genre <=> phd.genre; cmp != 0) return cmp;
        if (auto cmp = morph <=> phd.morph; cmp != 0) return cmp;
        for (int n = 0; n < SFBK_NAME_MAX; ++n) {
            if (auto cmp = name[n] <=> phd.name[n]; cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }
    bool operator<(const phdrinfo &phd) const { return (pbagid <=> phd.pbagid) < 0; }
    bool operator==(const phdrinfo &phd) const = default;

    char name[SFBK_NAME_MAX] {};
    unsigned short prstid;
    unsigned short bankid;
    unsigned short pbagid;
    unsigned library;
    unsigned genre;
    unsigned morph;
};

///Instrument Header Fields
struct ihdrinfo {
    ~ihdrinfo() = default;
    ihdrinfo(const char *nm = 0, const unsigned short ib = 0) :
        ibagid(ib) { for (auto &n : name) { if (!nm || !nm[0]) break; n = *(nm++); } }
    ihdrinfo(const ihdrinfo &ihd) = default;
    ihdrinfo(ihdrinfo &&ihd) = default;

    ihdrinfo& operator=(const ihdrinfo &ihd) = default;
    ihdrinfo& operator=(ihdrinfo &&ihd) = default;

    auto operator<=>(const ihdrinfo &ihd) const {
        if (auto cmp = ibagid <=> ihd.ibagid; cmp != 0) return cmp;
        for (int n = 0; n < SFBK_NAME_MAX; ++n) {
            if (auto cmp = name[n] <=> ihd.name[n]; cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }
    bool operator<(const ihdrinfo &ihd) const { return (ibagid <=> ihd.ibagid) < 0; }
    bool operator==(const ihdrinfo &ihd) const = default;

    char name[SFBK_NAME_MAX] {};
    unsigned short ibagid;
};

///Sample Header Fields
struct shdrinfo {
    ~shdrinfo() = default;
    shdrinfo(
        const char *nm = 0, const unsigned sb = 0, const unsigned se = 0,
        const unsigned lb = 0, const unsigned le = 0, const unsigned sr = 0,
        const char nr = 0, const char nt = 0, const unsigned short sl = 0,
        const unsigned short st = 0) :
        smpbeg(sb), smpend(se), loopbeg(lb),
        loopend(le), smprate(sr), noteroot(nr),
        notetune(nt), smplink(sl), smptyp(st) {
            for (auto &n : name) { if (!nm || !nm[0]) break; n = *(nm++); }
        }
    shdrinfo(const shdrinfo &shd) = default;
    shdrinfo(shdrinfo &&shd) = default;

    shdrinfo& operator=(const shdrinfo &shd) = default;
    shdrinfo& operator=(shdrinfo &&shd) = default;

    auto operator<=>(const shdrinfo &shd) const {
        if (auto cmp = smpbeg <=> shd.smpbeg; cmp != 0) return cmp;
        if (auto cmp = smpend <=> shd.smpend; cmp != 0) return cmp;
        if (auto cmp = loopbeg <=> shd.loopbeg; cmp != 0) return cmp;
        if (auto cmp = loopend <=> shd.loopend; cmp != 0) return cmp;
        if (auto cmp = smprate <=> shd.smprate; cmp != 0) return cmp;
        if (auto cmp = noteroot <=> shd.noteroot; cmp != 0) return cmp;
        if (auto cmp = notetune <=> shd.notetune; cmp != 0) return cmp;
        if (auto cmp = smplink <=> shd.smplink; cmp != 0) return cmp;
        if (auto cmp = smptyp <=> shd.smptyp; cmp != 0) return cmp;
        for (int n = 0; n < SFBK_NAME_MAX; ++n) {
            if (auto cmp = name[n] <=> shd.name[n]; cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }
    bool operator<(const shdrinfo &shd) const = default;
    bool operator>(const shdrinfo &shd) const = default;
    bool operator==(const shdrinfo &shd) const = default;
    bool operator<=(const shdrinfo &shd) const = default;
    bool operator!=(const shdrinfo &shd) const = default;
    bool operator>=(const shdrinfo &shd) const = default;

    char name[SFBK_NAME_MAX] {};
    unsigned smpbeg;
    unsigned smpend;
    unsigned loopbeg;
    unsigned loopend;
    unsigned smprate;
    unsigned char noteroot;
    char notetune;
    unsigned short smplink;
    unsigned short smptyp;
};

///Sample Data Fields
struct sdtainfo {
    std::vector<short> smpl;
    std::vector<unsigned char> sm24;
    
    bool empty() const { return smpl.empty() && sm24.empty(); }
};

///Preset Data Fields
struct pdtainfo {
    std::vector<phdrinfo>   phdr;
    std::vector<baginfo>    pbag;
    std::vector<modinfo>    pmod;
    std::vector<geninfo>    pgen;
    std::vector<ihdrinfo>   inst;
    std::vector<baginfo>    ibag;
    std::vector<modinfo>    imod;
    std::vector<geninfo>    igen;
    std::vector<shdrinfo>   shdr;
    
    bool empty() const {
        return phdr.empty() &&
               pbag.empty() &&
               pmod.empty() &&
               pgen.empty() &&
               inst.empty() &&
               ibag.empty() &&
               imod.empty() &&
               igen.empty() &&
               shdr.empty();
    }
};

///Soundfont Fields
struct riffsfbk {
    std::vector<chunk>      info;
    sdtainfo                sdta;
    pdtainfo                pdta;
    
    bool empty() const { return info.empty() && sdta.empty() && pdta.empty(); }
};


#endif
