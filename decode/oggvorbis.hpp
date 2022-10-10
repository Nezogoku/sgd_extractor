#ifndef OGG_VORBIS_HPP
#define OGG_VORBIS_HPP

#include <cstdint>
#include <vector>

//Code for decoding OGG-VORBIS files
std::vector<int16_t> oggVorbDecode(std::vector<char> oggVorbData, int channels);

#endif
