
#include <random>
#include "TalusTunInterface.h"
#include <Util/util.h>
TalusTunInterface::~TalusTunInterface() {
    Stop();
}

TalusTunInterface::TalusTunInterface(TalusTunCfg &cfg):viface::VIface("Talus%d") {
    //Set CFG
    setIPv4(cfg.ipv4Addr);
    setIPv4Broadcast(cfg.ipv4Broadcast);
    setIPv4Netmask(cfg.ipv4NetMark);
    setMTU(cfg.mtu);
    setMAC(cfg.mac);
}

bool TalusTunInterface::Listen(const viface::dispatcher_cb &cb) {
    if(!isUp()){
        return false;
    }
    Stop();
    m_running = true;
    m_listenThread = std::thread([cb, this](){
        std::set<VIface *> interfaces = {this};
        viface::dispatch(interfaces,[cb,this](std::string const& name, uint id,
                std::vector<uint8_t>& packet) {
            return m_running&&cb(name,id,packet);
        });
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

std::string TalusTunInterface::GeneraMac() {
    //make random mac
    auto r = toolkit::trim(toolkit::hexmem(toolkit::makeRandStr(5, false).c_str(),5));
    toolkit::replace(r," ",":");
    return std::string("00:")+r;
}
