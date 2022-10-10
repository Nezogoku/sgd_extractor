#include <vector>
#include "oggvorbis.hpp"
#include "stb_vorbis.h"


std::vector<int16_t> oggVorbDecode(std::vector<char> oggVorbData, int channels) {
    std::vector<int16_t> wavData;

    stb_vorbis *oggStream;
    stb_vorbis_info oggInfo;
    int oggSamples;

    oggStream = stb_vorbis_open_memory((unsigned char*)oggVorbData.data(), oggVorbData.size(), NULL, NULL);
    oggInfo = stb_vorbis_get_info(oggStream);

    //Update number channels
    oggInfo.channels = channels;
    //Get number samples
    oggSamples = stb_vorbis_stream_length_in_samples(oggStream) * oggInfo.channels;

    const int BUFFER_SIZE = 65536;
    int16_t buffer[BUFFER_SIZE];

    while (oggSamples) {
        oggSamples = stb_vorbis_get_samples_short_interleaved(oggStream, oggInfo.channels, buffer, BUFFER_SIZE);
        for (int b = 0; b < oggSamples; ++b) { wavData.push_back(buffer[b]); }
    }

    stb_vorbis_close(oggStream);

    return wavData;
}
