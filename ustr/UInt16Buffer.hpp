//
//  UInt16Buffer.hpp
//  power_calc
//
//  Created by Songli Huang on 2018/1/20.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#ifndef UInt16Buffer_hpp
#define UInt16Buffer_hpp

#include <cstdint>
#include <cstddef>

class UInt16Buffer {
public:
    UInt16Buffer(size_t init_size = 128);
    ~UInt16Buffer();
    UInt16Buffer& operator=(const UInt16Buffer&);
    UInt16Buffer(const UInt16Buffer&);
    void add_uint16(uint16_t c, bool bigendian);
    void add_uint8(uint8_t c, bool bigendian);
    uint16_t* get_data() const {
        return m_data;
    }
    size_t get_pos() const {
        return m_pos;
    }
private:
    void grow_size();
    uint16_t *m_data;
    size_t m_pos;
    size_t m_len;
};

#endif /* UInt16Buffer_hpp */
