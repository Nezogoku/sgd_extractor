#ifndef BIT_BYTE_HPP
#define BIT_BYTE_HPP

///Get variable little-endian int from array
static unsigned int getLeInt(char *src, int length) {
    int out = 0;
    for (int s = length; s > 0; --s) {
        out = (out << 8) | ((*(src + (s - 1))) & 0xFF);
    }
    return out;
}

///Get variable big-endian int from array
static unsigned int getBeInt(char *src, int length) {
    int out = 0;
    for (int s = 0; s < length; ++s) {
        out = (out << 8) | (*(src + s) & 0xFF);
    }
    return out;
}

///Compare variable char from array to char array
static bool cmpChar(char *src0, const char *src1, int length) {
    for (int s = 0; s < length; ++s) {
        if (src0[s] != src1[s]) return false;
    }
    return true;
}

///Reverse 32bit integer
static void setReverse(unsigned int &tmpInt) {
    unsigned int buffer = 0x00;
    for (int b = 0; b < 4; ++b) {
        buffer |= (unsigned char)((tmpInt >> (0x00 + (8 * b))) & 0xFF);
        if (b != 3) buffer <<= 8;
    }
    tmpInt = buffer;
}

///Convert double to hex
static void fractToHex(double tune, unsigned int &hex_tune) {
    while (tune != 0x00) {
        int buffer = std::floor(tune);

        hex_tune |= buffer;
        hex_tune <<= 0x04;

        tune -= buffer;
        tune *= 0x10;
    }
}

#endif
