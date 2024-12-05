#include "chunk_type.hpp"
#include "riffwave_types.hpp"
#include "riffwave_func.hpp"


///Unpacks file-dependent info from FACT data
void unpackFact(unsigned char *in, const unsigned length,
                const EndianType endian, const bool is_rv) {
    wav_inf.fact = {};
    if (!in || length < 4 || length % 4) return;

    const unsigned char *in_end = in + length;

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

    wav_inf.fact.smpsize = get_int();
    while (in < in_end) wav_inf.fact.smpinfo.push_back(get_int());
}

///Unpacks file-dependent info from FACT chunk
void unpackFact(const chunk chnk) {
    unpackFact(chnk.getArr().data(), chnk.size() - 8, chnk.getEnd(), chnk.getRev());
}