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
    for (int ba_i = 0; ba_i < length / (VAG_SHORT_BLOCK_ALIGN * chns); ++ba_i) {
        int data[VAG_SHORT_BLOCK_SAMPLES * chns] {};
        
        for (int ch_i = 0; ch_i < chns; ++ch_i) {
            unsigned char coef = *(in++);
            for (int n = 0; n < VAG_SHORT_BLOCK_SAMPLES; ++in) {
                data[(chns * n++) + ch_i] = in[0] & 0x0F;
                data[(chns * n++) + ch_i] = in[0] >> 4;
            }
            
            for (int bs_i = 0; bs_i < VAG_SHORT_BLOCK_SAMPLES; ++bs_i) {
                auto &smpl = data[(chns * bs_i) + ch_i];
                smpl <<= 12;
                if ((short)smpl < 0) smpl |= 0xFFFF0000;

                float tsmp;
                tsmp = smpl;
                tsmp = short(tsmp) >> (coef & 0x0F);
                tsmp += hist[ch_i][0] * VAG_SHORT_LOOKUP_TABLE[coef >> 4][0];
                tsmp += hist[ch_i][1] * VAG_SHORT_LOOKUP_TABLE[coef >> 4][1];

                hist[ch_i][1] = hist[ch_i][0];
                hist[ch_i][0] = tsmp;

                smpl = std::min(SHRT_MAX, std::max(int(std::round(tsmp)), SHRT_MIN));
            }
        }
        
        out.insert(out.end(), data, data + (VAG_SHORT_BLOCK_SAMPLES * chns));
    }

    return out;
}
