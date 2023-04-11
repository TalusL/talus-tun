

#include <Network/TcpServer.h>
#include <csignal>
#include "Http/HttpSession.h"
#include <Util/NoticeCenter.h>
#include "Http/config.h"
#include "System.h"
#include "Process.h"

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

    NoticeCenter::Instance().addListener(nullptr,Broadcast::kBroadcastHttpRequest,[](BroadcastHttpRequestArgs){
        if (parser.Url()=="/index/api/keepalive"){
            consumed = true;
            auto ip = sender.get_peer_ip();
            auto id = parser.getUrlArgs()["id"];
            if(id.empty()){
                invoker(400,{},"");
                return ;
            }
            invoker(200,{},"");
            WorkThreadPool::Instance().getPoller()->async([ip,id](){
                auto cmd = mINI::Instance()[UPDATE_IP_CMD];
                std::string path;
                replace(cmd,"$REMOTE_IP$",ip);
                replace(cmd,"$ID$",id);
                InfoL<<"run cmd:"<<cmd;
                Process process;
                process.run(cmd,path);
                process.wait();
            });
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