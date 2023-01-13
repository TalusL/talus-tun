

#ifndef TALUSTUN_TALUSTUNINTERFACE_H
#define TALUSTUN_TALUSTUNINTERFACE_H
#include <utility>
#include <thread>
#include <functional>
#include <tuntap++.hh>
#include "Network/Buffer.h"

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
    void Send(const toolkit::BufferRaw::Ptr&);
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
        int ipv4NetMark = 24;
        uint mtu = 1400;
    };
    explicit TalusTunInterface(class TalusTunCfg& cfg);
    ~TalusTunInterface();
    //Listen on interface
    bool Listen(const DataCallback& cb);
    //Stop
    void StopListen();

private:
    //select loop thread
    std::thread m_listenThread;
    //running flag
    volatile bool m_running = false;
};


#endif //TALUSTUN_TALUSTUNINTERFACE_H
