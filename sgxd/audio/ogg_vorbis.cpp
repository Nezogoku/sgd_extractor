#include <vector>
#define STB_VORBIS_NO_CRT
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#include "stb_vorbis/stb_vorbis.h"
#include "audio_func.hpp"

///Decodes Ogg Vorbis
std::vector<short> decodeOgg(unsigned char *in, const unsigned length,
                             const unsigned short chns) {
    if (!in || length < 4) return {};
    
    short buf[chns * 4094] {}; int num_s;
    std::vector<short> out;
    
    auto *vorb = stb_vorbis_open_memory(in, length, NULL, NULL);
    if (!vorb) return {};
    
    while ((num_s = stb_vorbis_get_frame_short_interleaved(vorb, chns, buf, chns * 4094)) > 0) {
        out.insert(out.end(), buf, buf + num_s);
    }
    
    return out;
}
