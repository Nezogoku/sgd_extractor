#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <direct.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "decode.hpp"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

using namespace std;


#define SGXD 0x53475844
#define RGND 0x52474E44
#define SEQD 0x53455144
#define WAVE 0x57415645
#define NAME 0x4E414D45

#define PCM16BE 0x01
#define OGGVORB 0x02
#define lPADPCM 0x03
#define ATRAC3p 0x04
#define sPADPCM 0x05
#define PS3_AC3 0x06

#define RIFF "RIFF"
#define rWAVE "WAVE"
#define fmt_ "fmt "
#define DATA "data"


struct rgnd_def {
    uint32_t determinator;
    uint8_t range_low;
    uint8_t range_high;
    uint8_t root_key;
    uint32_t sample_id;
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
};

struct name_def {
    uint16_t nID;
    uint16_t nType;
    uint32_t nOffset;
};


int main(int argc, char *argv[]) {
    bool log = false;

    vector <string> sourceArg,
                    newDir;

    string bank_name;
    uint32_t working_offset, chunk, numsnd, numrgd, numnam,
             sgxd_offset, rgnd_offset, seqd_offset, wave_offset, name_offset,
             sgxd_length, rgnd_length, seqd_length, wave_length, name_length,
             sgxd_name_offset, sgxd_sample_offset,
             rgnd_data_offset, wave_data_offset, name_data_offset;

    for (int i = 1; i < argc; ++i) {
        string source = argv[i],
               direct;

        if (source == "-d") {
            log = true;
            continue;
        }

        source.erase(remove(source.begin(), source.end(), '\"'), source.end());
        direct = source.substr(0, source.find_last_of("\\/") + 1);

        sourceArg.push_back(source);
        newDir.push_back(direct);
    }


    for (int index = 0; index < sourceArg.size(); ++index) {
        bool hasSamples = false,
             hasNames = false,
             hasDefinitions = false;



        ifstream sgd_file(sourceArg[index], ios::binary);

        if (!sgd_file.is_open()) {
            cerr << "Unable to open " << sourceArg[index] << " . . ." << endl;
            continue;
        }


        working_offset = 0x00;
        sgd_file.seekg(working_offset);
        sgd_file.read(reinterpret_cast<char*>(&chunk), sizeof(uint32_t));

        if (htonl(chunk) != SGXD) {
            cerr << sourceArg[index] << " is not an sgd . . ." << endl;
            sgd_file.close();
            continue;
        }

        sgd_file.seekg(working_offset + 0x04);
        sgd_file.read(reinterpret_cast<char*>(&sgxd_name_offset), sizeof(uint32_t));

        sgd_file.seekg(working_offset + 0x08);
        sgd_file.read(reinterpret_cast<char*>(&sgxd_sample_offset), sizeof(uint32_t));

        sgd_file.seekg(working_offset + 0x0C);
        sgd_file.read(reinterpret_cast<char*>(&sgxd_length), sizeof(uint32_t));
        sgxd_length -= 0x80000000;

        working_offset += 0x10;

        while (!sgd_file.eof()) {
            sgd_file.seekg(working_offset);
            sgd_file.read(reinterpret_cast<char*>(&chunk), sizeof(uint32_t));

            if (htonl(chunk) == RGND) {
                if (log) cout << "RGND header found at position: 0x" << hex << working_offset << dec << endl;
                hasDefinitions = true;
                rgnd_offset = working_offset;

                sgd_file.seekg(working_offset + 0x04);
                sgd_file.read(reinterpret_cast<char*>(&rgnd_length), sizeof(uint32_t));

                sgd_file.seekg(working_offset + 0x0C);
                sgd_file.read(reinterpret_cast<char*>(&chunk), sizeof(uint32_t));
                rgnd_data_offset = (working_offset + 0x10) + (chunk * 0x08);
                if (log) cout << "RGND data found at position: 0x" << hex << rgnd_data_offset << dec << endl;

                numrgd = (rgnd_offset + 0x08 + rgnd_length) - rgnd_data_offset;
                numrgd = std::floor(double(numrgd) / 0x38);

                working_offset += (rgnd_length + 0x08);
                if (log) cout << endl;
            }
            else if (htonl(chunk) == SEQD) {
                if (log) cout << "SEQD header found at position: 0x" << hex << working_offset << dec << endl;
                seqd_offset = working_offset;

                sgd_file.seekg(working_offset + 0x04);
                sgd_file.read(reinterpret_cast<char*>(&seqd_length), sizeof(uint32_t));

                working_offset += (seqd_length + 0x08);
                if (log) cout << endl;
            }
            else if (htonl(chunk) == WAVE) {
                if (log) cout << "WAVE header found at position: 0x" << hex << working_offset << dec << endl;
                hasSamples = true;
                wave_offset = working_offset;

                sgd_file.seekg(working_offset + 0x04);
                sgd_file.read(reinterpret_cast<char*>(&wave_length), sizeof(uint32_t));

                sgd_file.seekg(working_offset + 0x0C);
                sgd_file.read(reinterpret_cast<char*>(&numsnd), sizeof(uint32_t));
                if (log) cout << "Number of samples: " << numsnd << endl;

                wave_data_offset = working_offset + 0x10;
                if (log) cout << "WAVE data found at position: 0x" << hex << wave_data_offset << dec << endl;

                working_offset += (wave_length + 0x08);
                if (log) cout << endl;
            }
            else if (htonl(chunk) == NAME) {
                hasNames = true;

                if (log) cout << "NAME header found at position: 0x" << hex << working_offset << dec << endl;
                wave_offset = working_offset;

                sgd_file.seekg(working_offset + 0x04);
                sgd_file.read(reinterpret_cast<char*>(&name_length), sizeof(uint32_t));

                sgd_file.seekg(working_offset + 0x0C);
                sgd_file.read(reinterpret_cast<char*>(&numnam), sizeof(uint32_t));
                if (log) cout << "Number of names: " << numnam << endl;

                name_data_offset = working_offset + 0x10;
                if (log) cout << "NAME data found at position: 0x" << hex << name_data_offset << dec << endl;

                working_offset += (name_length + 0x08);
                if (log) cout << endl;
                break;
            }
            else working_offset += 0x04;
        }
        if (log) cout << endl;

        if (!hasDefinitions) {
            cerr << sourceArg[index] << " isn't a sound bank, so can't be parsed at the moment. . ." << endl;
            sgd_file.close();
            continue;
        }
        if (!hasSamples) {
            cerr << "There might be no samples in " << sourceArg[index] << " . . ." << endl;
            sgd_file.close();
            continue;
        }
        if (!hasNames) {
            cerr << "No names in " << sourceArg[index] << " . . ." << endl;
            sgd_file.close();
            continue;
        }


        //Get name offsets of samples
        vector<name_def> name_definitions;

        for (int d = 0; d < numnam; ++d) {
            name_def temp;
            working_offset = name_data_offset + (d * 0x08);

            sgd_file.seekg(working_offset + 0x00);
            sgd_file.read(reinterpret_cast<char*>(&temp.nID), sizeof(uint16_t));
            if (log) cout << "Name ID: " << temp.nID << endl;

            sgd_file.seekg(working_offset + 0x02);
            sgd_file.read(reinterpret_cast<char*>(&temp.nType), sizeof(uint16_t));
            if (log) cout << "Name type: 0x" << hex << temp.nType << dec << endl;

            sgd_file.seekg(working_offset + 0x04);
            sgd_file.read(reinterpret_cast<char*>(&temp.nOffset), sizeof(uint32_t));
            if (log) cout << "Name offset: 0x" << hex << temp.nOffset << dec << endl;

            //Type 0x2001 denotes sample maybe?
            if (temp.nType >= 0x2000 && temp.nType < 0x3000) {
                name_definitions.push_back(temp);
                if (log) cout << "Sample name found at position: 0x" << hex << temp.nOffset << dec << endl;
            }
        }

        //Reorganize names
        for (int a = 0; a < name_definitions.size(); ++a) {
            for (int b = a + 1; b < name_definitions.size(); ++b) {
                if (name_definitions[a].nType > name_definitions[b].nType) {

                    name_def temp = name_definitions[a];
                    name_definitions[a] = name_definitions[b];
                    name_definitions[b] = temp;
                }
            }
        }
        for (int a = 0; a < name_definitions.size(); ++a) {
            for (int b = a + 1; b < name_definitions.size(); ++b) {
                if (name_definitions[a].nType == name_definitions[b].nType &&
                    name_definitions[a].nID > name_definitions[b].nID) {

                    name_def temp = name_definitions[a];
                    name_definitions[a] = name_definitions[b];
                    name_definitions[b] = temp;
                }
            }
        }
        if (log) cout << endl;

        //Get names of samples
        char buff;
        vector<string> sndnames;
        for (auto offset_list : name_definitions) {
            string sndname = "";

            sgd_file.seekg(offset_list.nOffset);

            do {
                sgd_file.get(buff);
                if (buff != 0x00) sndname += buff;
            } while (buff != 0x00);
            sndname = sndname.substr(0, sndname.find_last_of('.'));

            sndnames.push_back(sndname);
            if (log) cout << "Name ID: " << offset_list.nID << endl;
            if (log) cout << "Name type: 0x" << hex << offset_list.nType << dec << endl;
            if (log) cout << "Added name: " << sndname << endl;
        }
        if (log) cout << endl;


        //Get bank name
        bank_name = "";
        sgd_file.seekg(sgxd_name_offset);

        do {
            sgd_file.get(buff);
            if (buff != 0x00) bank_name += buff;
        } while (buff != 0x00);

        if (log) cout << "Bank name: " << bank_name << endl;
        newDir[index] += '@' + bank_name;
        if (log) cout << "Output directory: " << newDir[index] << endl;
        if (log) cout << endl;


        //Get wave data
        vector<wave_def> wave_data;
        for (int w = 0; w < numsnd; ++w) {
            wave_def temp_wave;

            working_offset = wave_data_offset + (w * 0x38);

            sgd_file.seekg(working_offset + 0x04);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.name_offset), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x08);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.codec), sizeof(uint8_t));

            sgd_file.seekg(working_offset + 0x09);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.channels), sizeof(uint8_t));

            sgd_file.seekg(working_offset + 0x0C);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.sample_rate), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x10);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.info_type), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x14);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.info_value), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x20);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.sample_size), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x24);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.loop_start), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x28);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.loop_end), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x2C);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.stream_size), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x30);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.stream_offset), sizeof(uint32_t));

            sgd_file.seekg(working_offset + 0x34);
            sgd_file.read(reinterpret_cast<char*>(&temp_wave.stream_size_full), sizeof(uint32_t));

            wave_data.push_back(temp_wave);
        }

        for (int check = 0; check < wave_data.size(); ++check) {
            if (log) cout << "Size of sample: 0x" << hex << wave_data[check].stream_size << dec << endl;

            if (wave_data[check].stream_size == 0x00) {
                if (check + 1 >= wave_data.size()) wave_data[check].stream_size = sgxd_length - wave_data[check].stream_offset;
                else wave_data[check].stream_size = wave_data[check + 1].stream_offset - wave_data[check].stream_offset;
            }

            if (log) cout << "New size of sample: 0x" << hex << wave_data[check].stream_size << dec << endl;
        }
        if (log) cout << endl;


        //Get rgnd data
        vector<rgnd_def> rgnd_data;
        for (int r = 0; r < numrgd; ++r) {
            rgnd_def temp_rgnd;

            working_offset = rgnd_data_offset + (r * 0x38);
            if (log) cout << "Current offset: 0x" << hex << working_offset << dec << endl;

            sgd_file.seekg(working_offset + 0x00);
            sgd_file.read(reinterpret_cast<char*>(&temp_rgnd.determinator), sizeof(uint32_t));
            if (log) cout << "Determinator: " << temp_rgnd.determinator << endl;

            sgd_file.seekg(working_offset + 0x18);
            sgd_file.read(reinterpret_cast<char*>(&temp_rgnd.range_low), sizeof(uint8_t));
            if (log) cout << "Lowest range: " << int(temp_rgnd.range_low) << endl;

            sgd_file.seekg(working_offset + 0x19);
            sgd_file.read(reinterpret_cast<char*>(&temp_rgnd.range_high), sizeof(uint8_t));
            if (log) cout << "Highest range: " << int(temp_rgnd.range_high) << endl;

            sgd_file.seekg(working_offset + 0x1C);
            sgd_file.read(reinterpret_cast<char*>(&temp_rgnd.root_key), sizeof(uint8_t));
            if (log) cout << "Root key: " << int(temp_rgnd.root_key) << endl;

            sgd_file.seekg(working_offset + 0x34);
            sgd_file.read(reinterpret_cast<char*>(&temp_rgnd.sample_id), sizeof(uint32_t));
            if (log) cout << "Sample ID: " << temp_rgnd.sample_id << endl;

            rgnd_data.push_back(temp_rgnd);
        }
        if (log) cout << endl;


        //Get sample data
        for (int w = 0; w < numsnd; ++w) {
            sgd_file.seekg(sgxd_sample_offset + wave_data[w].stream_offset);

            for (int s = 0; s < wave_data[w].stream_size; ++s) {
                char buff;
                sgd_file.get(buff);
                wave_data[w].data.push_back(buff);
            }

            if (log) cout << "Size of raw audio: 0x" << hex << wave_data[w].data.size() << dec << endl;

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
                    vector<char> ().swap(wave_data[w].data);
                    continue;
            }
        }
        if (log) cout << endl;

        sgd_file.close();


        if (!(_mkdir(newDir[index].c_str()))) cout << "Created directory" << endl;
        if (!(_chdir(newDir[index].c_str()))) cout << "Moved to directory" << endl;
        cout << endl;

        while (sndnames.size() < numsnd) {
            string tempsndnam = bank_name + '_' + to_string(sndnames.size());
            if (log) cout << "Added name: " << tempsndnam << endl;
            sndnames.push_back(tempsndnam);
        }
        if (log) cout << endl;

        for (int defs = 0; defs < numrgd; ++defs) {
            string sample_path;

            int root = rgnd_data[defs].root_key,
                low = rgnd_data[defs].range_low,
                high = rgnd_data[defs].range_high,
                used,
                id = rgnd_data[defs].sample_id;

            if (wave_data[id].codec != PCM16BE && wave_data[id].codec != lPADPCM && wave_data[id].codec != sPADPCM) continue;

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
            //targetFrequency = rootFrequency * (2 ^ (1/12)) ^ (targetNote - rootNote)
            sampRate = std::round(sampRate * pow(pow(2.00, (1.00/12)), double(used - root)));
            blockAlign = channels * (bps/8);
            byteRate = sampRate * blockAlign;

            if (log) cout << "New sample rate: " << sampRate << endl;
            if (log) cout << "Block align: " << blockAlign << endl;
            if (log) cout << "Byte rate: " << byteRate << endl;
            if (log) cout << "Channels: " << channels << endl;


            sample_path = sndnames[id] + "_R";
            sample_path += to_string(root) + "_K" + to_string(used) + ".wav";
            sample_path.erase(remove(sample_path.begin(), sample_path.end(), '\"'), sample_path.end());

            ofstream wavFile(sample_path.c_str(), ios::binary);

            if (!wavFile.is_open()) {
                cerr << "Unable to write to " << sample_path << " . . ." << endl;
                continue;
            }

            wavFile.write(RIFF, sizeof(uint32_t));
            wavFile.write(reinterpret_cast<const char*>(&fSize), sizeof(uint32_t));
            wavFile.write(rWAVE, sizeof(uint32_t));
            wavFile.write(fmt_, sizeof(uint32_t));
            wavFile.write(reinterpret_cast<const char*>(&fmtSize), sizeof(uint32_t));
            wavFile.write(reinterpret_cast<const char*>(&audFmt), sizeof(uint16_t));
            wavFile.write(reinterpret_cast<const char*>(&channels), sizeof(uint16_t));
            wavFile.write(reinterpret_cast<const char*>(&sampRate), sizeof(uint32_t));
            wavFile.write(reinterpret_cast<const char*>(&byteRate), sizeof(uint32_t));
            wavFile.write(reinterpret_cast<const char*>(&blockAlign), sizeof(uint16_t));
            wavFile.write(reinterpret_cast<const char*>(&bps), sizeof(uint16_t));
            wavFile.write(DATA, sizeof(uint32_t));
            wavFile.write(reinterpret_cast<const char*>(&wavSize), sizeof(uint32_t));
            wavFile.write(reinterpret_cast<const char*>(wave_data[id].pcmle.data()), wavSize);

            wavFile.close();
            cout << "Successfully wrote to " << sample_path << endl;
        }
    }
}
