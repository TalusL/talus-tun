//
// Created by liangzhuohua on 2023/3/10.
//

#ifndef TALUSTUN_DATAFILTER_H
#define TALUSTUN_DATAFILTER_H
#include "Network/Buffer.h"
#include "Util/mini.h"
#include "Config.h"
#include "Util/MD5.h"

using namespace std;
using namespace toolkit;

class DataFilter{
public:
    static void Filter(const Buffer::Ptr &buf){
        static string passStr = mINI::Instance()[CONFIG_PASS];
        static string passMd5 = MD5(passStr).rawdigest();
        static auto *hp = reinterpret_cast<uint64_t *>(passMd5.data());
        static auto *lp = reinterpret_cast<uint64_t *>(passMd5.data() + sizeof(uint64_t));
        static uint64_t pass = buf->size()%2?*hp:*lp;
        auto * data = reinterpret_cast<uint8_t *>(buf->data());
        for (int offset = 0; offset + sizeof(uint64_t) < buf->size(); offset+= sizeof(uint64_t)){
            *((uint64_t*)(data+offset)) = *((uint64_t*)(data+offset))^pass;
        }
    }
};

#endif //TALUSTUN_DATAFILTER_H
