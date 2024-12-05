#include <cmath>
#include <climits>
#include <vector>
#include "audio_func.hpp"


///Decodes Sony Short ADPCM
std::vector<short> decodeSonyShrtAdpcm(unsigned char *in, const unsigned length,
                                       const unsigned short chns) {
    if (!in || length < 4 || !chns) return {};

    const unsigned char *in_end = in + length;
    const unsigned short VAG_SHORT_BLOCK_ALIGN = 4;
    const unsigned short VAG_SHORT_BLOCK_SAMPLES = (VAG_SHORT_BLOCK_ALIGN - 1) * 2;
    const unsigned short VAG_SHORT_BLOCK_FULL = VAG_SHORT_BLOCK_SAMPLES * chns;
    const double VAG_SHORT_LOOKUP_TABLE[][2] = {
        {0.0, 0.0},
        {60.0 / 64.0, 0.0},
        {115.0 / 64.0, -52.0 / 64.0},
        {98.0 / 64.0, -55.0 / 64.0},
        {122.0 / 64.0, -60.0 / 64.0},
        {30.0 / 64.0, -0.0 / 64.0},
        {57.5 / 64.0, -26.0 / 64.0},
        {49.0 / 64.0, -27.5 / 64.0},
        {61.0 / 64.0, -30.0 / 64.0},
        {15.0 / 64.0, -0.0 / 64.0},
        {28.75/ 64.0, -13.0 / 64.0},
        {24.5 / 64.0, -13.75/ 64.0},
        {30.5 / 64.0, -15.0 / 64.0},
        {32.0 / 64.0, -60.0 / 64.0},
        {15.0 / 64.0, -60.0 / 64.0},
        {7.0 / 64.0, -60.0 / 64.0},
    };
    std::vector<short> out;

    double hist[chns][2] {};
    for (int ch_i = 0; in < in_end; ++ch_i) {
        int o_sz = out.size();
        unsigned char coef = *(in++);
        unsigned char data[VAG_SHORT_BLOCK_SAMPLES] {};

        for (int n = 0; n < VAG_SHORT_BLOCK_SAMPLES; ++in) {
            data[n++] = in[0] & 0x0F;
            data[n++] = in[0] >> 4;
        }

        out.resize(o_sz + VAG_SHORT_BLOCK_FULL);
        for (int bs_i = 0; bs_i < VAG_SHORT_BLOCK_SAMPLES; ++bs_i) {
            short smpl;
            smpl = data[bs_i];
            smpl <<= 12;
            if (smpl & 0x8000) smpl |= 0xFFFF0000;

            float tsmp;
            tsmp = smpl;
            tsmp = short(tsmp) >> (coef & 0x0F);
            tsmp += hist[ch_i][0] * VAG_SHORT_LOOKUP_TABLE[coef >> 4][0];
            tsmp += hist[ch_i][1] * VAG_SHORT_LOOKUP_TABLE[coef >> 4][1];

            hist[ch_i][1] = hist[ch_i][0];
            hist[ch_i][0] = tsmp;

            smpl = std::min(SHRT_MAX, std::max(int(std::round(tsmp)), SHRT_MIN));
            out[o_sz + (bs_i * chns) + (ch_i % chns)] = smpl;
        }
    }

    return out;
}
