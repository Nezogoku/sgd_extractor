#include <algorithm>
#include <vector>
#include <utility>
#include "maiatrac3plus/Mai_Base0.h"
#include "maiatrac3plus/MaiAT3PlusFrameDecoder.h"
#include "audio_func.hpp"

///Decodes Sony AT3+
std::vector<short> decodeSonyAt3p(unsigned char *in, const unsigned length, const unsigned smpls,
                                  const unsigned short align, const unsigned short chns, const unsigned *skip) {
    if (!in || !length || !align || !chns) return {};

    const unsigned char *in_end = in + length;
    const unsigned short AT3P_FRAME_SAMPLES = 2048;
    MaiAT3PlusFrameDecoder t_st;
    std::vector<short> out;
    short *cur = 0, *end = 0;
    
    out.resize(smpls * chns);
    cur = out.data(); end = cur + out.size();

    while (in < in_end) {
        Mai_I8 buf[align] {};
        Mai_I16 *ptr = 0;
        Mai_I32 o_ch;
        int num_s = AT3P_FRAME_SAMPLES * chns;

        for (auto &b : buf) {
            if (in >= in_end) break;
            b = *(in++);
        }

        if (t_st.decodeFrame(buf, align, &o_ch, &ptr)) continue;
        if (o_ch != chns) continue;
        if (cur + num_s > end) num_s = end - cur;
        
        if (ptr) { std::move(ptr, ptr + num_s, cur); cur += num_s; }
    }

    if (cur < end) out.resize(cur - out.data());
    if (skip) out.erase(out.begin(), out.begin() + (*skip * chns));
    return std::move(out);
}
