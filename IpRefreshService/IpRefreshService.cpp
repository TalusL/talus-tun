

#include <Network/TcpServer.h>
#include <csignal>
#include "Http/HttpSession.h"
#include <Util/NoticeCenter.h>
#include "Http/config.h"
#include "System.h"
#include "Util/base64.h"

using namespace toolkit;
using namespace mediakit;


#define HTTP_PORT "http.port"
#define UPDATE_IP_CMD "service.cmd"
onceToken tk([]() {
    mINI::Instance()[HTTP_PORT] = 8233;
    mINI::Instance()[UPDATE_IP_CMD] = "/bin/sh -c 'echo $REMOTE_IP$ |gzip|base64 > /var/www/html/$ID$'";
});

int  main(int argc,char **argv){
    Logger::Instance().add(std::make_shared<ConsoleChannel> ());

    if(argc<2){
        ErrorL<<"need config file to setup!";
        return -1;
    }

#ifndef BUILD_DEBUG
    bool kill_parent_if_failed = true;
    System::startDaemon(kill_parent_if_failed);
#endif

    loadIniConfig(argv[1]);

    int httpPort = mINI::Instance()[HTTP_PORT];
    TcpServer::Ptr httpSrv(new TcpServer());
    httpSrv->start<HttpSession>(httpPort,"0.0.0.0");

    std::map<std::string,std::string> ipMap;
    std::mutex mtx;
    NoticeCenter::Instance().addListener(nullptr,Broadcast::kBroadcastHttpRequest,[&](BroadcastHttpRequestArgs){
        if (parser.Url()=="/index/api/keepalive"){
            consumed = true;
            auto ip = sender.get_peer_ip();
            auto id = parser.getUrlArgs()["id"];
            if(id.empty()){
                invoker(400,{},"");
                return ;
            }
            std::lock_guard<std::mutex> lck(mtx);
            ipMap[id] = ip;
            invoker(200,{},ip);
        }
    });
    NoticeCenter::Instance().addListener(nullptr,Broadcast::kBroadcastHttpRequest,[&](BroadcastHttpRequestArgs){
        if (parser.Url()=="/index/api/getaddr"){
            consumed = true;
            auto id = parser.getUrlArgs()["id"];
            if(id.empty()){
                invoker(400,{},"");
                return ;
            }
            std::lock_guard<std::mutex> lck(mtx);
            invoker(200,{},encodeBase64(ipMap[id]));
        }
    });

    //设置退出信号处理函数
    static semaphore sem;
    signal(SIGINT, [](int) {
        InfoL << "SIGINT:exit";
        signal(SIGINT, SIG_IGN);// 设置退出信号
        sem.post();
    });// 设置退出信号
    sem.wait();

    return 0;
}