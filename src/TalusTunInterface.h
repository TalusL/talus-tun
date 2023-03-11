

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
#include <Poller/EventPoller.h>

using namespace toolkit;

using DataCallback = std::function<void(const toolkit::Buffer::Ptr&)>;
class TunIO:public tuntap::tun{
public:
    explicit TunIO();
    ~TunIO();
    void SetIPv4(const std::string& ipv4);
    void SetIPv4Netmask(uint netmask);
    void SetMTU(uint mtu);
    void Up();
    void Down();
    virtual void Send(const toolkit::Buffer::Ptr&);
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
    void AddDispatcher(const std::string& addr, const Dispatcher::Ptr& dispatcher){
        sockaddr_storage taddr;
        SockUtil::getDomainIP(addr.c_str(), 0, taddr, AF_INET, SOCK_STREAM, IPPROTO_TCP);
        m_routerRules[((sockaddr_in*)&taddr)->sin_addr.s_addr] = dispatcher;
    }
    //Dispatch pkt
    bool Dispatch(const toolkit::Buffer::Ptr& pkt);

    void Send(const Buffer::Ptr &ptr) override;


private:
    std::mutex m_routerRulesMtx;
    std::map<uint,Dispatcher::Ptr> m_routerRules;
    //select loop thread
    std::thread m_listenThread;
    //running flag
    volatile bool m_running = false;
    //poller
    EventPoller::Ptr m_poller = EventPollerPool::Instance().getPoller();
};


#endif //TALUSTUN_TALUSTUNINTERFACE_H
