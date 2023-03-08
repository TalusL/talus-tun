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
        InfoL<< toolkit::SocketHelper::get_peer_ip()<<":"<<toolkit::SocketHelper::get_peer_port();
    }
    void onRecv(const Buffer::Ptr &buffer) override {
    }
    void onError(const SockException &err) override{
        WarnL << err.what();
    }
    //每隔一段时间触发，用来做超时管理
    void onManager() override{
    }
};

#endif //TALUSTUN_WSSESSION_H
