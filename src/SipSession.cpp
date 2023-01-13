//
// Created by liangzhuohua on 2023/1/12.
//

#include "SipSession.h"
#include <regex>

using namespace std;

bool reg_search(const string& str,const string& reg,smatch & m){
    if(!regex_search(str,m,regex(reg))){
        return false;
    }
    return true;
}

SipSession::SipSession(const Socket::Ptr &sock) : Session(sock) {
    m_sipThPool = std::make_shared<ThreadPool>(1, ThreadPool::PRIORITY_NORMAL, true);
}

SipSession::~SipSession() {

}

void SipSession::onRecv(const Buffer::Ptr & pkt) {
    m_sipThPool->async([this,pkt](){
        processSip(pkt);
    });
}

void SipSession::onError(const SockException &err) {

}

void SipSession::onManager() {
}

void SipSession::processSip(const Buffer::Ptr &pkt) {
    smatch match;
    set<string> methods = {"REGISTER","INVITE","BYE"};

    string msgBody(pkt->data(),pkt->size());
    if(!reg_search(msgBody,"^(\\S+)",match)){
        return;
    }
    auto m = match.str(1);
    if(methods.find(m)==methods.end()){
        //TODO NOT SUPPORT
        return;
    }

    if(m!="REGISTER"&&!m_register){
        //TODO NOT AUTH
        return;
    }
}
