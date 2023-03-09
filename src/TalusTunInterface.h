

#ifndef TALUSTUN_TALUSTUNINTERFACE_H
#define TALUSTUN_TALUSTUNINTERFACE_H
#include <utility>
#include <thread>
#include <functional>
#include <tuntap++.hh>
#include "Network/Buffer.h"
#include <map>
#include <netinet/in.h>
#include "Util/util.h"
#include "Network/sockutil.h"
#include "Util/logger.h"

using namespace toolkit;

using DataCallback = std::function<void(const toolkit::BufferRaw::Ptr&)>;
class TunIO:tuntap::tun{
public:
    explicit TunIO();
    ~TunIO();
    void SetIPv4(const std::string& ipv4);
    void SetIPv4Netmask(uint netmask);
    void SetMTU(uint mtu);
    void Up();
    void Down();
    void Send(const toolkit::Buffer::Ptr&);
    toolkit::BufferRaw::Ptr Receive();

private:
    std::string m_deviceName;
    std::string m_ipv4;
    uint m_netmask = 24;
    uint m_mtu = 1420;
};

class TalusTunInterface :public TunIO {
public:
    class TalusTunCfg{
    public:
        TalusTunCfg(std::string ipv4Addr, uint ipv4NetMark,uint mtu)
                    : ipv4Addr(std::move(ipv4Addr)),
                    ipv4NetMark(ipv4NetMark),
                    mtu(mtu){}

        std::string ipv4Addr;
        uint ipv4NetMark = 24;
        uint mtu = 1400;
    };
    class Dispatcher{
    public:
        using Ptr = std::shared_ptr<Dispatcher>;
        static Dispatcher::Ptr makeDispatcher(const std::string& addr,uint mask,const DataCallback&  cb){
            sockaddr_storage taddr;
            SockUtil::getDomainIP(addr.c_str(), 0, taddr, AF_INET, SOCK_STREAM, IPPROTO_TCP);

            auto d = std::make_shared<Dispatcher>();
            d->m_addr = ((sockaddr_in*)&taddr)->sin_addr.s_addr;
            d->m_mask = mask;
            d->m_cb = cb;
            return d;
        }
        Dispatcher() = default;
        uint32_t m_addr{};
        uint m_mask{};
        DataCallback m_cb;
    };


    void Config(const TalusTunCfg& cfg);
    ~TalusTunInterface();
    //Listen on interface
    bool Start();
    //Stop
    void Stop();
    //Instance
    static TalusTunInterface * Instance(){
        static TalusTunInterface talusTunInterface;
        return &talusTunInterface;
    }
    //AddDispatcher
    void AddDispatcher(uint index, const Dispatcher::Ptr& dispatcher){
        m_routerRules[index] = dispatcher;
    }

private:
    std::mutex m_routerRulesMtx;
    std::map<uint,Dispatcher::Ptr> m_routerRules;
    //select loop thread
    std::thread m_listenThread;
    //running flag
    volatile bool m_running = false;
    //dispatch pkt
    void dispatch(const toolkit::BufferRaw::Ptr& pkt){

        auto getIp = [](uint32_t n){
            std::string ip = StrPrinter
                    <<std::to_string(*((uint8_t*)(&n)+0))<<"."
                    <<std::to_string(*((uint8_t*)(&n)+1))<<"."
                    <<std::to_string(*((uint8_t*)(&n)+2))<<"."
                    <<std::to_string(*((uint8_t*)(&n)+3));
            return ip;
        };

        std::lock_guard<std::mutex> lck(m_routerRulesMtx);
        uint32_t addr = *((uint32_t*)(pkt->data()+16));
        for (const auto &router: m_routerRules){
            auto mask = ~htonl(router.second->m_mask>=32?0:(0xFFFFFFFF >> router.second->m_mask));
            auto raddr = addr&mask;
            auto laddr = router.second->m_addr&mask;

            DebugL<< getIp(mask) <<" "<<getIp(addr)<<" "<<getIp(router.second->m_addr)<<" "<<getIp(raddr)<<" == "<<getIp(laddr);
            if(raddr==laddr){
                if(router.second->m_cb){
                    router.second->m_cb(pkt);
                }
                return;
            }
        }
    }
};


#endif //TALUSTUN_TALUSTUNINTERFACE_H
