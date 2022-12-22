#include <algorithm>
#include <cmath>
#include <climits>
#include <vector>
#include "psadpcm.hpp"

using std::vector;
using std::round;
using std::max;
using std::min;


std::vector<int16_t> adpcmDecode(std::vector<char> adpcmData, int &loop_s, int &loop_e, bool &isLooped) {
    vector<int16_t> wavData;

    bool isLoopStart = false;
    bool isLoopEnd = false;

    vchunk adpcmChunk;
    for (int i = 0; i < adpcmData.size();) {
        char coeff = adpcmData[i++];                                                        // Get decoding coefficient

        adpcmChunk.shift = int8_t(coeff & 0xF);                                             // Get shift byte
        adpcmChunk.predict = int8_t((coeff & 0xF0) >> 4);                                   // Get predicting byte
        adpcmChunk.flag = adpcmData[i++];                                                   // Get flag byte

        //Get compressed sound data
        for (int ind = 0, len = i + SAMPLE_BYTES; i < len; ++i, ++ind) adpcmChunk.data[ind] = adpcmData[i];

        if (adpcmChunk.flag == PLAYBACK_END) break;
        else if (adpcmChunk.flag == LOOP_FIRST_BLOCK) {
            isLoopStart = true;
            loop_s = wavData.size();
            if (wavData.size()) loop_s -= 1;
        }
        else if (adpcmChunk.flag == LOOP_LAST_BLOCK) {
            isLoopEnd = true;
            loop_e = wavData.size() + SAMPLE_NIBBLE;
            if (wavData.size()) loop_e -= 1;
        }

        int16_t samples[SAMPLE_NIBBLE];

        //Expand nibble to byte
        for (int b = 0; b < SAMPLE_BYTES; ++b) {
            samples[b * 2] = adpcmChunk.data[b] & 0x0F;
            samples[(b * 2) + 1] = (adpcmChunk.data[b] & 0xF0) >> 4;
        }

        //Decode samples
        for (int b = 0; b < SAMPLE_NIBBLE; ++b) {
            //Shift nibble to top range
            int16_t samp = samples[b] << 12;

            if ((samp & 0x8000)) samp = int16_t(samp | 0xFFFF0000);

            double sample;
            sample = samp;
            sample = int16_t(sample) >> adpcmChunk.shift;
            sample += adpcmChunk.hist[0] * vagLut[adpcmChunk.predict][0];
            sample += adpcmChunk.hist[1] * vagLut[adpcmChunk.predict][1];

            adpcmChunk.hist[1] = adpcmChunk.hist[0];
            adpcmChunk.hist[0] = sample;

            //Ensure new sample is not outside int16_t value range
            int16_t newSample;
            newSample = int16_t(min(SHRT_MAX, max(int(round(sample)), SHRT_MIN)));

            wavData.push_back(newSample);
        }
    }

    adpcmData.clear();

    if (isLoopStart && isLoopEnd) isLooped = true;

    return wavData;
}

std::vector<int16_t> sAdpcmDecode(std::vector<char> sAdpcmData, int channels) {
    vector<vector<int16_t>> t_out(channels);
    //int a_size = (sAdpcmData.size() /
    int l_size = 0;

    vector<int16_t> wavData;

    int ch = 0;

    gchunk adpcmChunk[channels] = {};
    for (int i = 0; i < sAdpcmData.size();) {
        char coeff = sAdpcmData[i++];                                                       // Get decoding coefficient

        adpcmChunk[ch].shift = int8_t(coeff & 0x0F);                                        // Get shift byte
        adpcmChunk[ch].predict = int8_t((coeff & 0xF0) >> 4);                               // Get predicting byte

        //Get compressed sound data
        for (int ind = 0, len = i + SHORT_SAMPLE_BYTES; i < len; ++i, ++ind) adpcmChunk[ch].data[ind] = sAdpcmData[i];

        int16_t samples[SHORT_SAMPLE_NIBBLE] = {};

        //Expand nibble to byte
        for (int b = 0; b < SHORT_SAMPLE_BYTES; ++b) {
            samples[b * 2] = adpcmChunk[ch].data[b] & 0x0F;
            samples[(b * 2) + 1] = (adpcmChunk[ch].data[b] & 0xF0) >> 4;
        }

        //Decode samples
        for (int b = 0; b < SHORT_SAMPLE_NIBBLE; ++b) {
            //Shift nibble to top range
            int16_t samp = samples[b] << 12;

            if ((samp & 0x8000)) samp = int16_t(samp | 0xFFFF0000);

            double sample;
            sample = samp;
            sample = int16_t(sample) >> adpcmChunk[ch].shift;
            sample += adpcmChunk[ch].hist[0] * vagLut[adpcmChunk[ch].predict][0];
            sample += adpcmChunk[ch].hist[1] * vagLut[adpcmChunk[ch].predict][1];

            adpcmChunk[ch].hist[1] = adpcmChunk[ch].hist[0];
            adpcmChunk[ch].hist[0] = sample;

            //Ensure new sample is not outside int16_t value range
            int16_t newSample;
            newSample = int16_t(min(INT16_MAX, max(int(round(sample)), INT16_MIN)));

            t_out[ch].push_back(newSample);
        }

        //Update channel
        ch = (ch + 1) % channels;
    }

    //Match sizes of samples
    for (auto &vect : t_out) if (vect.size() > l_size) l_size = vect.size();
    for (auto &vect : t_out) if (vect.size() < l_size) vect.resize(l_size);

    for (int w = 0; w < l_size; ++w) {
        for (int c = 0; c < channels; ++c) {
            wavData.push_back(t_out[c][w]);
        }
    }

    sAdpcmData.clear();
    t_out.clear();

    return wavData;
}
