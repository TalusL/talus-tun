//
// Created by Wind on 2023/3/8.
//

#ifndef TALUSTUN_WSSESSION_H
#define TALUSTUN_WSSESSION_H

#include "Util/logger.h"
#include "Http/WebSocketSession.h"

using namespace std;
using namespace toolkit;
using namespace mediakit;


class WsSession : public Session {
public:
    explicit WsSession(const Socket::Ptr &pSock) : Session(pSock){
    }
    ~WsSession() override= default;

    void attachServer(const Server &server) override{
        InfoL<< toolkit::SocketHelper::get_peer_ip()<<":"<<toolkit::SocketHelper::get_peer_port()<<" ws connect!";
        m_ticker.resetTime();
    }
    void onRecv(const Buffer::Ptr &buffer) override {
        const std::string configCmd = "GetTalusTunConfig";
        if(buffer->size()>=configCmd.length()&& strncmp(configCmd.c_str(),buffer->data(),configCmd.size())==0){
//            m_config = true;
            SockSender::send("TalusTunConfig,192.168.122.222,24,1450");
        }
    }
    void onError(const SockException &err) override{
        WarnL << err.what();
    }
    //每隔一段时间触发，用来做超时管理
    void onManager() override{
        if(!m_config&&m_ticker.elapsedTime()>10*1000){
            toolkit::SocketHelper::shutdown(SockException(Err_shutdown,"not config for long!"));
        }
    }
    bool m_config = false;
    Ticker m_ticker;
};

#endif //TALUSTUN_WSSESSION_H
