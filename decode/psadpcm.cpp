#include <cmath>
#include <climits>
#include <vector>
#include "psadpcm.hpp"

using std::vector;
using std::round;
using std::max;
using std::min;


vector<int16_t> adpcmDecode(vector<char> adpcmData) {
    vector<int16_t> wavData;

    double hist0 = 0.0,
           hist1 = 0.0;

    for (int i = 0; i < adpcmData.size(); ++i) {
        char decodingCoeff;
        decodingCoeff = adpcmData[i++];                                                     // Get decoding coefficient

        chunk adpcmChunk;
        adpcmChunk.shift = int8_t(decodingCoeff & 0xF);                                     // Get shift byte
        adpcmChunk.predict = int8_t((decodingCoeff & 0xF0) >> 4);                           // Get predicting byte
        adpcmChunk.flag = adpcmData[i++];                                                   // Get flag byte

        for (int b = i, ind = 0; b < i + 14; ++b, ++ind) adpcmChunk.data[ind] = adpcmData[b];   // Get compressed sound data
        i += 13;                                                                            // Play catch up

        if (adpcmChunk.flag == PLAYBACK_END) break;

        int16_t *samples = new int16_t[SAMPLE_NIBBLE];

        //Expand nibble to byte
        for (int b = 0; b < SAMPLE_BYTES; ++b) {
            samples[b * 2] = adpcmChunk.data[b] & 0xF;
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

    vector<char> ().swap(adpcmData);

    return wavData;
}
