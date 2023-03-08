
#include <thread>
#include <Util/util.h>
#include <csignal>
#include <Network/TcpServer.h>
#include "Http/HttpSession.h"

#include "WsClient.h"
#include "WsSession.h"

#include "Http/WebSocketClient.h"

using namespace std;
using namespace mediakit;


struct WsSessionCreator {
    //返回的Session必须派生于SendInterceptor，可以返回null(拒绝连接)
    Session::Ptr operator()(const Parser &header, const HttpSession &parent, const Socket::Ptr &pSock) {
        return std::make_shared<SessionTypeImp<WsSession> >(header, parent, pSock);
    }
};

int main(int argc,char **argv){
    Logger::Instance().add(std::make_shared<ConsoleChannel> ());

    auto pid = fork();

    //设置退出信号处理函数
    static semaphore sem;
    signal(SIGINT, [](int) {
        InfoL << "SIGINT:exit";
        signal(SIGINT, SIG_IGN);// 设置退出信号
        sem.post();
    });// 设置退出信号
    if(pid == 0){
//    if(argc>1&&string(argv[1])=="-s"){
        TcpServer::Ptr httpSrv(new TcpServer());
        //http服务器,支持websocket
        httpSrv->start<WebSocketSessionBase<WsSessionCreator, HttpSession> >(8989,"0.0.0.0");
        sem.wait();
    } else {
        auto client = make_shared<WebSocketClient<WsClient,WebSocketHeader::BINARY,false>>();
        sleep(2);
        auto cfgUrl = "ws://127.0.0.1:8989/ww";
        dynamic_pointer_cast<WsClient>(client)->SetCfgUrl(cfgUrl);
        client->startWebSocket(cfgUrl, 5);
        sem.wait();
    }

    return 0;
}