#include <vector>
#include "maiatrac3plus/Mai_Base0.h"
#include "maiatrac3plus/MaiAT3PlusFrameDecoder.h"
#include "audio_func.hpp"

///Decodes Sony ADPCM
std::vector<short> decodeSonyAt3p(unsigned char *in, const unsigned length, const unsigned short align,
                                  const unsigned short chns, const unsigned skip) {
    if (!in || !length || !align || !chns) return {};

    const unsigned char *in_end = in + length;
    const unsigned short AT3P_FRAME_SAMPLES = 2048;
    const unsigned short AT3P_FRAME_FULL = AT3P_FRAME_SAMPLES * chns;
    MaiAT3PlusFrameDecoder t_st;
    std::vector<short> out;

    while (in < in_end) {
        Mai_I8 in_buf[align] {};
        Mai_I16 t_buf[AT3P_FRAME_FULL] {}, **t_ptr = new Mai_I16*[1] {};
        Mai_I32 o_ch;
        int o_sz = out.size(), bf_i = 0;

        for (auto &b : in_buf) {
            if (in >= in_end) break;
            b = *(in++);
        }
        out.resize(o_sz + AT3P_FRAME_FULL);

        t_ptr[0] = t_buf;
        if (t_st.decodeFrame(in_buf, align, &o_ch, t_ptr)) continue;
        if (o_ch != chns) continue;
        while (o_sz < out.size()) out[o_sz++] = t_buf[bf_i++];
    }
    //out.erase(out.begin(), out.begin() + (skip * chns));

    return out;
}
