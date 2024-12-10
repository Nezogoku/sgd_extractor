#include <cmath>
#include <climits>
#include <vector>
#include "audio_func.hpp"


///Decodes Sony ADPCM
std::vector<short> decodeSonyAdpcm(unsigned char *in, const unsigned length,
                                   const unsigned short chns,
                                   signed *loop_b, signed *loop_e) {
    if (!in || length < 16) return {};

    const unsigned char *in_end = in + length;
    const unsigned short VAG_BLOCK_ALIGN = 16;
    const unsigned short VAG_BLOCK_SAMPLES = (VAG_BLOCK_ALIGN - 2) * 2;
    const double VAG_LOOKUP_TABLE[][2] = {
        {0.0, 0.0},
        {60.0 / 64.0, 0.0},
        {115.0 / 64.0, -52.0 / 64.0},
        {98.0 / 64.0, -55.0 / 64.0},
        {122.0 / 64.0, -60.0 / 64.0},
    };
    std::vector<short> out;

    double hist[chns][2] {};
    for (int ba_i = 0; ba_i < length / (VAG_BLOCK_ALIGN * chns); ++ba_i) {
        int data[VAG_BLOCK_SAMPLES * chns] {};

        for (int ch_i = 0; ch_i < chns; ++ch_i) {
            unsigned char coef = *(in++);
            unsigned char flag = *(in++);

            for (int n = 0; n < VAG_BLOCK_SAMPLES; ++in) {
                data[(chns * n++) + ch_i] = in[0] & 0x0F;
                data[(chns * n++) + ch_i] = in[0] >> 4;
            }

            if (flag & 0x01 && loop_e && *loop_e < 0) *loop_e = (out.size() / chns) + VAG_BLOCK_SAMPLES - 1; // Loop stop
            if (flag & 0x04 && loop_b && *loop_b < 0) *loop_b = (out.size() / chns); // Loop start
            if (flag == 0x07) break; // End playback

            for (int bs_i = 0; bs_i < VAG_BLOCK_SAMPLES; ++bs_i) {
                auto &smpl = data[(chns * bs_i) + ch_i];
                smpl <<= 12;
                if ((short)smpl < 0) smpl |= 0xFFFF0000;

                float tsmp;
                tsmp = smpl;
                tsmp = short(tsmp) >> (coef & 0x0F);
                tsmp += hist[ch_i][0] * VAG_LOOKUP_TABLE[coef >> 4][0];
                tsmp += hist[ch_i][1] * VAG_LOOKUP_TABLE[coef >> 4][1];

                hist[ch_i][1] = hist[ch_i][0];
                hist[ch_i][0] = tsmp;

                smpl = std::min(SHRT_MAX, std::max(int(std::round(tsmp)), SHRT_MIN));
            }
        }

        out.insert(out.end(), data, data + (VAG_BLOCK_SAMPLES * chns));
    }

    return out;
}
