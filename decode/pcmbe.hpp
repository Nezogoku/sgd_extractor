#ifndef PCMBE_HPP
#define PCMBE_HPP

#include <cstdint>
#include <vector>

///Code for converting header-less PCMBE files
std::vector<int16_t> convertPcmBE(std::vector<char> pcmData);

#endif
