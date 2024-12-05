#include "chunk_type.hpp"
#include "uuid_type.hpp"
#include "riffwave_const.hpp"
#include "riffwave_types.hpp"
#include "riffwave_func.hpp"


///Unpacks format info from FMT data
void unpackFmt(unsigned char *in, const unsigned length,
               const EndianType endian, const bool is_rv) {
    wav_inf.fmt = {};
    if (!in || length < 14) return;

    const unsigned char *in_end = in + length;
    unsigned t_sz;

    auto get_int = [&in, &endian](unsigned length) -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < length; ++i) {
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
    auto get_uid = [&get_int, &in]() -> uuid {
        uuid out;
        out.g0 = get_int(4);
        out.g1 = get_int(2);
        out.g2 = get_int(2);
        for (auto &g : out.g3) g = *(in++);
        return out;
    };

    wav_inf.fmt.codec = get_int(2);
    wav_inf.fmt.chns = get_int(2);
    wav_inf.fmt.smprate = get_int(4);
    wav_inf.fmt.bytrate = get_int(4);
    wav_inf.fmt.align = get_int(2);
    if (length >= 16) wav_inf.fmt.bitrate = get_int(2);
    if (length >= 18) t_sz = get_int(2);
    else t_sz = 0;
    if (t_sz) {
        switch (wav_inf.fmt.codec) {
            case CODEC_PCM:
            case CODEC_IEEE_FLOAT:
            case CODEC_DVI_ADPCM:
            case CODEC_SIERRA_ADPCM:
            case CODEC_G723_ADPCM:
            case CODEC_SONARC:
            case CODEC_DOLBY_AC2:
            case CODEC_CRES_VQLPC:
            case CODEC_G721_ADPCM:
            case CODEC_CREATIVE_ADPCM:
            case CODEC_CREATIVE_FSPCH8:
            case CODEC_CREATIVE_FSPCH10:
            case CODEC_EXTENSIBLE:
                if (t_sz >= 2 ) wav_inf.fmt.smpinfo = get_int(2);
                if (t_sz >= 6 ) wav_inf.fmt.chnmask = get_int(4);
                if (t_sz >= 24) wav_inf.fmt.guid = get_uid();
            default:
                while (in < in_end) wav_inf.fmt.extra.push_back(*(in++));
        }
    }
}

///Unpacks format info from FMT chunk
void unpackFmt(const chunk chnk) {
    unpackFmt(chnk.getArr().data(), chnk.size() - 8, chnk.getEnd(), chnk.getRev());
}
