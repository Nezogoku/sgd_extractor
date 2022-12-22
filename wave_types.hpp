#ifndef WAVE_TYPES_HPP
#define WAVE_TYPES_HPP

#include <cstdint>

#define SPEAKER_FL          1 << 0
#define SPEAKER_FR          1 << 1
#define SPEAKER_FC          1 << 2
#define SPEAKER_LF          1 << 3
#define SPEAKER_BL          1 << 4
#define SPEAKER_BR          1 << 5
#define SPEAKER_FLC         1 << 6
#define SPEAKER_FRC         1 << 7
#define SPEAKER_BC          1 << 8
#define SPEAKER_SL          1 << 9
#define SPEAKER_SR          1 << 10
#define SPEAKER_TC          1 << 11
#define SPEAKER_TFL         1 << 12
#define SPEAKER_TFC         1 << 13
#define SPEAKER_TRC         1 << 14
#define SPEAKER_TBL         1 << 15
#define SPEAKER_TBC         1 << 16
#define SPEAKER_TBR         1 << 17
#define SPEAKER_RESERVED    1 << 31


struct wave_header {
    const char riff[4] {'R','I','F','F'};
    uint32_t file_size;
    char wave[4] {'W','A','V','E'};

    const char fmt_[4] {'f','m','t',' '};
    uint32_t fmt_size;
    uint16_t fmt_codec;
    uint16_t fmt_channels;
    uint32_t fmt_samplerate;
    uint32_t fmt_byterate;
    uint16_t fmt_blockalign;
    uint16_t fmt_bps;
    uint16_t fmt_sub_size;

    uint16_t fmt_sub_bps_blockalign;
    uint32_t fmt_sub_mask;
    uint64_t fmt_sub_gui[4] {};
    int fmt_sub_padding = 0x00;

    const char fact[4] {'f','a','c','t'};
    uint32_t fact_size;
    uint32_t fact_raw_samples;
    uint32_t fact_skipped_samples;

    const char smpl[4] {'s','m','p','l'};
    uint32_t smpl_size = 0x24;
    uint32_t smpl_manufacturer = 0x00;
    uint32_t smpl_product = 0x00;
    uint32_t smpl_period;
    uint32_t smpl_key = 0x3C;
    uint32_t smpl_tune = 0x00;
    uint32_t smpl_smpte_format = 0x00;
    uint32_t smpl_smpte_offset = 0x00;
    uint32_t smpl_amount_loops = 0x00;
    uint32_t smpl_amount_extra = 0x00;

    uint32_t smpl_loop_id = 0x00;
    uint32_t smpl_loop_type = 0x00;
    uint32_t smpl_loop_start;
    uint32_t smpl_loop_end;
    uint32_t smpl_loop_resolution = 0x00;
    uint32_t smpl_loop_number = 0x00;

    const char data[4] {'d','a','t','a'};
    uint32_t data_size;
};


#endif
