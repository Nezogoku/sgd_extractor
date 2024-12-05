#ifndef SGXD_CONST_HPP
#define SGXD_CONST_HPP

//SGXD common codecs
#define CODEC_PCM16LE           0x00 // 16bit Little Endian Integer PCM
#define CODEC_PCM16BE           0x01 // 16bit Big Endian Integer PCM
#define CODEC_OGG_VORBIS        0x02 // Ogg Vorbis
#define CODEC_SONY_ADPCM        0x03 // Sony Adaptive PCM
#define CODEC_SONY_ATRAC3PLUS   0x04 // Sony ATRAC3+
#define CODEC_SONY_SHORT_ADPCM  0x05 // Sony Short Adaptive PCM
#define CODEC_DOLBY_AC_3        0x06 // ATSC A/52

///SGXD Sequence Type
enum SgxdSeqdType : short { SEQD_REQUEST = 0, SEQD_RAWMIDI };

///SGXD Sequence CC Values
enum SgxdSeqdCC : unsigned char {
    //PSX-style controllers
    SEQD_UNKNOWN0               = 0x03,
    SEQD_PSX_LOOP               = 0x63,
    SEQD_UNKNOWN1               = 0x76,
    SEQD_UNKNOWN2,
    //PSX-style loop values
    SEQD_PSX_LOOPSTART          = 0x14,
    SEQD_PSX_LOOPEND            = 0x1E,
};

///SGXD Request Values
enum SgxdRequest : unsigned char {
    REQUEST_STREAM,
    REQUEST_NOTE,
    REQUEST_SEQUENCE,
    REQUEST_SAMPLE,
    REQUEST_SETUP,
    REQUEST_EFFECT,
    REQUEST_CONFIG,
};


#endif
