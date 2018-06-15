//
//  UInt16Buffer.cpp
//  power_calc
//
//  Created by Songli Huang on 2018/1/20.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#include "UInt16Buffer.hpp"
#include <cstring>


UInt16Buffer::UInt16Buffer(size_t init_size) : m_pos(0), m_len(init_size) {
    m_data = new uint16_t[init_size];
}

void UInt16Buffer::grow_size() {
    size_t new_size = (size_t)(m_len + 2) * 1.5;
    uint16_t *new_data = new uint16_t[new_size];
    memcpy(new_data, m_data, m_pos);
    
    m_len = new_size;
    delete[] m_data;
    m_data = new_data;
}

void UInt16Buffer::add_uint8(uint8_t c, bool bigendian) {
    add_uint16(c, bigendian);
}

void UInt16Buffer::add_uint16(uint16_t c, bool bigendian) {
    if (m_pos >= m_len) {
        grow_size();
    }
    if (!bigendian) {
        uint16_t tmp = (c & 0x0F);
        tmp = tmp << 8;
        tmp |= (((c >> 8) & 0x0F));
        m_data[m_pos++] = tmp;
    } else {
        m_data[m_pos++] = c;
    }
}

UInt16Buffer::~UInt16Buffer() {
    if (m_data) {
        delete[] m_data;
        m_data = NULL;
    }
}

