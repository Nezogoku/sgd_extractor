#include <cmath>
#include <climits>
#include <vector>
#include "pcmbe.hpp"

using std::vector;


vector<int16_t> convertPcmBE(vector<char> pcmData) {
    vector<int16_t> wavData;

    for (int i = 0; i < pcmData.size(); i += 2) {
        int16_t sample = (((int16_t)pcmData[i + 1]) << 8) | (0x00FF & pcmData[i]);

        //Ensure new sample is not outside int16_t value range
        int16_t newSample;
        newSample = int16_t(std::min(SHRT_MAX, std::max(int(round(sample)), SHRT_MIN)));

        wavData.push_back(newSample);
    }

    vector<char> ().swap(pcmData);

    return wavData;
}
