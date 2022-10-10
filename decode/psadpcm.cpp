#include <cmath>
#include <climits>
#include <iostream>
#include <vector>
#include "psadpcm.hpp"

using std::vector;
using std::round;
using std::max;
using std::min;


std::vector<int16_t> adpcmDecode(std::vector<char> adpcmData) {
    vector<int16_t> wavData;

    double hist0 = 0.0,
           hist1 = 0.0;

    for (int i = 0; i < adpcmData.size(); ++i) {
        char decodingCoeff;
        decodingCoeff = adpcmData[i++];                                                     // Get decoding coefficient

        vchunk adpcmChunk;
        adpcmChunk.shift = int8_t(decodingCoeff & 0xF);                                     // Get shift byte
        adpcmChunk.predict = int8_t((decodingCoeff & 0xF0) >> 4);                           // Get predicting byte
        adpcmChunk.flag = adpcmData[i++];                                                   // Get flag byte
        //Get compressed sound data
        for (int b = i, ind = 0; b < i + SAMPLE_BYTES; ++b, ++ind) adpcmChunk.data[ind] = adpcmData[b];
        i += SAMPLE_BYTES - 1;                                                              // Play catch up

        if (adpcmChunk.flag == PLAYBACK_END) break;

        int16_t *samples = new int16_t[SAMPLE_NIBBLE];

        //Expand nibble to byte
        for (int b = 0; b < SAMPLE_BYTES; ++b) {
            samples[b * 2] = adpcmChunk.data[b] & 0xF0;
            samples[(b * 2) + 1] = (adpcmChunk.data[b] & 0xF0) >> 4;
        }

        //Decode samples
        for (int b = 0; b < SAMPLE_NIBBLE; ++b) {
            //Shift nibble to top range
            int16_t samp = samples[b] << 12;

            if ((samp & 0x8000) != 0) samp = int16_t(samp | 0xFFFF0000);

            double sample;
            sample = samp;
            sample = int16_t(sample) >> adpcmChunk.shift;
            sample += hist0 * vagLut[adpcmChunk.predict][0];
            sample += hist1 * vagLut[adpcmChunk.predict][1];

            hist1 = hist0;
            hist0 = sample;

            //Ensure new sample is not outside int16_t value range
            int16_t newSample;
            newSample = int16_t(min(SHRT_MAX, max(int(round(sample)), SHRT_MIN)));

            wavData.push_back(newSample);
        }

        delete[] samples;
    }

    adpcmData.clear();

    return wavData;
}

//Work in progress...
std::vector<int16_t> sAdpcmDecode(std::vector<char> sAdpcmData, int channels) {
    vector<vector<int16_t>> chanData;
    chanData.resize(channels);

    vector<int16_t> wavData;

    double hist0 = 0.0,
           hist1 = 0.0;

    int ch = 0;
    for (int i = 0; i < sAdpcmData.size(); ++i) {
        char decodingCoeff;
        decodingCoeff = sAdpcmData[i++];                                                    // Get decoding coefficient

        gchunk adpcmChunk;
        adpcmChunk.shift = int8_t(decodingCoeff & 0x0F);                                    // Get shift byte
        adpcmChunk.predict = int8_t((decodingCoeff & 0xF0) >> 4);                           // Get predicting byte
        //Get compressed sound data
        for (int b = i, ind = 0; b < i + SHORT_SAMPLE_BYTES; ++b, ++ind) adpcmChunk.data[ind] = sAdpcmData[b];
        i += SHORT_SAMPLE_BYTES - 1;                                                        // Play catch up

        int16_t *samples = new int16_t[SHORT_SAMPLE_NIBBLE];

        //Expand nibble to byte
        for (int b = 0; b < SHORT_SAMPLE_BYTES; ++b) {
            samples[b * 2] = adpcmChunk.data[b] & 0xF0;
            samples[(b * 2) + 1] = (adpcmChunk.data[b] & 0xF0) >> 4;
        }

        //Decode samples
        for (int b = 0; b < SHORT_SAMPLE_NIBBLE; ++b) {
            double sample = 0;
            sample = ((int16_t)(samples[b] << 12) & 0xF000);
            if (((int16_t)sample & 0x8000) != 0) sample = ((int16_t)sample | 0xFFFF0000);
            sample = ((int16_t)sample >> adpcmChunk.shift);
            sample += vagLut[adpcmChunk.predict][0] * hist0;
            sample += vagLut[adpcmChunk.predict][1] * hist1;

            hist1 = hist0;
            hist0 = sample;

            //Ensure new sample is not outside int16_t value range
            int16_t newSample;
            newSample = int16_t(min(INT16_MAX, max(int(round(sample)), INT16_MIN)));

            chanData[ch++].push_back(newSample);
            ch %= channels;
        }

        delete[] samples;
    }


    //Interleave samples per channel
    for (int d = 0; d < chanData[0].size(); ++d) {
        for (int c = 0; c < channels; ++c) { wavData.push_back(chanData[c][d]); }
    }

    sAdpcmData.clear();
    chanData.clear();

    return wavData;
}
