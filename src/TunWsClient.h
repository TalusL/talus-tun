//
// Created by Wind on 2023/3/8.
//

#ifndef TALUSTUN_TUNWSCLIENT_H
#define TALUSTUN_TUNWSCLIENT_H

#include "Network/TcpClient.h"
#include "Http/WebSocketClient.h"
#include "TalusTunInterface.h"
#include "DataFilter.h"

using namespace toolkit;
using namespace mediakit;
class TunWsClient : public TcpClient {
public:
    explicit TunWsClient(const EventPoller::Ptr &poller = nullptr){
    }
    ~TunWsClient() override = default;
    void SetCfgUrl(const std::string& url){
        m_cfgUrl = url;
    }
protected:
    void onRecv(const Buffer::Ptr &pBuf) override {
        DataFilter::Filter(pBuf);
//        InfoL<<"recv from ws:"<<pBuf->size();
        const std::string configCmd = "TalusTunConfig";
        if(pBuf->size()>configCmd.length()&& strncmp(configCmd.c_str(),pBuf->data(),configCmd.size())==0){
            m_config = true;
            auto configStr = pBuf->toString();
            InfoL<<"Got tun config:"<<configStr;
            auto cfg = split(configStr,",");
            if(cfg.size()!=4){
                ErrorL<<"invalid cfg:"<<configStr;
                return;
            }
            //config
            TalusTunInterface::Instance()->Stop();
            TalusTunInterface::Instance()->Down();
            TalusTunInterface::Instance()->Config(TalusTunInterface::TalusTunCfg(cfg[1], std::stol(cfg[2]), std::stol(cfg[3])));
            auto dispatcher = TalusTunInterface::Dispatcher::makeDispatcher("0.0.0.0",0,[this](const toolkit::Buffer::Ptr& pkt){
//                InfoL<<"recv from tun:"<<pkt->size();
//                InfoL<<"send to ws:"<<pkt->size();
                if(!m_config){
                    return;
                }
                getPoller()->async([this,pkt](){
                    DataFilter::Filter(pkt);
                    SockSender::send(pkt->data(),pkt->size());
                });
            });
            TalusTunInterface::Instance()->AddDispatcher("0.0.0.0",dispatcher);
            TalusTunInterface::Instance()->Up();
            TalusTunInterface::Instance()->Start();
        }else{
            if(!m_config){
                return;
            }
//            InfoL<<"send to tun:"<<pBuf->size();
            TalusTunInterface::Instance()->Send(pBuf);
        }
    }
    //????????????????????????
    void onErr(const SockException &ex) override {
        WarnL << ex.what();
        m_config = false;
        getPoller()->doDelayTask(1000,[this](){
            dynamic_cast<WebSocketClient<TunWsClient,WebSocketHeader::BINARY,false>*>(this)->startWebSocket(m_cfgUrl, 5);
            return 0;
        });
    }
    //tcp??????????????????2????????????????????????
    void onManager() override {
        if(!m_config&&m_ticker.elapsedTime()>10*1000){
            shutdown(SockException(Err_shutdown,"not config for long!"));
        }
    }
    //???????????????????????????
    void onConnect(const SockException &ex) override{
        if(ex){
            DebugL << m_cfgUrl <<" " << ex.what();
            getPoller()->doDelayTask(1000,[this](){
                dynamic_cast<WebSocketClient<TunWsClient,WebSocketHeader::BINARY,false>*>(this)->startWebSocket(m_cfgUrl, 5);
                return 0;
            });
            return;
        }
        m_ticker.resetTime();
        auto repBuf = make_shared<BufferLikeString>("GetTalusTunConfig");
        DataFilter::Filter(repBuf);
        SockSender::send(repBuf->data(),repBuf->size());
    }
    //?????????????????????????????????
    void onFlush() override{
    }
    std::string m_cfgUrl;
    bool m_config = false;
    Ticker m_ticker;
};


#endif //TALUSTUN_TUNWSCLIENT_H
