# sgd_extractor
Extracts audio from sgd files and converts them to wav files.

Included stb_vorbis (decode/vorbis) and a52 (decode/ac3) libraries.

Borrowed code from jmarti856 and vgmstream (decode/psadpcm) with some minor changes.

Currently extracts all known audio types except for Atrac3+.

    USAGE: sgd_extractor.exe [-d] <file(s).sgd>
        
        -d prints a log
