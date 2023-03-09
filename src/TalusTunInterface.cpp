
#include <random>
#include "TalusTunInterface.h"
#include <Util/util.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <arpa/inet.h>


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
                dispatch(pkt);
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
    std::mt19937 random(std::random_device{}());
    std::uniform_int_distribution<unsigned int> generator(0, 99);
    this->name("Talus"+ std::to_string(generator(random)));
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
}


void TunIO::Send(const toolkit::Buffer::Ptr& data) {
    write(data->data(),data->size());
}

TunIO::~TunIO() {
}

void TunIO::SetIPv4Netmask(uint netmask) {
    m_netmask = netmask;
}

void TunIO::Down() {
    this->down();
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
