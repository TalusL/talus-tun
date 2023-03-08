//
// Created by Wind on 2023/3/8.
//

#ifndef TALUSTUN_WSCLIENT_H
#define TALUSTUN_WSCLIENT_H

#include "Network/TcpClient.h"

using namespace toolkit;
class WsClient : public TcpClient {
public:
    explicit WsClient(const EventPoller::Ptr &poller = nullptr){
    }
    ~WsClient() override = default;
protected:
    void onRecv(const Buffer::Ptr &pBuf) override {

    }
    //被动断开连接回调
    void onErr(const SockException &ex) override {
        WarnL << ex.what();
    }
    //tcp连接成功后每2秒触发一次该事件
    void onManager() override {
    }
    //连接服务器结果回调
    void onConnect(const SockException &ex) override{
        DebugL << ex.what();
    }

    //数据全部发送完毕后回调
    void onFlush() override{
    }
};


#endif //TALUSTUN_WSCLIENT_H
