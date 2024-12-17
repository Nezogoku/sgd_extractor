#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>
#include "liba52/mm_accel.h"
#include "liba52/a52.h"
#include "audio_func.hpp"


///Decodes Sony ADPCM
std::vector<short> decodeDolbyAc3(unsigned char *in, const unsigned length, const unsigned smpls,
                                  const unsigned short align, const unsigned short chns) {
    if (!in || length < 7 || !align) return {};
    
    const unsigned char *in_end = in + length;
    const unsigned short A52_FRAME_BLOCKS = 6;
    const unsigned short A52_FRAME_BLOCK_SAMPLES = 256;
    a52_state_s *t_st;
    std::vector<short> out;
    short *cur = 0, *end = 0;
    
    out.resize(smpls * chns);
    cur = out.data(); end = cur + out.size();
    
    t_st = a52_init(0);
    while (in < in_end) {
        int flgs = 0, c_fl = 0, s_rt = 0, b_rt = 0;
        sample_t lev = 1.0, bias = 0.0;
        
        if (!a52_syncinfo(in, &flgs, &s_rt, &b_rt)) { in += 1; continue; }
        
        if (a52_frame(t_st, in, &c_fl, &lev, bias)) {
            cur += (A52_FRAME_BLOCKS * A52_FRAME_BLOCK_SAMPLES * chns);
            in += align; continue;
        }
        
        for (int bf_i = 0; bf_i < A52_FRAME_BLOCKS; ++bf_i) {
            if (a52_block(t_st)) break;
            sample_t *tmp = a52_samples(t_st);
            short data[A52_FRAME_BLOCK_SAMPLES * chns] {};
            int num_s = sizeof(data) / sizeof(short);
            
            for (int ch_i = 0, bs_i = 0; ch_i < chns; ++ch_i) {
                for (int fb_i = 0; fb_i < A52_FRAME_BLOCK_SAMPLES; ++fb_i) {
                    data[(chns * fb_i) + ch_i] = std::floor(tmp[bs_i++] * 32767.5);
                }
            }
            if (cur + num_s > end) num_s = end - cur;
            
            std::move(data, data + num_s, cur);
            cur += num_s;
        }
        
        in += align;
    }
    a52_free(t_st);
    
    if (cur < end) out.resize(cur - out.data());
    return std::move(out);
}