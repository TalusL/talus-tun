//
// Created by liangzhuohua on 2023/1/12.
//

#ifndef TALUSTUN_TRANSPORT_H
#define TALUSTUN_TRANSPORT_H

#include <random>
#include <iostream>
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
    bool Start(uint16_t remotePort = 0,const std::string& remoteAddr = ""){
        if(!m_sock){
            return false;
        }
        if(TCP_ACTIVE == m_transportType){
            m_sock->setOnRead([this](const Buffer::Ptr &buf, struct sockaddr *addr, int addr_len){
                if(m_onRead){
                    m_onRead(buf,addr,addr_len);
                }
            });
            m_sock->connect(remoteAddr,remotePort,[](const SockException &err){
                //TODO error
            },5,m_localAddr,m_localPort);
        }
        if(TCP_PASSIVE == m_transportType) {
            m_sock->setOnAccept([this](Socket::Ptr &sock, std::shared_ptr<void> &complete) {
                if(!m_ClientSock){
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
                }else{
                    sock->closeSock();
                }
            });
        }
        if(UDP == m_transportType){
            if(remotePort&&!remoteAddr.empty()){
                makeAddr(&m_udpRemote, remoteAddr.c_str(), remotePort);
                m_bindUdp = true;
            }
            m_sock->setOnRead([this](const Buffer::Ptr &buf, struct sockaddr *addr, int addr_len){
                if(!m_bindUdp){
                    m_udpRemote = *(sockaddr_storage*)addr;
                    m_bindUdp = true;
                }
                if(m_onRead){
                    m_onRead(buf,addr,addr_len);
                }
            });
        }

        m_sock->setOnErr([](const SockException &err){
            std::cout<<err.what();
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
    bool Send(const toolkit::BufferRaw::Ptr& pkt) const {
        if(!m_sock){
            return false;
        }
        if(m_transportType == TCP_PASSIVE){
            if(m_ClientSock){
                m_ClientSock->send(pkt);
            }
        }
        else if(m_transportType == UDP) {
            m_sock->send(pkt,(sockaddr*)&m_udpRemote, sizeof(sockaddr));
        }else{
            m_sock->send(pkt);
        }
        return true;
    }
    void SetDataCallback(const toolkit::Socket::onReadCB& cb){
        m_onRead = cb;
    }

private:
    Socket::Ptr m_sock;
    Socket::Ptr m_ClientSock;
    TransportType m_transportType;
    const std::pair<uint16_t,uint16_t> m_portRange = {30000,40000};
    uint16_t m_localPort = 0;
    std::string m_localAddr;
    toolkit::Socket::onReadCB m_onRead;
    volatile bool m_bindUdp = false;
    struct sockaddr_storage m_udpRemote{};
};


#endif //TALUSTUN_TRANSPORT_H
