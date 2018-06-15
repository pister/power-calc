//
//  UnicodeString.cpp
//  power_calc
//
//  Created by Songli Huang on 2018/1/20.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#include "UnicodeString.hpp"
#include "UInt16Buffer.hpp"
#include <cstring>
#include <string>

using std::string;

UnicodeString::UnicodeString() {
    assign(BOM_DEFAULT, NULL, 0);
}

UnicodeString::UnicodeString(uint16_t bom, const uint16_t* src) {
    size_t len = 0;
    const uint16_t *pc = src;
    while (*pc != 0) {
        pc++;
        len++;
    }
    assign(bom, src, len);
}

UnicodeString::UnicodeString(uint16_t bom, const uint16_t* src, size_t len) {
    assign(bom, src, len);
}

UnicodeString::UnicodeString(const UnicodeString& src) {
    assign(src.m_bom, src.m_data, src.m_length);
}

UnicodeString::~UnicodeString() {
    if (m_data) {
        delete[] m_data;
        m_data = NULL;
    }
}

UnicodeString& UnicodeString::operator=(const UnicodeString& src){
    if (m_data) {
         delete[] m_data;
    }
    assign(src.m_bom, src.m_data, src.m_length);
    return *this;
}

void UnicodeString::assign(uint16_t bom, const uint16_t* src, size_t len) {
    m_bom = bom;
    m_length = len;
    m_data = new uint16_t[m_length + 1];
    m_data[m_length] = 0;
    if (m_length > 0) {
        memcpy(m_data, src, m_length * sizeof(uint16_t));
    }
}



UnicodeString* UnicodeString::decodeByUTF8(const char* bytes, size_t len) {
    size_t pos = 0;
    if (len >= 3) {
        if (bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF) {
            // BOM
            pos = 3;
        }
    }
    // 7bit     0xxxxxxx
    // 11bit    110xxxxx 10xxxxxx
    // 16bit    1110xxxx 10xxxxxx 10xxxxxx
    // 21bit    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    // 26bit    111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    // 31bit    1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    uint16_t bom = BOM_DEFAULT;
    bool bigendian = (bom == BOM_BIG_ENDIAN);
    UInt16Buffer buf(len);
    while (pos < len) {
        char c = bytes[pos];
        if (c > 0) {
            // 7bit
            buf.add_uint8(c, bigendian);
            pos++;
            continue;
        }
        if (((c >> 5) & 0x07) == 6 ) {
            // match 110xxxxx
            char c2 = bytes[pos + 1];
            // check for 10xxxxxx
            if (((c2 >> 6) & 0x3) != 2) {
                throw std::string("bad utf8 string.");
            }
            char c1_data = c << 5;
            char c2_data = c2 << 6;
            char t = c1_data;
            t = t << 6;
            t |= c2_data;
            buf.add_uint16(t, bigendian);
            pos += 2;
            continue;
        }
        if(((c >> 4) & 0x0F) == 14 ) {
            // match 1110xxxx
            char c2 = bytes[pos + 1];
            char c3 = bytes[pos + 2];
            if (((c2 >> 6) & 0x3) != 2) {
                throw std::string("bad utf8 string.");
            }
            if (((c3 >> 6) & 0x3) != 2) {
                throw std::string("bad utf8 string.");
            }
            char c1_data = c << 4;
            char c2_data = c2 << 6;
            char c3_data = c3 << 6;
            uint16_t t = c1_data;
            t = t << 6;
            t |= c2_data;
            t = t << 6;
            t |= c3_data;
            buf.add_uint16(t, bigendian);
            pos += 3;
            continue;
        }
        if(((c >> 3) & 0x1F) == 30 ) {
            // match 11110xxx
            throw std::string("unsupport utf8 more than 2bytes");
        }
        if(((c >> 2) & 0x4F) == 62 ) {
            // match 111110xx
            throw std::string("unsupport utf8 more than 2bytes");
        }
        if(((c >> 1) & 0x8F) == 126 ) {
            // match 1111110x
            throw std::string("unsupport utf8 more than 2bytes");
        }
        throw std::string("bad utf8 string.");
    }
    return new UnicodeString(bom, buf.get_data(), buf.get_pos());
}

UnicodeString* UnicodeString::decodeByGBK(const char* bytes, size_t len) {
    return NULL;
}

const char* UnicodeString::encodeToUTF8(char* bytes, size_t pos, size_t offset) {
    return NULL;
}

const char* UnicodeString::encodeToGBK(char* bytes, size_t pos, size_t offset) {
    return NULL;
}
