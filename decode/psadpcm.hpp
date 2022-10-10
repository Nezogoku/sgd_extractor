#ifndef PS_ADPCM_HPP
#define PS_ADPCM_HPP

#include <cstdint>
#include <vector>

#define SAMPLE_BYTES 14
#define SAMPLE_NIBBLE SAMPLE_BYTES * 2
#define SHORT_SAMPLE_BYTES 3
#define SHORT_SAMPLE_NIBBLE SHORT_SAMPLE_BYTES * 2

//Flags for PS-ADPCM flag byte
enum flags {
    NOTHING = 0,         /* Nothing*/
    LOOP_LAST_BLOCK = 1, /* Last block to loop */
    LOOP_REGION = 2,     /* Loop region*/
    LOOP_END = 3,        /* Ending block of the loop */
    LOOP_FIRST_BLOCK = 4,/* First block of looped data */
    UNK = 5,             /* Ending position?*/
    LOOP_START = 6,      /* Starting block of the loop*/
    PLAYBACK_END = 7     /* Playback ending position */
};

//For decoding PS-ADPCM files
const double vagLut[][2] = {
    {0.0,           0.0},
    {60.0 / 64.0,   0.0},
    {115.0 / 64.0,  -52.0 / 64.0},
    {98.0 / 64.0,   -55.0 / 64.0},
    {122.0 / 64.0,  -60.0 / 64.0},

    {30.0 / 64.0,   -0.0 / 64.0},
    {57.5 / 64.0,   -26.0 / 64.0},
    {49.0 / 64.0,   -27.5 / 64.0},
    {61.0 / 64.0,   -30.0 / 64.0},
    {15.0 / 64.0,   -0.0 / 64.0},
    {28.75/ 64.0,   -13.0 / 64.0},
    {24.5 / 64.0,   -13.75/ 64.0},
    {30.5 / 64.0,   -15.0 / 64.0},
    {32.0 / 64.0,   -60.0 / 64.0},
    {15.0 / 64.0,   -60.0 / 64.0},
    {7.0 / 64.0,    -60.0 / 64.0},
};

//PS-ADPCM chunks
struct vchunk {
    uint8_t shift;
    uint8_t predict;
    uint8_t flag;
    uint8_t data[SAMPLE_BYTES];
};

//Short PS-ADPCM chunks
struct gchunk {
    uint8_t shift;
    uint8_t predict;
    uint8_t data[SHORT_SAMPLE_BYTES];
};

//Code for decoding header-less PS-ADPCM files
std::vector<int16_t> adpcmDecode(std::vector<char> adpcmData);

//Code for decoding header-less PS-IMA_ADPCM files
std::vector<int16_t> sAdpcmDecode(std::vector<char> sAdpcmData, int channels);

#endif
