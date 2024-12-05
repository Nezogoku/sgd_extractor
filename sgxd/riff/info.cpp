#include <vector>
#include "fourcc_type.hpp"
#include "chunk_type.hpp"
#include "riff_func.hpp"


///Unpacks variable chunks from INFO data
void unpackInfo(unsigned char *in, const unsigned length,
                const EndianType endian, const bool is_rv) {
    riff_inf.info.clear();
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

        riff_inf.info.push_back(t_ch);
    }
}

///Unpacks variable chunks from INFO chunk
void unpackInfo(const chunk chnk) {
    unpackInfo(chnk.getArr().data(), chnk.size() - 8, chnk.getEnd(), chnk.getRev());
}
