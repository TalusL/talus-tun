
#include <thread>
#include <Util/util.h>
#include <csignal>
#include <Network/TcpServer.h>
#include "Http/HttpSession.h"
#include "System.h"
#include <Util/onceToken.h>

#include "TunWsClient.h"
#include "TunWsSession.h"

#include "Http/WebSocketClient.h"

#include "Config.h"

using namespace std;
using namespace mediakit;
using namespace toolkit;


onceToken tk([](){
mINI::Instance()[CONFIG_MODE] = 2;
mINI::Instance()[CONFIG_NAME] = "Talus0";
mINI::Instance()[CONFIG_WS_LISTEN_IP] = "0.0.0.0";
mINI::Instance()[CONFIG_WS_LISTEN_PORT] = 8989;
mINI::Instance()[CONFIG_TUN_NET_IP] = "10.22.33.1";
mINI::Instance()[CONFIG_TUN_NET_MASK] = "24";
mINI::Instance()[CONFIG_TUN_NET_MTU] = 1450;
mINI::Instance()[CONFIG_WS_URL] = "ws://127.0.0.1:8989/ubuntu.iso";
mINI::Instance()[CONFIG_SSL_CERT] = "./default.pem";
mINI::Instance()[CONFIG_ADDR_ALLOC_BEGIN] = 100;
mINI::Instance()[CONFIG_ADDR_ALLOC_END] = 200;
mINI::Instance()[CONFIG_ENABLE_SSL] = false;
mINI::Instance()[CONFIG_PASS] = "TalusTun0123456789";
});

struct WsSessionCreator {
    //返回的Session必须派生于SendInterceptor，可以返回null(拒绝连接)
    Session::Ptr operator()(const Parser &header, const HttpSession &parent, const Socket::Ptr &pSock) {
        return std::make_shared<SessionTypeImp<TunWsSession> >(header, parent, pSock);
    }
};


bool loadIniConfig(const char *ini_path) {
    string ini;
    if (ini_path && ini_path[0] != '\0') {
        ini = ini_path;
    } else {
        ini = exePath() + ".ini";
    }
    try {
        mINI::Instance().parseFile(ini);
        return true;
    } catch (std::exception &) {
        InfoL << "dump ini file to:" << ini;
        mINI::Instance().dumpFile(ini);
        return false;
    }
}

int main(int argc,char **argv){
    Logger::Instance().add(std::make_shared<ConsoleChannel> ());

    if(argc<2){
        ErrorL<<"need config file to setup!";
        return -1;
    }

//    bool kill_parent_if_failed = true;
//    System::startDaemon(kill_parent_if_failed);

    loadIniConfig(argv[1]);

    int mode = mINI::Instance()[CONFIG_MODE];
    string listenIp = mINI::Instance()[CONFIG_WS_LISTEN_IP];
    int listenPort = mINI::Instance()[CONFIG_WS_LISTEN_PORT];
    string tunNetIp = mINI::Instance()[CONFIG_TUN_NET_IP];
    int tunNetMask = mINI::Instance()[CONFIG_TUN_NET_MASK];
    int tunNetMtu = mINI::Instance()[CONFIG_TUN_NET_MTU];
    string wsUrl = mINI::Instance()[CONFIG_WS_URL];
    string sslCert = mINI::Instance()[CONFIG_SSL_CERT];
    bool enableSsl = mINI::Instance()[CONFIG_ENABLE_SSL];


    //设置退出信号处理函数
    static semaphore sem;
    signal(SIGINT, [](int) {
        InfoL << "SIGINT:exit";
        signal(SIGINT, SIG_IGN);// 设置退出信号
        sem.post();
    });// 设置退出信号

    if(mode == 1){
        TcpServer::Ptr httpSrv(new TcpServer());

        if(enableSsl){
            SSL_Initor::Instance().loadCertificate(sslCert);
            httpSrv->start<WebSocketSessionBase<WsSessionCreator, HttpsSession> >(listenPort,listenIp);
        }else{
            httpSrv->start<WebSocketSessionBase<WsSessionCreator, HttpSession> >(listenPort,listenIp);
        }


        //config
        TalusTunInterface::Instance()->Stop();
        TalusTunInterface::Instance()->Down();
        TalusTunInterface::Instance()->Config(TalusTunInterface::TalusTunCfg(tunNetIp, tunNetMask, tunNetMtu));
        TalusTunInterface::Instance()->Up();
        TalusTunInterface::Instance()->Start();

        sem.wait();
    } else {
        auto client = make_shared<WebSocketClient<TunWsClient,WebSocketHeader::BINARY,false>>();
        dynamic_pointer_cast<TunWsClient>(client)->SetCfgUrl(wsUrl);
        client->startWebSocket(wsUrl, 5);
        sem.wait();
    }

    return 0;
}