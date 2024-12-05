#include <cmath>
#include <climits>
#include <vector>
#include "audio_func.hpp"


///Decodes Sony ADPCM
std::vector<short> decodeSonyAdpcm(unsigned char *in, const unsigned length) {
    if (!in || length < 16) return {};

    const unsigned char *in_end = in + length;
    const unsigned short VAG_ALIGN = 16;
    const unsigned short VAG_SAMPLES = (VAG_ALIGN - 2) * 2;
    const double VAG_LOOKUP_TABLE[][2] = {
        {0.0, 0.0},
        {60.0 / 64.0, 0.0},
        {115.0 / 64.0, -52.0 / 64.0},
        {98.0 / 64.0, -55.0 / 64.0},
        {122.0 / 64.0, -60.0 / 64.0},
    };
    std::vector<short> out;

    //loop_beg = loop_end = -1;

    double hist[2] {};
    while (in < in_end) {
        unsigned char coef = *(in++);
        unsigned char flag = *(in++);
        unsigned char data[VAG_SAMPLES] {};

        for (int n = 0; n < VAG_SAMPLES; ++in) {
            data[n++] = in[0] & 0x0F;
            data[n++] = in[0] >> 4;
        }

        if (flag == 0x07) break; // End playback
        //if (flag == 0x04) loop_beg = out.size();

        for (const auto &nib : data) {
            short smpl;
            smpl = nib;
            smpl <<= 12;
            if (smpl & 0x8000) smpl |= 0xFFFF0000;

            float tsmp;
            tsmp = smpl;
            tsmp = short(tsmp) >> (coef & 0x0F);
            tsmp += hist[0] * VAG_LOOKUP_TABLE[coef >> 4][0];
            tsmp += hist[1] * VAG_LOOKUP_TABLE[coef >> 4][1];

            hist[1] = hist[0];
            hist[0] = tsmp;

            smpl = std::min(SHRT_MAX, std::max(int(std::round(tsmp)), SHRT_MIN));
            out.push_back(smpl);
        }
        //if (flag == 0x01) loop_end = out.size();
    }
    //if (loop_beg < 0 || loop_end < 0) loop_beg = loop_end = -1;

    return out;
}
