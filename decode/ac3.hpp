#ifndef AC_3_HPP
#define AC_3_HPP

#include <cstdint>
#include <vector>

std::vector<int16_t> ac3Decode(std::vector<char> ac3Data, int channels, int blocksize);

#endif
