#ifndef SGXD_TYPES_HPP
#define SGXD_TYPES_HPP

#include <cstdint>
#include <vector>
#include <string>

#define SGXD    0x53475844
#define RGND    0x52474E44
#define SEQD    0x53455144
#define WAVE    0x57415645
#define NAME    0x4E414D45

enum sgxdCodec {
    PCM16BE = 0x01,
    OGGVORB,
    cSADPCM,
    ATRAC3p,
    sSADPCM,
    DOLAC_3
};


struct rgnd_def {
    uint32_t determinator_snd;
    uint32_t determinator_sfx;
    uint8_t range_low;
    uint8_t range_high;
    uint8_t root_key;
    int8_t correction;
    int8_t pan;
    uint32_t sample_id;
};

struct seqd_def {
    uint32_t name_offset;
    uint8_t sequence_format;
};

struct wave_def {
    uint32_t name_offset;
    uint8_t codec;
    uint8_t channels;
    uint32_t sample_rate;
    uint32_t info_type;
    uint32_t info_value;
    int32_t sample_loopstart;
    int32_t sample_loopend;
    uint32_t sample_size;
    uint32_t stream_size;
    uint32_t stream_offset;
    uint32_t stream_size_full;

    bool is_looped;
    bool is_bank;
    std::vector<char> data;

    std::string name;
    std::vector<int16_t> pcmle;
};

struct name_def {
    uint16_t name_id;
    uint16_t name_type;
    uint32_t name_offset;

    std::string name;
};


#endif
