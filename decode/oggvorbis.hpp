#ifndef OGG_VORBIS_HPP
#define OGG_VORBIS_HPP

#include <cstdint>
#include <vector>

//Code for decoding header-less PS-ADPCM files
std::vector<int16_t> oggVorbDecode(std::vector<char> oggVorbData);

#endif
