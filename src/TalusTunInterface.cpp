
#include <random>
#include "TalusTunInterface.h"
#include "Util/mini.h"
#include "Config.h"
#include <Util/util.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "Process.h"


TalusTunInterface::~TalusTunInterface() {
    Stop();
}

void TalusTunInterface::Config(const TalusTunCfg &cfg) {
    //Set CFG
    SetIPv4(cfg.ipv4Addr);
    SetIPv4Netmask(cfg.ipv4NetMark);
    SetMTU(cfg.mtu);
}

bool TalusTunInterface::Start() {
    Stop();
    m_running = true;
    m_listenThread = std::thread([this](){
        while (m_running){
            auto pkt = Receive();
            if(pkt&&pkt->size()){
                m_poller->async([this,pkt](){
                    Dispatch(pkt);
                });
            }
        }
    });
    m_listenThread.detach();
    return true;
}

void TalusTunInterface::Stop() {
    m_running = false;
    if(m_listenThread.joinable()){
        m_listenThread.join();
    }
}


TunIO::TunIO() {
    std::string tunName = mINI::Instance()[CONFIG_NAME];
    this->name(tunName);
    m_deviceName = this->name();
}

void TunIO::SetIPv4(const std::string &ipv4) {
    struct in_addr addr{};

    if (!inet_pton(AF_INET, ipv4.c_str(), &addr)) {
        std::ostringstream what;
        what << "--- Invalid IPv4 address (" << ipv4 << ") for ";
        what << this->m_deviceName << "." << std::endl;
        throw std::invalid_argument(what.str());
    }

    this->m_ipv4 = ipv4;
}



void TunIO::SetMTU(uint mtu) {
    std::ostringstream what;

    if (mtu < ETH_HLEN) {
        what << "--- MTU " << mtu << " too small (< " << ETH_HLEN << ").";
        what << std::endl;
        throw std::invalid_argument(what.str());
    }

    if (mtu > 65536) {
        what << "--- MTU " << mtu << " too large (> 65536)." << std::endl;
        throw std::invalid_argument(what.str());
    }

    this->m_mtu = mtu;
}

void TunIO::Up() {
    ip(m_ipv4,m_netmask);
    this->up();
    std::string upScript = mINI::Instance()[CONFIG_UP_SCRIPT];
    if(!upScript.empty()){
        Process process;
        std::string log;
        process.run(upScript,log);
    }
}


void TunIO::Send(const toolkit::Buffer::Ptr& pkt) {
    write(pkt->data(),pkt->size());
}

TunIO::~TunIO() {
}

void TunIO::SetIPv4Netmask(uint netmask) {
    m_netmask = netmask;
}

void TunIO::Down() {
    this->down();
    std::string downScript = mINI::Instance()[CONFIG_DOWN_SCRIPT];
    if(!downScript.empty()){
        Process process;
        std::string log;
        process.run(downScript,log);
    }
}

toolkit::BufferRaw::Ptr TunIO::Receive() {
    static std::vector<uint8_t> vector(1600);
    ulong size = 0;
    if((size = read(vector.data(),vector.size()))== -1){
        return{};
    }
    auto pkt = toolkit::BufferRaw::create();
    pkt->assign(reinterpret_cast<const char *>(vector.data()), size);
    return pkt;
}

bool TalusTunInterface::Dispatch(const toolkit::Buffer::Ptr& pkt){

    auto getIp = [](uint32_t n){
        std::string ip = StrPrinter
                <<std::to_string(*((uint8_t*)(&n)+0))<<"."
                <<std::to_string(*((uint8_t*)(&n)+1))<<"."
                <<std::to_string(*((uint8_t*)(&n)+2))<<"."
                <<std::to_string(*((uint8_t*)(&n)+3));
        return ip;
    };

    uint32_t addr = *((uint32_t*)(pkt->data()+16));
    for (const auto &router: m_routerRules){
        auto mask = ~htonl(router.second->m_mask>=32?0:(0xFFFFFFFF >> router.second->m_mask));
        auto raddr = addr&mask;
        auto laddr = router.second->m_addr&mask;

//            DebugL<< getIp(mask) <<" "<<getIp(addr)<<" "<<getIp(router.second->m_addr)<<" "<<getIp(raddr)<<" == "<<getIp(laddr);
        if(raddr==laddr){
            if(router.second->m_cb){
                router.second->m_cb(pkt);
            }
            return true;
        }
    }
    return false;
}

bool TalusTunInterface::isOnLinkPkt(const Buffer::Ptr &buf) {
    return m_routerRules.find(*((uint32_t*)buf->data()+16) ) != m_routerRules.end();
}

void TalusTunInterface::Send(const Buffer::Ptr &pkt) {
    m_poller->async([this,pkt]() {
//        if (isOnLinkPkt(pkt)) {
//            Dispatch(pkt);
//            return;
//        }
        TunIO::Send(pkt);
    });
}
