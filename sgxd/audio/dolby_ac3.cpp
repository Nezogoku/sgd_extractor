#include <cmath>
#include <vector>
#include "liba52/mm_accel.h"
#include "liba52/a52.h"
#include "audio_func.hpp"

///Decodes Sony ADPCM
std::vector<short> decodeDolbyAc3(unsigned char *in, const unsigned length, unsigned short align,
                                  const unsigned short chns) {
    if (!in || length < 7) return {};
    if (in[0] == 0x4F &&
        in[1] == 0x67 &&
        in[2] == 0x67 &&
        in[3] == 0x53) return {};
    
    const unsigned char *in_end = in + length;
    const unsigned short A52_FRAME_BLOCKS = 6;
    const unsigned short A52_FRAME_BLOCK_SAMPLES = 256;
    const unsigned short A52_FRAME_BLOCK_FULL = A52_FRAME_BLOCK_SAMPLES * chns;
    a52_state_s *t_st = a52_init(0);
    std::vector<short> out;
    
    if (!align) align = 512;
    while (in < in_end) {
        int flgs = 0, c_fl = 0, s_rt = 0, b_rt = 0;
        sample_t lev = 1.0, bias = 0.0;
        unsigned char in_buf[align] {};
        
        for (int i = 0; i < align; ++i) {
            if (in + i >= in_end) break;
            in_buf[i] = in[i];
        }
        
        if (!a52_syncinfo(in_buf, &flgs, &s_rt, &b_rt)) { in += 1; continue; }
        if (a52_frame(t_st, in_buf, &c_fl, &lev, bias)) { in += align; continue; }
        
        for (int bl_i = 0; bl_i < A52_FRAME_BLOCKS; ++bl_i) {
            int o_sz = out.size();
            out.resize(o_sz + A52_FRAME_BLOCK_FULL);
            
            if (a52_block(t_st)) break;
            sample_t *smpls = a52_samples(t_st);
            
            for (int sm_i = 0; sm_i < A52_FRAME_BLOCK_FULL; ++sm_i) {
                int bs_i = sm_i / A52_FRAME_BLOCK_SAMPLES;
                int ch_i = sm_i % A52_FRAME_BLOCK_SAMPLES;
                out[o_sz + (bs_i * chns) + ch_i] = std::floor(smpls[sm_i] * 32767.5);
            }
        }
        
        in += align;
    }
    a52_free(t_st);
    
    return out;
}