//
// Created by liangzhuohua on 2023/1/12.
//

#ifndef TALUSTUN_SIPSESSION_H
#define TALUSTUN_SIPSESSION_H
#include <Network/Session.h>
#include <Thread/ThreadPool.h>

using namespace toolkit;

class SipSession : public Session {
public:
    explicit SipSession(const Socket::Ptr &sock);
    ~SipSession() override;

    void onRecv(const Buffer::Ptr &) override;
    void onError(const SockException &err) override;
    void onManager() override;
private:
    std::shared_ptr<ThreadPool> m_sipThPool;
    void processSip(const Buffer::Ptr &pkt);
    bool m_register = false;
};


#endif //TALUSTUN_SIPSESSION_H
