# sgd_extractor
Extracts miscellaneous data from Sony .sgd and .sgh+.sgb files.
SGD files contain the entire data.
SGH files are headers whereas SGB files contain the audio.

Included a52, maiatrac3plus, and stb_vorbis libraries.

Borrowed code from jmarti856 and vgmstream (sgxd/audio) with some minor changes.

Information on SGXD based off of personal exploration and information pulled from https://github.com/Nenkai/010GameTemplates/blob/main/Sony/SGXD.bt.

## How to use

`sgd_extractor` and `sgd_extractor -h` displays a help message

`sgd_extractor -d ...` toggles debug mode

`sgd_extractor -t ...` activates text extraction mode (following .sgd files have textual information additionally extracted)