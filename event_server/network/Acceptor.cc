// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "Acceptor.h"

#include "InetAddress.h"
#include "SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <event2/event.h>
#include <glog/logging.h>

using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(Event::DispatcherImp *dispatcher, const InetAddress &listenAddr, bool reuseport)
        : dispatcher_(dispatcher),
          acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
          listening_(false),
          idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
/*  acceptChannel_.setReadCallback(
      std::bind(&Acceptor::handleRead, this));*/

    //listen();
}

Acceptor::~Acceptor() {
    ::close(idleFd_);
}

void Acceptor::listen() {
    //loop_->assertInLoopThread();
    assert(dispatcher_->isInLoopThread());
    listening_ = true;
    if (!acceptSocket_.listen()) return;

    int fd = acceptSocket_.fd();
    accept_event_ = dispatcher_->createFileEvent(fd,
                                                 [this](uint32_t) {
                                                     handleRead();
                                                 },
                                                 Event::FileTriggerType::Level,
                                                 Event::FileReadyType::Read);

}

void Acceptor::handleRead() {
    //assert(flags & (Envoy::Event::FileReadyType::Read));
    //loop_->assertInLoopThread();
    InetAddress peerAddr;
    //FIXME loop until no more

    LOG(INFO) << "handleRead() event occurred";

    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        // string hostport = peerAddr.toIpPort();
        // LOG_TRACE << "Accepts of " << hostport;
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    } else {
        //LOG_SYSERR << "in Acceptor::handleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE) {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

