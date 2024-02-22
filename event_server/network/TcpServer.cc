// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "TcpServer.h"

#include "Acceptor.h"
#include "SocketsOps.h"

#include <stdio.h>  // snprintf
#include <iostream>
#include <csignal>
#include "event_server/event/Dispatcher.h"

using namespace muduo;
using namespace muduo::net;


TcpServer::TcpServer(Dispatcher *dispatcher,
                     const InetAddress &listenAddr,
                     const string &nameArg,
                     Option option)
        : dispatcher_(dispatcher),
          ipPort_(listenAddr.toIpPort()),
          name_(nameArg),
          acceptor_(new Acceptor(dispatcher, listenAddr, option == kReusePort)),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback),
          nextConnId_(1) {
    acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
    //loop_->assertInLoopThread();
    //LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto &item: connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        //conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    //threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (started_.getAndSet(1) == 0) {
        //threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listening());
        acceptor_->listen();

        dispatcher_->dispatch_loop();
    }


}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    //loop_->assertInLoopThread();
    //EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;

    std::cout << "TcpServer::newConnection [" << name_
              << "] - new connection [" << connName
              << "] from " << peerAddr.toIpPort() << std::endl;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TcpConnectionPtr conn(new TcpConnection(dispatcher_,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));

    conn->setWriteCompleteCallback([](const TcpConnectionPtr &conn){
        std::cout << "send is done!!!" << std::endl;
        conn->inputBuffer()->retrieveAll();
        std::cout << conn->inputBuffer()->peek() << std::endl;
    });
    conn->setMessageCallback([](const TcpConnectionPtr &conn, Buffer *buffer)
                                    {
                                        size_t len = buffer->readableBytes();
                                        conn->send(buffer);
                                    });
    conn->setCloseCallback(
            std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe

    //ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));

    // main-loop >> io-loop 에서 처리하도록 하는 명령함수가 필요함
    conn->setReadEventPtr(conn->connectEstablished());
    connections_[connName] = std::move(conn);

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    // FIXME: unsafe
    //loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
    auto temp = connections_[conn->name()];
    //temp->getReadEventPtr()->cancelEvent();

    std::cout << "TcpServer::removeConnectionInLoop [" << name_
              << "] - connection " << temp->name() << std::endl;

    size_t n = connections_.erase(conn->name());
    (void) n;
    assert(n == 1);

}

/*void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    //loop_->assertInLoopThread();
    std::cout << "TcpServer::removeConnectionInLoop [" << name_
              << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    (void) n;
    assert(n == 1);
    //EventLoop* ioLoop = conn->getLoop();
    //ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}*/



