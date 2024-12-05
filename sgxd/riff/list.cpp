#include "fourcc_type.hpp"
#include "chunk_type.hpp"
#include "riff_func.hpp"


///Unpacks subchunk from LIST data
void unpackList(unsigned char *in, const unsigned length,
                const EndianType endian, const bool is_rv) {
    riff_inf.list = {};
    if (!in || length < 4) return;

    const unsigned char *in_end = in + length;

    auto get_fcc = [&in]() -> unsigned {
        unsigned out = 0;
        for (int t = 0; t < 4; ++t) out = (out << 8) | *(in++);
        return out;
    };

    riff_inf.list.setRev(is_rv);
    riff_inf.list.setEnd(endian);
    riff_inf.list.setFcc(get_fcc());
    riff_inf.list.setArr(in, length - 4);
}

///Unpacks subchunk from LIST chunk
void unpackList(const chunk chnk) {
    unpackList(chnk.getArr().data(), chnk.size() - 8, chnk.getEnd(), chnk.getRev());
}
