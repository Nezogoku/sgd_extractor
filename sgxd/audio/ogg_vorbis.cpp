#include <vector>
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#include "stb_vorbis/stb_vorbis.h"
#include "audio_func.hpp"

///Decodes Ogg Vorbis
std::vector<short> decodeOgg(unsigned char *in, const unsigned length,
                             const unsigned short chns) {
    if (!in || length < 4) return {};
    
    const unsigned short BUFFER_SIZE = 4096 * chns;
    short t_buf[BUFFER_SIZE] {};
    stb_vorbis *t_st;
    unsigned n_smp;
    std::vector<short> out;

    t_st = stb_vorbis_open_memory(in, length, NULL, NULL);
    if (!t_st) return {};

    //Get number samples
    n_smp = stb_vorbis_stream_length_in_samples(t_st) * chns;

    while (n_smp) {
        n_smp = stb_vorbis_get_samples_short_interleaved(t_st, chns, t_buf, BUFFER_SIZE);
        out.insert(out.end(), t_buf, t_buf + (n_smp * chns));
    }

    stb_vorbis_close(t_st);
    
    return out;
}
