//
// Created by Wind on 2023/3/8.
//

#ifndef TALUSTUN_WSCLIENT_H
#define TALUSTUN_WSCLIENT_H

#include "Network/TcpClient.h"
#include "Http/WebSocketClient.h"

using namespace toolkit;
using namespace mediakit;
class WsClient : public TcpClient {
public:
    explicit WsClient(const EventPoller::Ptr &poller = nullptr){
    }
    ~WsClient() override = default;
    void SetCfgUrl(const std::string& url){
        m_cfgUrl = url;
    }
protected:
    void onRecv(const Buffer::Ptr &pBuf) override {
        const std::string configCmd = "TalusTunConfig";
        if(pBuf->size()>configCmd.length()&& strncmp(configCmd.c_str(),pBuf->data(),configCmd.size())==0){
            m_config = true;
            InfoL<<"Got tun config:"<<pBuf->toString();
        }
    }
    //被动断开连接回调
    void onErr(const SockException &ex) override {
        WarnL << ex.what();
        dynamic_cast<WebSocketClient<WsClient,WebSocketHeader::BINARY,false>*>(this)->startWebSocket(m_cfgUrl, 5);
    }
    //tcp连接成功后每2秒触发一次该事件
    void onManager() override {
        if(!m_config&&m_ticker.elapsedTime()>10*1000){
            shutdown(SockException(Err_shutdown,"not config for long!"));
        }
    }
    //连接服务器结果回调
    void onConnect(const SockException &ex) override{
        if(ex){
            DebugL << ex.what();
        }
        m_ticker.resetTime();
        SockSender::send("GetTalusTunConfig");
    }
    //数据全部发送完毕后回调
    void onFlush() override{
    }
    std::string m_cfgUrl;
    bool m_config = false;
    Ticker m_ticker;
};


#endif //TALUSTUN_WSCLIENT_H
