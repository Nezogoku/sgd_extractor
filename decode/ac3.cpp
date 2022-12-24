#include <cmath>
#include <vector>
#include "ac3.hpp"
#include "ac3/mm_accel.h"
#include "ac3/a52.h"

using std::vector;


std::vector<int16_t> ac3Decode(std::vector<char> ac3Data, int channels, int blocksize) {
    vector<vector<int16_t>> t_out(channels);
    vector<int16_t> wavData;

    a52_state_s *t_state = a52_init(0);

    if (ac3Data.size() > 6) {
        for (int a = 0; a < std::floor(ac3Data.size() / (float)blocksize); ++a) {
            uint8_t src[blocksize] = {};

            for (int t = 0; t < blocksize; ++t) {
                src[t] = ac3Data[(a * blocksize) + t];
            }

            int flags = 0, s_rate = 0, b_rate = 0,
                c_flag = 0;

            int decSize = a52_syncinfo(src, &flags, &s_rate, &b_rate);

            if (!decSize) break;

            sample_t lev = 1, bias = 0;
            a52_frame(t_state, src, &c_flag, &lev, bias);

            for (int b = 0; b < 6; ++b) {
                a52_block(t_state);

                sample_t *samples = a52_samples(t_state);
                for (int c = 0, s = 0; c < channels; ++c) {
                    for (int w = 0; w < 256; ++w) {
                        t_out[c].push_back(std::floor(samples[s++] * 32767.5));
                    }
                }
            }
        }
    }

    for (int w = 0; w < t_out[0].size(); ++w) {
        for (int c = 0; c < channels; ++c) {
            wavData.push_back(t_out[c][w]);
        }
    }

    a52_free(t_state);
    t_out.clear();
    ac3Data.clear();

    return wavData;
}
