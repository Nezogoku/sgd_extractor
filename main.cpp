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
#include "bit_byte.hpp"
#include "decode.hpp"
#include "sgxd_types.hpp"
#include "wave_types.hpp"

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
        uint32_t chunk, numsnd = 0, numrgd = 0, numnam = 0,
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
        data_length %= 0x80000000;

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

        /*
        if (!hasDefinitions) {
            cerr << "This file isn't a sound bank, so can't be parsed at the moment . . ." << endl;
            continue;
        }
        */
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
            in.read((char*)(&temp_wave.sample_loopstart), sizeof(uint32_t));

            in.seekg(dex + 0x28);
            in.read((char*)(&temp_wave.sample_loopend), sizeof(uint32_t));

            in.seekg(dex + 0x2C);
            in.read((char*)(&temp_wave.stream_size), sizeof(uint32_t));

            in.seekg(dex + 0x30);
            in.read((char*)(&temp_wave.stream_offset), sizeof(uint32_t));

            in.seekg(dex + 0x34);
            in.read((char*)(&temp_wave.stream_size_full), sizeof(uint32_t));

            if (temp_wave.sample_loopstart < 0 &&
                temp_wave.sample_loopend   < 0) temp_wave.is_looped = false;
            else temp_wave.is_looped = true;

            temp_wave.is_bank = false;

            wave_data.push_back(temp_wave);
        }


        //Get rgnd data if exists
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

            wave_data[temp_rgnd.sample_id].is_bank = true;

            rgnd_data.push_back(temp_rgnd);
        }
        if (hasDefinitions && log) cout << endl;


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
                    if (log) cout << "Size of new data after PCM16BE to PCM16LE: 0x" << hex << wave_data[w].pcmle.size() * 2 << dec << endl;
                    continue;

                case OGGVORB:
                    wave_data[w].pcmle = oggVorbDecode(wave_data[w].data, wave_data[w].channels);
                    if (log) cout << "Size of new data after OGGVORBIS to PCM16LE: 0x" << hex << wave_data[w].pcmle.size() * 2 << dec << endl;
                    continue;

                case cSADPCM:
                    wave_data[w].pcmle = adpcmDecode(wave_data[w].data,
                                                     wave_data[w].sample_loopstart, wave_data[w].sample_loopend, wave_data[w].is_looped);
                    if (log) cout << "Size of new data after PS ADPCM to PCM16LE: 0x" << hex << wave_data[w].pcmle.size() * 2 << dec << endl;
                    continue;

                case sSADPCM:
                    wave_data[w].pcmle = sAdpcmDecode(wave_data[w].data, wave_data[w].channels);
                    if (log) cout << "Size of new data after short PS ADPCM to PCM16LE: 0x" << hex << wave_data[w].pcmle.size() * 2 << dec << endl;
                    continue;

                case ATRAC3p:
                    //wave_data[w].pcmle = at3pDecode(wave_data[w].data, wave_data[w].channels, wave_data[w].sample_rate);
                    //if (log) cout << "Size of new data after AT3+ to PCM16LE: 0x" << hex << wave_data[w].pcmle.size() * 2 << dec << endl;
                    continue;

                case DOLAC_3:
                    int b_size;
                    if (wave_data[w].info_type == 0x80 || wave_data[w].info_type == 0x90) b_size = wave_data[w].info_value;
                    else b_size = 512;

                    wave_data[w].pcmle = ac3Decode(wave_data[w].data, wave_data[w].channels, b_size);
                    if (log) cout << "Size of new data after AC-3 to PCM16LE: 0x" << hex << wave_data[w].pcmle.size() * 2 << dec << endl;
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

        //Write wav files
        auto iter = rgnd_data.begin();
        for (int id = 0; id < numsnd;) {
            if (wave_data[id].pcmle.empty()) { id += 1; continue; }
            if (log) cout << endl;

            const uint32_t PCML_GUID[] = {0x00000001,
                                          0x0000, 0x0010,
                                          0x800000AA00389B71};
            string sample_path, sample_name;

            wave_header header;
            header.fmt_size = 0x10;
            header.fmt_codec = 0x01;
            header.fmt_channels = wave_data[id].channels;
            header.fmt_samplerate = wave_data[id].sample_rate;
            header.fmt_bps = 0x10;
            if (wave_data[id].channels > 2) {
                header.fmt_size += 0x24;
                header.fmt_sub_size = 0x22;
                header.fmt_sub_bps_blockalign = header.fmt_bps;

                switch(header.fmt_channels) {
                    case 3:
                        header.fmt_sub_mask = SPEAKER_FL | SPEAKER_FR |
                                              SPEAKER_FC;
                        break;

                    case 4:
                        header.fmt_sub_mask = SPEAKER_FL | SPEAKER_FR |
                                              SPEAKER_BL | SPEAKER_BR;
                        break;

                    case 5:
                        header.fmt_sub_mask = SPEAKER_FL | SPEAKER_FR |
                                              SPEAKER_FC |
                                              SPEAKER_BL | SPEAKER_BR;
                        break;

                    case 6:
                        header.fmt_sub_mask = SPEAKER_FL | SPEAKER_FR |
                                              SPEAKER_FC | SPEAKER_LF |
                                              SPEAKER_BL | SPEAKER_BR;
                        break;

                    case 7:
                        header.fmt_sub_mask = SPEAKER_FL | SPEAKER_FR |
                                              SPEAKER_FC | SPEAKER_LF |
                                              SPEAKER_BC |
                                              SPEAKER_BL | SPEAKER_BR;
                        break;

                    case 8:
                        header.fmt_sub_mask = SPEAKER_FL | SPEAKER_FR |
                                              SPEAKER_FC | SPEAKER_LF |
                                              SPEAKER_SL | SPEAKER_SR |
                                              SPEAKER_BL | SPEAKER_BR;
                        break;

                    default:
                        continue;
                }

                header.fmt_sub_gui[0] = 0x01;
                header.fmt_sub_gui[1] = 0x00;
                header.fmt_sub_gui[2] = 0x10;
                header.fmt_sub_gui[3] = 0x800000AA00389B71;
            }
            header.data_size = wave_data[id].pcmle.size() * 0x02;

            sample_name = wave_data[id].name;

            iter = std::find_if(iter, rgnd_data.end(), [&] (rgnd_def const &r) { return r.sample_id == id; });
            if (wave_data[id].is_bank) {
                if (iter == rgnd_data.end()) {
                    id += 1;
                    iter = rgnd_data.begin();
                    continue;
                }

                int def = iter - rgnd_data.begin(),
                    root = rgnd_data[def].root_key,
                    low = rgnd_data[def].range_low,
                    high = rgnd_data[def].range_high,
                    tune = rgnd_data[def].correction,
                    pan = rgnd_data[def].pan,
                    used;

                if (low == high) used = high;
                else if (low == 0x00 && high == 0x7F) used = root - 0x12;
                else if (high - low > 30 && high == 0x7F) used = low;
                else if (high - low > 30 && low == 0x00) used = high;
                else used = high;

                if (log) cout << "Old sample rate: " << header.fmt_samplerate << endl;

                //Calculate new frequency...
                //targetFrequency = rootFrequency * (2 ^ (1/12)) ^ (targetNote + (rootNoteTune/100) - rootNote)
                header.fmt_samplerate = std::round(header.fmt_samplerate * pow(pow(2.00, (1.00/12)), double(used + (tune / 100.00) - root)));


                sample_name += "_R" + std::to_string(root);
                sample_name += "_C" + std::to_string(tune);
                sample_name += "_K" + std::to_string(used);
                sample_name += (pan < 0) ? "_L" : (pan > 0) ? "_R" : "_M";
            }

            header.fmt_blockalign = header.fmt_channels * (header.fmt_bps/8);
            header.fmt_byterate = header.fmt_samplerate * header.fmt_blockalign;

            //Calculate sample period...
            //smplPeriod = (1/rootFrequency) * 1000000000
            header.smpl_period = (1.00/header.fmt_samplerate) * 1000000000;

            //Calculate sample tuning
            //fractToHex((tune/100.00), smplTune);

            //Determine number of loops
            if (wave_data[id].is_looped) {
                header.smpl_amount_loops += 0x01;
                header.smpl_loop_start = wave_data[id].sample_loopstart;
                header.smpl_loop_end = wave_data[id].sample_loopend;

                if (header.smpl_loop_start < 0) header.smpl_loop_start = 0;
                if (header.smpl_loop_end < 0) header.smpl_loop_end = header.data_size - 1;
            }

            header.smpl_size += (header.smpl_amount_loops * 0x18);

            if (log) cout << "Sample rate: " << header.fmt_samplerate << endl;
            if (log) cout << "Block align: " << header.fmt_blockalign << endl;
            if (log) cout << "Byte rate: " << header.fmt_byterate << endl;
            if (log) cout << "Channels: " << header.fmt_channels << endl;
            if (log) cout << "Sample period: " << header.smpl_period << endl;
            if (log) cout << "Sample pressed key with root 60: " << header.smpl_key << endl;
            if (log) cout << "Sample fractional semi-tuning: " << header.smpl_tune << "/100" << endl;
            if (log) cout << "Is looped: " << ((wave_data[id].is_looped) ? "TRUE" : "FALSE") << endl;
            if (log && wave_data[id].is_looped) {
                cout << "Number loops: " << header.smpl_amount_loops << endl;
                cout << "Loop start: " << header.smpl_loop_start << endl;
                cout << "Loop end: " << header.smpl_loop_end << endl;
            }

            header.file_size = 0x24 + header.fmt_size + header.smpl_size + header.data_size;
            if (log) cout << "Final file size: " << header.file_size << endl;


            sample_path = newDir + "/" + sample_name + ".wav";
            sample_path.erase(remove(sample_path.begin(), sample_path.end(), '\"'), sample_path.end());

            ofstream wavFile(sample_path, ios::binary);
            if (!wavFile.is_open()) {
                cerr << "Unable to write to " << sample_path << " . . ." << endl;
                continue;
            }

            try {
                wavFile.write(header.riff, sizeof(uint32_t));
                wavFile.write((const char*)(&header.file_size), sizeof(uint32_t));
                wavFile.write(header.wave, sizeof(uint32_t));
                wavFile.write(header.fmt_, sizeof(uint32_t));
                wavFile.write((const char*)(&header.fmt_size), sizeof(uint32_t));
                wavFile.write((const char*)(&header.fmt_codec), sizeof(uint16_t));
                wavFile.write((const char*)(&header.fmt_channels), sizeof(uint16_t));
                wavFile.write((const char*)(&header.fmt_samplerate), sizeof(uint32_t));
                wavFile.write((const char*)(&header.fmt_byterate), sizeof(uint32_t));
                wavFile.write((const char*)(&header.fmt_blockalign), sizeof(uint16_t));
                wavFile.write((const char*)(&header.fmt_bps), sizeof(uint16_t));
                if (wave_data[id].channels > 2) {
                    wavFile.write((const char*)(&header.fmt_sub_size), sizeof(uint16_t));
                    wavFile.write((const char*)(&header.fmt_sub_bps_blockalign), sizeof(uint16_t));
                    wavFile.write((const char*)(&header.fmt_sub_mask), sizeof(uint32_t));
                    wavFile.write((const char*)(&header.fmt_sub_gui[0]), sizeof(uint32_t));
                    wavFile.write((const char*)(&header.fmt_sub_gui[1]), sizeof(uint16_t));
                    wavFile.write((const char*)(&header.fmt_sub_gui[2]), sizeof(uint16_t));
                    wavFile.write((const char*)(&header.fmt_sub_gui[3]), sizeof(uint64_t));
                    wavFile.write((const char*)(&header.fmt_sub_padding), 0x0C);
                }
                wavFile.write(header.smpl, sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_size), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_manufacturer), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_product), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_period), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_key), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_tune), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_smpte_format), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_smpte_offset), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_amount_loops), sizeof(uint32_t));
                wavFile.write((const char*)(&header.smpl_amount_extra), sizeof(uint32_t));
                if (wave_data[id].is_looped) {
                    wavFile.write((const char*)(&header.smpl_loop_id), sizeof(uint32_t));
                    wavFile.write((const char*)(&header.smpl_loop_type), sizeof(uint32_t));
                    wavFile.write((const char*)(&header.smpl_loop_start), sizeof(uint32_t));
                    wavFile.write((const char*)(&header.smpl_loop_end), sizeof(uint32_t));
                    wavFile.write((const char*)(&header.smpl_loop_resolution), sizeof(uint32_t));
                    wavFile.write((const char*)(&header.smpl_loop_number), sizeof(uint32_t));
                }
                wavFile.write(header.data, sizeof(uint32_t));
                wavFile.write((const char*)(&header.data_size), sizeof(uint32_t));
                wavFile.write((const char*)(wave_data[id].pcmle.data()), header.data_size);

                cout << "Successfully wrote to " << sample_path << endl;
                successful = true;
            }
            catch (ofstream::failure &e) {
                cerr << "Unable to write to " << sample_path << endl;
                cerr << e.what() << endl;
            }
            wavFile.close();


            if (iter == rgnd_data.end()) { id += 1; iter = rgnd_data.begin(); }
            else iter += 1;
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
        getSamples(sgd_file, 0x00, log);
    }

    return 0;
}
