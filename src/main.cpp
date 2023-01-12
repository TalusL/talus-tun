
#include <thread>
#include <iostream>
#include "TalusTunInterface.h"
#include "Transport.h"

using namespace std;
int main(){

    Transport transport(Transport::TransportType::TCP_ACTIVE);
    auto b = transport.Init();
    auto port = transport.GetLocalPort();
    transport.Start(2223,"10.8.9.97");
    cout<<"port:"<<port<<"\n";

    TalusTunInterface::TalusTunCfg cfg("192.168.111.2","255.255.255.0","192.168.111.255",1400,TalusTunInterface::GeneraMac());
    TalusTunInterface interface(cfg);
    interface.up();

    interface.Listen([&transport](string const& name, uint id, vector<uint8_t>& packet){
        static uint count = 0;

        cout << "+++ Received packet " << dec << count;
        cout << " from interface " << name;
        cout << " (" << id << ") of size " << packet.size();
        cout << " and CRC of 0x" << hex << viface::utils::crc32(packet);
        cout << endl;
        cout << viface::utils::hexdump(packet) << endl;
        count++;

        transport.Send(packet.data(),packet.size());

        return true;
    });



    std::this_thread::sleep_for(std::chrono::seconds (1000));

    return 0;
}