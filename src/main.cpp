
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