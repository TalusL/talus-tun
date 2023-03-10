//
// Created by liangzhuohua on 2023/3/9.
//

#ifndef TALUSTUN_TUNADDRALLOC_H
#define TALUSTUN_TUNADDRALLOC_H

#include <map>
#include <mutex>
#include <string>
#include <regex>
#include "Config.h"

using namespace std;

class TunAddrAlloc {
public:
    //Instance
    static TunAddrAlloc * Instance(){
        static TunAddrAlloc tunAddrAlloc;
        return &tunAddrAlloc;
    }
    string AllocAddr(){
        static int startAddr = mINI::Instance()[CONFIG_ADDR_ALLOC_BEGIN];
        static int endAddr = mINI::Instance()[CONFIG_ADDR_ALLOC_END];
        static string ip = mINI::Instance()[CONFIG_TUN_NET_IP];
        if(m_currIndex<0){
            m_currIndex = startAddr;
        }
        smatch ipPrefix;
        if(!regex_match(ip,ipPrefix,regex(R"((\d+.\d+.\d+.)\d+)"))){
            ErrorL<<"config ip addr error! "<<ip;
            return "";
        }
        string prefix = ipPrefix.str(1);
        for(int i=m_currIndex;i<=endAddr;i++){
            lock_guard<mutex> lck(m_allocAddrMtx);
            if(m_allocAddr.find(i)==m_allocAddr.end()){
                m_currIndex = i;
                auto allocIp = prefix+ to_string(m_currIndex);
                m_allocAddr[m_currIndex] = allocIp;
                return allocIp;
            }
        }
        return "";
    }
    void releaseAddr(const string& addr){
        smatch ipPrefix;
        if(!regex_match(addr,ipPrefix,regex(R"(\d+.\d+.\d+.(\d+))"))){
            ErrorL<<"releaseAddr error! "<<addr;
            return;
        }
        auto i = ipPrefix.str(1);
        {
            lock_guard<mutex> lck(m_allocAddrMtx);
            m_allocAddr.erase(stoi(i));
            InfoL<<addr<<" released!";
        }
    }

protected:
    mutex m_allocAddrMtx;
    map<int,string> m_allocAddr;
    TunAddrAlloc() = default;
    int m_currIndex = -1;
};


#endif //TALUSTUN_TUNADDRALLOC_H
