//
// Created by liangzhuohua on 2023/1/12.
//

#ifndef TALUSTUN_TRANSPORT_H
#define TALUSTUN_TRANSPORT_H

#include <random>
#include "Network/Socket.h"

using namespace toolkit;

class Transport {
public:
    enum TransportType{
        UDP = 1,
        TCP_ACTIVE = 2,
        TCP_PASSIVE = 3
    };
    explicit Transport(const TransportType& type){
        m_transportType = type;
    }
    bool Init(uint16_t port = 0,const std::string& localAddr = "0.0.0.0"){
        if(!port){
            port = AllocPort();
        }
        m_localPort = port;
        m_localAddr = localAddr;
        m_sock = Socket::createSocket(nullptr, true);
        SockUtil::setRecvBuf(m_sock->rawFD(), 4 * 1024 * 1024);
        if(UDP == m_transportType){
            return m_sock->bindUdpSock(port,localAddr);
        }
        if(TCP_PASSIVE == m_transportType){
            return m_sock->listen(port,localAddr);
        }
        if(TCP_ACTIVE == m_transportType){
            return true;
        }
        return false;
    }
    uint16_t GetLocalPort() const{
        return m_localPort;
    }
    uint16_t AllocPort() const{
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<uint16_t> dist(m_portRange.first, m_portRange.second);
        return dist(mt);
    }
    bool Start(uint16_t remotePort = 0,const std::string& remoteAddr = "0.0.0.0"){
        if(!m_sock){
            return false;
        }
        if(TCP_ACTIVE == m_transportType){
            m_remoteAddr = remoteAddr;
            m_remotePort = remotePort;
            m_sock->connect(m_remoteAddr,m_remotePort,[](const SockException &err){
                //TODO error
            },5,m_localAddr,m_localPort);
        }
        if(TCP_PASSIVE == m_transportType) {
            m_sock->setOnAccept([this](Socket::Ptr &sock, std::shared_ptr<void> &complete) {
                    m_ClientSock = sock;
                    m_ClientSock->setOnRead([this,sock](const Buffer::Ptr &buf, struct sockaddr *addr, int addr_len){
                        m_ClientSock->send(buf);
                        if(m_onRead){
                            m_onRead(buf,addr,addr_len);
                        }
                    });
                    m_ClientSock->setOnErr([](const SockException &err){
                        //TODO error
                    });
            });
        }
        if(TCP_ACTIVE == m_transportType || UDP == m_transportType){
            m_sock->setOnRead([this](const Buffer::Ptr &buf, struct sockaddr *addr, int addr_len){
                if(m_onRead){
                    m_onRead(buf,addr,addr_len);
                }
            });
        }

        m_sock->setOnErr([](const SockException &err){
            //TODO error
        });
        return true;
    }
    static void makeAddr(struct sockaddr_storage *out,const char *ip,uint16_t port){
        *out = SockUtil::make_sockaddr(ip, port);
    }
    void Stop(){
        if(m_sock)
            m_sock->closeSock();
    }
    bool Send(uint8_t * data,uint32_t len) const {
        if(!m_sock){
            return false;
        }
        if(m_transportType == UDP){
            struct sockaddr_storage addrDst{};
            makeAddr(&addrDst,m_remoteAddr.c_str(),m_remotePort);
            m_sock->send((char*)data,len,(sockaddr*)&addrDst, sizeof(sockaddr_storage));
        }
        if(m_transportType == TCP_ACTIVE){
            m_sock->send((char*)data,len);
        }
        if(m_transportType == TCP_PASSIVE){
            if(m_ClientSock){
                m_ClientSock->send((char*)data,len);
            }
        }
        return true;
    }

private:
    Socket::Ptr m_sock;
    Socket::Ptr m_ClientSock;
    TransportType m_transportType;
    const std::pair<uint16_t,uint16_t> m_portRange = {30000,40000};
    uint16_t m_localPort = 0;
    uint16_t m_remotePort = 0;
    std::string m_remoteAddr;
    std::string m_localAddr;
    toolkit::Socket::onReadCB m_onRead;
};


#endif //TALUSTUN_TRANSPORT_H
