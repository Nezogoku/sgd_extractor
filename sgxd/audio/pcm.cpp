#include <cmath>
#include <climits>
#include <vector>
#include "audio_func.hpp"


///Decodes Linear PCM
std::vector<short> decodePcm(unsigned char *in, const unsigned length, const unsigned short align,
                             const unsigned short chns, const unsigned short bits, const unsigned short bytes,
                             const bool is_be, const bool is_signed, const bool is_lalign, const bool is_interl) {
    if (!in || !length || !align    ||
        !chns || !bits || !bytes    ||
        align < bytes  || bytes > 4 ||
        align > length || bits > 32 ||
        length % align) return {};

    const unsigned char *in_end = in + length;
    const unsigned long long PCM_VAL = 1LL << bits;
    const unsigned long long PCM_VAL_2 = PCM_VAL >> 1;
    const signed long long PCM_MIN = (!is_signed) ? 0 : -PCM_VAL_2;
    const unsigned long long PCM_MAX = ((!is_signed) ? PCM_VAL : PCM_VAL_2) - 1;
    std::vector<short> out(length / bytes);

    auto get_smp = [&in, &is_be](int length) -> unsigned {
        unsigned out = 0;
        switch (is_be) {
            case true:
                while (length--) out = (out << 8) | *(in++);
                break;
            case false:
                for (int i = 0; i < length; ++i) out |= (unsigned)*(in++) << (8 * i);
                break;
        }
        return out;
    };

    float hist[chns] {};
    for (int sm_i = 0, ch_i = 0; in < in_end; ++sm_i) {
        int tsmp = get_smp(bytes);

        if (is_lalign) tsmp >>= ((bytes * 8) - bits);
        if (is_signed && (tsmp & (PCM_VAL - 1))) tsmp |= (-1LL >> bits) << bits;
        if (!is_signed) tsmp -= PCM_VAL_2;
        if (bits != 16) {
            float state, tsmp1;
            state = (rand() / (float)RAND_MAX) - 0.5;
            tsmp1 = (tsmp / (float)PCM_VAL_2) * 32767.5;
            if (bits > 16) tsmp1 += state - hist[ch_i % chns];
            tsmp = std::floor(tsmp1);

            hist[ch_i % chns] = state;
        }
        tsmp = std::min(SHRT_MAX, std::max(tsmp, SHRT_MIN));

        if (!is_interl) {
            ch_i = sm_i / align;
            out[((sm_i % align) * chns) + (ch_i % chns)] = tsmp;
        }
        else { out[sm_i] = tsmp; ch_i += 1; }
    }

    return out;
}
