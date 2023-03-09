//
// Created by Wind on 2023/3/8.
//

#ifndef TALUSTUN_TUNWSSESSION_H
#define TALUSTUN_TUNWSSESSION_H

#include "Util/logger.h"
#include "Http/WebSocketSession.h"
#include "TunAddrAlloc.h"

using namespace std;
using namespace toolkit;
using namespace mediakit;


class TunWsSession : public Session {
public:
    explicit TunWsSession(const Socket::Ptr &pSock) : Session(pSock){
    }
    ~TunWsSession() override= default;

    void attachServer(const Server &server) override{
        InfoL<< toolkit::SocketHelper::get_peer_ip()<<":"<<toolkit::SocketHelper::get_peer_port()<<" ws connect!";
        m_ticker.resetTime();
    }
    void onRecv(const Buffer::Ptr &buffer) override {
        InfoL<<"recv from ws:"<<buffer->size();
        const std::string configCmd = "GetTalusTunConfig";
        if(buffer->size()>=configCmd.length()&& strncmp(configCmd.c_str(),buffer->data(),configCmd.size())==0){
            static uint mask = 24;
            static uint mtu = 1450;

            m_config = true;
            string addr = TunAddrAlloc::Instance()->AllocAddr();
            m_addr = addr;
            string response = StrPrinter << "TalusTunConfig,"<< addr << "," << mask << "," << mtu;

            InfoL<<"Addr alloc remote: "<<toolkit::SocketHelper::get_peer_ip()<<":"<<toolkit::SocketHelper::get_peer_port()
                <<" <-> "<<addr << "/" << mask << "," << mtu;

            auto dispatcher = TalusTunInterface::Dispatcher::makeDispatcher(addr,32,[this](const toolkit::BufferRaw::Ptr& pkt){
                InfoL<<"recv from tun:"<<pkt->size();
                InfoL<<"send to ws:"<<pkt->size();
                SockSender::send(pkt->data(),pkt->size());
            });
            TalusTunInterface::Instance()->AddDispatcher(0,dispatcher);
            SockSender::send(response);
        }else{
            InfoL<<"send to tun:"<<buffer->size();
            TalusTunInterface::Instance()->Send(buffer);
        }
    }
    void onError(const SockException &err) override{
        WarnL << err.what();
        TunAddrAlloc::Instance()->releaseAddr(m_addr);
    }
    //每隔一段时间触发，用来做超时管理
    void onManager() override{
        if(!m_config&&m_ticker.elapsedTime()>10*1000){
            toolkit::SocketHelper::shutdown(SockException(Err_shutdown,"not config for long!"));
        }
    }
    bool m_config = false;
    Ticker m_ticker;
    string m_addr;
};

#endif //TALUSTUN_TUNWSSESSION_H
