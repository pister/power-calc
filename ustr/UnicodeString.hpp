//
//  UnicodeString.hpp
//  power_calc
//
//  Created by Songli Huang on 2018/1/20.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#ifndef UnicodeString_hpp
#define UnicodeString_hpp

#include <cstdint>
#include <cstddef>

#define BOM_LITTEL_ENDIAN   0xFFFE
#define BOM_BIG_ENDIAN      0xFEFF
#define BOM_DEFAULT      BOM_LITTEL_ENDIAN


// 2-bytes unicode UCS2
// not support all utf-8 string
class UnicodeString {
public:
    UnicodeString();
    UnicodeString(uint16_t bom, const uint16_t* src);
    UnicodeString(uint16_t bom, const uint16_t* src, size_t len);
    UnicodeString(const UnicodeString& src);
    ~UnicodeString();
    UnicodeString& operator=(const UnicodeString& src);
    static UnicodeString* decodeByUTF8(const char* bytes, size_t len);
    static UnicodeString* decodeByGBK(const char* bytes, size_t len);
    const char* encodeToUTF8(char* bytes, size_t pos, size_t len);
    const char* encodeToGBK(char* bytes, size_t pos, size_t len);
    uint16_t getChar(size_t pos) const {
        return m_data[pos];
    }
    size_t getLength() const {
        return m_length;
    }
private:
    void assign(uint16_t bom, const uint16_t* src, size_t len);
private:
    uint16_t m_bom;
    uint16_t* m_data;
    size_t m_length;
};
#endif /* UnicodeString_hpp */
