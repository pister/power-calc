//
//  io.cpp
//  power_calc
//
//  Created by Songli Huang on 2018/6/19.
//  Copyright © 2018年 Songli Huang. All rights reserved.
//

#include "lib_io.hpp"
#include <stdio.h>

typedef struct {
    FILE* fp;
    bool closed;
public:
    void close() {
        if (!closed) {
            fclose(fp);
            closed = true;
        }
    }
} Rt_FileData;

RtObject XFileClass::newObject(const std::string& filename, const std::string& mode) {
    return XFileClass::newObject(filename.c_str(), mode.c_str());
}

RtObject XFileClass::newObject(const char* filename, const char* mode) {
    FILE* fp = fopen(filename, mode);
    if (!fp) {
        return RtObject::Null;
    }
    Rt_FileData* fileData = new Rt_FileData;
    fileData->fp = fp;
    fileData->closed = false;
    return new XObject(instance(), fileData);
}

void XFileClass::on_destroying(XObject& instance) const {
    Rt_FileData* fd = (Rt_FileData*)instance.getData();
    if (fd) {
        fd->close();
        delete fd;
    }
}

RtObject XFileClass::__str__(XObject& instance, const std::vector<RtObject>& args) {
    Rt_FileData* fd = (Rt_FileData*)instance.getData();
    std::ostringstream oss;
    if (fd && !fd->closed) {
        oss<< "<file@" << (LONG64)fd->fp << ">";
    } else {
        oss<< "<closed file>";
    }
    return XStringClass::newObject(oss.str());
}

RtObject XFileClass::__close__(XObject& instance, const std::vector<RtObject>& args) {
    Rt_FileData* fd = (Rt_FileData*)instance.getData();
    if (fd) {
        fd->close();
    }
    return RtObject::Null;
}

// 读取一个字符并返回
RtObject XFileClass::__read_byte__(XObject& instance, const std::vector<RtObject>& args) {
    Rt_FileData* fd = (Rt_FileData*)instance.getData();
    if (!fd || fd->closed) {
        return RtObject::Null;
    }
    int c = fgetc(fd->fp);
    return RtObject::RtObject((LONG64)c);
}

// a,如果无参数，读取一个字节并返回（bytes类型）
// b,如果有1个参数，看参数类型。如果是整数类型，则表示需要读取的数量，如果是整数值是1，同a,
//   否则返回bytes对象，大小是读取的内容字节数
//   如果这个参数是bytes类型，则把读取的内容写入该bytes数据中，读取大小为该bytes的大小
RtObject XFileClass::__read__(XObject& instance, const std::vector<RtObject>& args) {
    Rt_FileData* fd = (Rt_FileData*)instance.getData();
    if (!fd || fd->closed) {
        return RtObject::Null;
    }
    size_t args_size = args.size();
    if (args_size > 0 && args.at(0).getType() == RtObject::RT_TYPE_OBJECT) {
        XObject* object = args.at(0).getObject();
        if (object == NULL) {
            return RtObject::Null;
        }
        if (object->getClass() != XBytesClass::instance()) {
            return RtObject::Null;
        }
        XBytesData* bytesData = (XBytesData*)object->getData();
        BYTE* data = bytesData->data;
        size_t len = bytesData->len;
        size_t read_size = fread(data, 1, len, fd->fp);
        return RtObject::RtObject((LONG64)read_size);
    } else {
        size_t buff_len;
        if (args_size == 0) {
            buff_len = 1;
        } else {
            buff_len = (size_t)args.at(0).getIntValue();
        }
  
        BYTE* data = new BYTE[buff_len];
        size_t read_size = fread(data, 1, buff_len, fd->fp);
        RtObject ret = XBytesClass::newObject(data, read_size);
        delete [] data;
        return ret;
    }
    return RtObject::Null;
}
