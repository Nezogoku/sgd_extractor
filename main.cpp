#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <direct.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "decode.hpp"

using std::cerr;
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::remove;
using std::string;
using std::vector;


#define SGXD    0x53475844
#define RGND    0x52474E44
#define SEQD    0x53455144
#define WAVE    0x57415645
#define NAME    0x4E414D45

#define PCM16BE 0x01
#define OGGVORB 0x02
#define lPADPCM 0x03
#define ATRAC3p 0x04
#define sPADPCM 0x05
#define PS3_AC3 0x06

#define RIFF    "RIFF"
#define rWAVE   "WAVE"
#define fmt_    "fmt "
#define DATA    "data"


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
    uint32_t sample_size;
    uint32_t loop_start;
    uint32_t loop_end;
    uint32_t stream_size;
    uint32_t stream_offset;
    uint32_t stream_size_full;

    vector<char> data;
    vector<int16_t> pcmle;
    string name;
};

struct name_def {
    uint16_t name_id;
    uint16_t name_type;
    uint32_t name_offset;

    string name;
};


void setReverse(uint32_t &tmpInt) {
    uint32_t buffer = 0x00;
    for (int b = 0; b < 4; ++b) {
        buffer |= uint8_t((tmpInt >> (0x00 + (8 * b))) & 0xFF);
        if (b != 3) buffer <<= 8;
    }
    tmpInt = buffer;
}

bool findSGXD(ifstream &in, int &dex) {
    uint32_t chunk;
    char buff;
    bool found = false;

    while (!found && in.get(buff)) {
        in.seekg(dex);
        in.read((char*)(&chunk), sizeof(uint32_t));
        setReverse(chunk);

        if (chunk == SGXD) found = true;
        else dex += 0x01;
    }

    return found;
}

bool getSamples(ifstream &in, int dex, bool log) {
    if (log) cout << std::setfill('0') << std::right;

    bool successful = false;
    int counter = 0;

    while (findSGXD(in, dex)) {
        cout << "SGD found: " << ++counter << endl;

        string bank_name, newDir;
        uint32_t chunk, numsnd, numrgd, numnam,
                 sgxd_offset, rgnd_offset, seqd_offset, wave_offset, name_offset,
                 data_length, rgnd_length, seqd_length, wave_length, name_length,
                 sgxd_name_offset, data_offset,
                 rgnd_data_offset, seqd_data_offset, wave_data_offset, name_data_offset;
        bool hasSamples = false,
             hasNames = false,
             hasDefinitions = false,
             hasSequences = false;
        char buff;

        sgxd_offset = dex;

        in.seekg(dex + 0x04);
        in.read((char*)(&sgxd_name_offset), sizeof(uint32_t));

        in.seekg(dex + 0x08);
        in.read((char*)(&data_offset), sizeof(uint32_t));

        in.seekg(dex + 0x0C);
        in.read((char*)(&data_length), sizeof(uint32_t));
        data_length -= 0x80000000;

        dex += 0x10;
        while (in.get(buff) && dex < (sgxd_offset + data_offset + data_length)) {
            in.seekg(dex);
            in.read((char*)(&chunk), sizeof(uint32_t));
            setReverse(chunk);

            if (chunk == RGND) {
                if (log) cout << "\nRGND header found at position: 0x" << hex << dex << dec << endl;
                hasDefinitions = true;
                rgnd_offset = dex;

                in.seekg(dex + 0x04);
                in.read((char*)(&rgnd_length), sizeof(uint32_t));

                in.seekg(dex + 0x0C);
                in.read((char*)(&chunk), sizeof(uint32_t));
                rgnd_data_offset = (dex + 0x10) + (chunk * 0x08);
                if (log) cout << "RGND data found at position: 0x" << hex << rgnd_data_offset << dec << endl;

                numrgd = (rgnd_offset + 0x08 + rgnd_length) - rgnd_data_offset;
                numrgd = std::floor(double(numrgd) / 0x38);

                dex += (rgnd_length + 0x08);
            }
            else if (chunk == SEQD) {
                if (log) cout << "\nSEQD header found at position: 0x" << hex << dex << dec << endl;
                hasSequences = true;
                seqd_offset = dex;

                in.seekg(dex + 0x04);
                in.read((char*)(&seqd_length), sizeof(uint32_t));

                seqd_data_offset = dex + 0x08;

                dex += (seqd_length + 0x08);
            }
            else if (chunk == WAVE) {
                if (log) cout << "\nWAVE header found at position: 0x" << hex << dex << dec << endl;
                hasSamples = true;
                wave_offset = dex;

                in.seekg(dex + 0x04);
                in.read((char*)(&wave_length), sizeof(uint32_t));

                in.seekg(dex + 0x0C);
                in.read((char*)(&numsnd), sizeof(uint32_t));
                if (log) cout << "Number of samples: " << numsnd << endl;

                wave_data_offset = dex + 0x10;
                if (log) cout << "WAVE data found at position: 0x" << hex << wave_data_offset << dec << endl;

                dex += (wave_length + 0x08);
            }
            else if (chunk == NAME) {
                hasNames = true;

                if (log) cout << "\nNAME header found at position: 0x" << hex << dex << dec << endl;
                wave_offset = dex;

                in.seekg(dex + 0x04);
                in.read((char*)(&name_length), sizeof(uint32_t));

                in.seekg(dex + 0x0C);
                in.read((char*)(&numnam), sizeof(uint32_t));
                if (log) cout << "Number of names: " << numnam << endl;

                name_data_offset = dex + 0x10;
                if (log) cout << "NAME data found at position: 0x" << hex << name_data_offset << dec << endl;

                dex += (name_length + 0x08);
                if (log) cout << endl;
                break;
            }
            else dex += 0x04;
        }

        if (!hasDefinitions) {
            cerr << "This file isn't a sound bank, so can't be parsed at the moment . . ." << endl;
            continue;
        }
        if (!hasSamples) {
            cerr << "There might be no samples in this file . . ." << endl;
            continue;
        }
        if (!hasNames) {
            cerr << "No names in this file . . ." << endl;
            continue;
        }


        //Get name offsets of samples
        vector<name_def> name_definitions;
        for (int d = 0; d < numnam; ++d) {
            name_def temp;
            dex = name_data_offset + (d * 0x08);

            in.seekg(dex + 0x00);
            in.read((char*)(&temp.name_id), sizeof(uint16_t));
            if (log) cout << "Name ID: " << temp.name_id << endl;

            in.seekg(dex + 0x02);
            in.read((char*)(&temp.name_type), sizeof(uint16_t));
            if (log) cout << "Name type: 0x" << hex << temp.name_type << dec << endl;

            in.seekg(dex + 0x04);
            in.read((char*)(&temp.name_offset), sizeof(uint32_t));
            if (log) cout << "Name offset: 0x" << hex << sgxd_offset + temp.name_offset << dec << endl;

            //Type 0x2000 to 0x2FFF denotes sample maybe?
            if (temp.name_type >= 0x2000 && temp.name_type < 0x3000) {
                name_definitions.push_back(temp);
            }
        }
        if (log) cout << endl;

        //Get stored names
        for (auto offset_list : name_definitions) {
            offset_list.name = "";

            in.seekg(offset_list.name_offset);
            do {
                in.get(buff);
                if (buff != 0x00) offset_list.name += buff;
            } while (buff != 0x00);

            if (log) cout << "Name ID: " << offset_list.name_id << endl;
            if (log) cout << "Name type: 0x" << hex << offset_list.name_type << dec << endl;
            if (log) cout << "Added name: " << offset_list.name << endl;
        }
        if (log) cout << endl;


        //Get bank name
        bank_name = "";
        in.seekg(sgxd_offset + sgxd_name_offset);
        do {
            in.get(buff);
            if (buff != 0x00) bank_name += buff;
        } while (buff != 0x00);

        if (log) cout << "Bank name: " << bank_name << endl;
        newDir = '@' + bank_name;
        if (log) cout << "Output directory: \"" << newDir << "\"" << endl;
        if (log) cout << endl;


        //Get wave data
        vector<wave_def> wave_data;
        for (int w = 0; w < numsnd; ++w) {
            wave_def temp_wave;

            dex = wave_data_offset + (w * 0x38);

            in.seekg(dex + 0x04);
            in.read((char*)(&temp_wave.name_offset), sizeof(uint32_t));

            //Get sample name if specified
            if (temp_wave.name_offset) {
                temp_wave.name = "";
                in.seekg(sgxd_offset + temp_wave.name_offset);
                do {
                    in.get(buff);
                    if (buff != 0x00) temp_wave.name += buff;
                } while (buff != 0x00);
            }

            in.seekg(dex + 0x08);
            in.read((char*)(&temp_wave.codec), sizeof(uint8_t));

            in.seekg(dex + 0x09);
            in.read((char*)(&temp_wave.channels), sizeof(uint8_t));

            in.seekg(dex + 0x0C);
            in.read((char*)(&temp_wave.sample_rate), sizeof(uint32_t));

            in.seekg(dex + 0x10);
            in.read((char*)(&temp_wave.info_type), sizeof(uint32_t));

            in.seekg(dex + 0x14);
            in.read((char*)(&temp_wave.info_value), sizeof(uint32_t));

            in.seekg(dex + 0x20);
            in.read((char*)(&temp_wave.sample_size), sizeof(uint32_t));

            in.seekg(dex + 0x24);
            in.read((char*)(&temp_wave.loop_start), sizeof(uint32_t));

            in.seekg(dex + 0x28);
            in.read((char*)(&temp_wave.loop_end), sizeof(uint32_t));

            in.seekg(dex + 0x2C);
            in.read((char*)(&temp_wave.stream_size), sizeof(uint32_t));

            in.seekg(dex + 0x30);
            in.read((char*)(&temp_wave.stream_offset), sizeof(uint32_t));

            in.seekg(dex + 0x34);
            in.read((char*)(&temp_wave.stream_size_full), sizeof(uint32_t));

            wave_data.push_back(temp_wave);
        }


        //Get rgnd data
        vector<rgnd_def> rgnd_data;
        for (int r = 0; r < numrgd; ++r) {
            rgnd_def temp_rgnd;

            dex = rgnd_data_offset + (r * 0x38);
            if (log) cout << "Current offset: 0x" << hex << dex << dec << endl;

            in.seekg(dex + 0x00);
            in.read((char*)(&temp_rgnd.determinator_snd), sizeof(uint32_t));
            //Might actually refer to output channel or something...
            if (log) cout << "Sample Determinator: " << temp_rgnd.determinator_snd << endl;

            in.seekg(dex + 0x0C);
            in.read((char*)(&temp_rgnd.determinator_sfx), sizeof(uint32_t));
            if (log) cout << "SFX Determinator: " << hex
                          << "0x" << std::setw(2) << int(temp_rgnd.determinator_sfx & 0xFF)         << ' '
                          << "0x" << std::setw(2) << int((temp_rgnd.determinator_sfx >> 8 ) & 0xFF) << ' '
                          << "0x" << std::setw(2) << int((temp_rgnd.determinator_sfx >> 16) & 0xFF) << ' '
                          << "0x" << std::setw(2) << int((temp_rgnd.determinator_sfx >> 24) & 0xFF)
                          << dec << endl;

            in.seekg(dex + 0x18);
            in.read((char*)(&temp_rgnd.range_low), sizeof(uint8_t));
            if (log) cout << "Lowest range: " << int(temp_rgnd.range_low) << endl;

            in.seekg(dex + 0x19);
            in.read((char*)(&temp_rgnd.range_high), sizeof(uint8_t));
            if (log) cout << "Highest range: " << int(temp_rgnd.range_high) << endl;

            in.seekg(dex + 0x1C);
            in.read((char*)(&temp_rgnd.root_key), sizeof(uint8_t));
            if (log) cout << "Root key: " << int(temp_rgnd.root_key) << endl;

            in.seekg(dex + 0x1D);
            in.read((char*)(&temp_rgnd.correction), sizeof(uint8_t));
            if (log) cout << "Fine tune: " << int(temp_rgnd.correction) << endl;

            in.seekg(dex + 0x23);
            in.read((char*)(&temp_rgnd.pan), sizeof(uint8_t));
            if (log) cout << "Pan: " << int(temp_rgnd.pan) << endl;

            in.seekg(dex + 0x34);
            in.read((char*)(&temp_rgnd.sample_id), sizeof(uint32_t));
            if (log) cout << "Sample ID: " << temp_rgnd.sample_id << endl;

            rgnd_data.push_back(temp_rgnd);
        }
        if (log) cout << endl;


        //Get sample data
        for (int w = 0; w < numsnd; ++w) {
            in.seekg(data_offset + sgxd_offset + wave_data[w].stream_offset);

            for (int s = 0; s < wave_data[w].stream_size; ++s) {
                in.get(buff);
                wave_data[w].data.push_back(buff);
            }

            if (log) cout << "Size of raw audio: 0x" << hex << wave_data[w].data.size() << dec << endl;
            if (wave_data[w].data.empty()) continue;

            //Convert sample data
            switch(wave_data[w].codec) {
                case PCM16BE:
                    wave_data[w].pcmle = convertPcmBE(wave_data[w].data);
                    if (log) cout << "Size of new file after BE to LE conversion: 0x" << hex << wave_data[w].pcmle.size() << dec << endl;
                    continue;

                case OGGVORB:
                    //wave_data[w].pcmle = oggVorbDecode(wave_data[w].data, wave_data[w].channels);
                    continue;

                case lPADPCM:
                case sPADPCM:
                    wave_data[w].pcmle = adpcmDecode(wave_data[w].data);
                    if (log) cout << "Size of new file after ADPCM conversion: 0x" << hex << wave_data[w].pcmle.size() << dec << endl;
                    continue;

                case ATRAC3p:
                    continue;

                case PS3_AC3:
                    continue;

                default:
                    wave_data[w].data.clear();
                    continue;
            }
        }
        if (log) cout << endl;


        if (!(_mkdir(newDir.c_str()))) cout << "Extract to \"" << newDir << "\"" << endl;
        cout << endl;


        //Apply names
        for (int n = 0; n < numsnd; ++n) {
            if (!wave_data[n].name.empty()) continue;

            string index = "000" + std::to_string(n);

            wave_data[n].name = bank_name + "_" + index.substr(index.size() - 3);
            if (log) cout << "Sample " << n << " name: " << wave_data[n].name << endl;
        }
        if (log) cout << endl;

        for (int defs = 0; defs < numrgd; ++defs) {
            cout << endl;
            string sample_path;

            int root = rgnd_data[defs].root_key,
                low = rgnd_data[defs].range_low,
                high = rgnd_data[defs].range_high,
                used,
                tune = rgnd_data[defs].correction,
                pan = rgnd_data[defs].pan,
                id = rgnd_data[defs].sample_id;

            if (wave_data[id].codec != PCM16BE &&
                wave_data[id].codec != lPADPCM &&
                wave_data[id].codec != sPADPCM) continue;
            else if (wave_data[id].pcmle.empty()) continue;

            if (low == high) used = high;
            else if (low == 0x00 && high == 0x7F) used = root - 0x12;
            else if (high - low > 30 && high == 0x7F) used = low;
            else if (high - low > 30 && low == 0x00) used = high;
            else used = high;


            uint32_t fSize,
                     fmtSize = 0x10,
                     sampRate = wave_data[id].sample_rate,
                     byteRate,
                     wavSize = wave_data[id].pcmle.size() * 0x02;
            uint16_t audFmt = 0x01,
                     channels = wave_data[id].channels,
                     blockAlign,
                     bps = 0x10;


            fSize = 0x20 + wavSize;
            if (log) cout << "Final file size: " << fSize << endl;


            if (log) cout << "Old sample rate: " << sampRate << endl;

            //Calculate new frequency...
            //targetFrequency = rootFrequency * (2 ^ (1/12)) ^ (targetNote + (rootNoteTune/100) - rootNote)
            sampRate = std::round(sampRate * pow(pow(2.00, (1.00/12)), double(used + (tune / 100.00) - root)));
            blockAlign = channels * (bps/8);
            byteRate = sampRate * blockAlign;

            if (log) cout << "New sample rate: " << sampRate << endl;
            if (log) cout << "Block align: " << blockAlign << endl;
            if (log) cout << "Byte rate: " << byteRate << endl;
            if (log) cout << "Channels: " << channels << endl;


            sample_path = newDir + "/" + wave_data[id].name;
            sample_path += "_R" + std::to_string(root);
            sample_path += "_C" + std::to_string(tune);
            sample_path += "_K" + std::to_string(used);
            sample_path += (pan < 0) ? "_L" : (pan > 0) ? "_R" : "_M";
            sample_path += ".wav";
            sample_path.erase(remove(sample_path.begin(), sample_path.end(), '\"'), sample_path.end());

            ofstream wavFile(sample_path, ios::binary);
            if (!wavFile.is_open()) {
                cerr << "Unable to write to " << sample_path << " . . ." << endl;
                continue;
            }

            wavFile.write(RIFF, sizeof(uint32_t));
            wavFile.write((const char*)(&fSize), sizeof(uint32_t));
            wavFile.write(rWAVE, sizeof(uint32_t));
            wavFile.write(fmt_, sizeof(uint32_t));
            wavFile.write((const char*)(&fmtSize), sizeof(uint32_t));
            wavFile.write((const char*)(&audFmt), sizeof(uint16_t));
            wavFile.write((const char*)(&channels), sizeof(uint16_t));
            wavFile.write((const char*)(&sampRate), sizeof(uint32_t));
            wavFile.write((const char*)(&byteRate), sizeof(uint32_t));
            wavFile.write((const char*)(&blockAlign), sizeof(uint16_t));
            wavFile.write((const char*)(&bps), sizeof(uint16_t));
            wavFile.write(DATA, sizeof(uint32_t));
            wavFile.write((const char*)(&wavSize), sizeof(uint32_t));
            wavFile.write((const char*)(wave_data[id].pcmle.data()), wavSize);

            wavFile.close();
            cout << "Successfully wrote to " << sample_path << endl;

            if (!successful) successful = true;
        }

        cout << "\n\n";
    }

    return successful;
}


int main(int argc, char *argv[]) {
    bool log = false;

    for (int i = 1; i < argc; ++i) {
        string source = argv[i];
        source.erase(remove(source.begin(), source.end(), '\"'), source.end());

        if (source == "-d") {
            log = true;
            continue;
        }
        else if (source[0] == '-') {
            cerr << "Unknown option " << source << " . . ." << endl;
            continue;
        }


        _chdir((source.substr(0, source.find_last_of("\\/"))).c_str());

        source = source.substr(source.find_last_of("\\/") + 1);
        if (log) cout << "Current file: " << source << endl;

        ifstream sgd_file(source, ios::binary);
        if (!sgd_file.is_open()) {
            cerr << "Unable to open " << source << " . . ." << endl;
            continue;
        }
        else if (!getSamples(sgd_file, 0x00, log)) {
            cerr << "No sound banks found in " << source << " . . ." << endl;
        }
    }

    return 0;
}
