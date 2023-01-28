
#include <thread>
#include <iostream>
#include "TalusTunInterface.h"
#include "Transport.h"
#include <Util/util.h>
#include <csignal>
#include "PrintUtil.h"
#include <Network/TcpServer.h>
#include "Http/HttpSession.h"

using namespace std;


int main(int argc,char **argv){
    Logger::Instance().add(std::make_shared<ConsoleChannel> ());

//    Transport transport(argc>1?Transport::TransportType::UDP:Transport::TransportType::UDP);
//    auto b = argc>1?transport.Init(2556,"0.0.0.0"):transport.Init();
//    if(!b){
//        cout<<"create transport fail!";
//        return -1;
//    }
//    auto port = transport.GetLocalPort();
//    cout<<"port:"<<port<<"\n";
//
//    if(argc>1){
//        transport.Start();
//    }else{
//        transport.Start(2556,"192.168.1.55");
//    }
//
//    TalusTunInterface::TalusTunCfg cfg(argc>1?"192.168.111.1":"192.168.111.2",24,1400);
//    TalusTunInterface interface(cfg);
//    interface.Up();
//
//    interface.Listen([&transport](const toolkit::BufferRaw::Ptr& pkt){
//        dump_packet(pkt->size(), reinterpret_cast<char *>(pkt->data()));
//
//        transport.Send(pkt);
//
//        return true;
//    });
//
//    transport.SetDataCallback([&](const Buffer::Ptr &buf, struct sockaddr *addr, int addr_len){
//        dump_packet(buf->size(), reinterpret_cast<char *>(buf->data()));
//        toolkit::BufferRaw::Ptr pkt = toolkit::BufferRaw::create();
//        pkt->assign(buf->data(), buf->size());
//        interface.Send(pkt);
//    });

    TcpServer::Ptr httpSrv = make_shared<TcpServer>();
    httpSrv->start<mediakit::HttpSession>(8080);


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