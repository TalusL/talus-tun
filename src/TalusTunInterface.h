

#ifndef TALUSTUN_TALUSTUNINTERFACE_H
#define TALUSTUN_TALUSTUNINTERFACE_H
#include <utility>
#include <viface/viface.hpp>
#include <viface/utils.hpp>
#include <thread>

class TalusTunInterface : public viface::VIface {
public:
    class TalusTunCfg{
    public:
        TalusTunCfg(std::string ipv4Addr, std::string ipv4NetMark,
                    std::string ipv4Broadcast, uint mtu,std::string mac)
                    : ipv4Addr(std::move(ipv4Addr)),
                    ipv4NetMark(std::move(ipv4NetMark)),
                    ipv4Broadcast(std::move(ipv4Broadcast)),
                    mtu(mtu),mac(std::move(mac)){}

        std::string ipv4Addr;
        std::string ipv4NetMark;
        std::string ipv4Broadcast;
        std::string mac;
        uint mtu = 1400;
    };
    explicit TalusTunInterface(class TalusTunCfg& cfg);
    ~TalusTunInterface();
    //Listen on interface
    bool Listen(const viface::dispatcher_cb& cb);
    //Stop
    void Stop();
    //
    static std::string GeneraMac();
private:
    //select loop thread
    std::thread m_listenThread;
    //running flag
    volatile bool m_running = false;
};


#endif //TALUSTUN_TALUSTUNINTERFACE_H
