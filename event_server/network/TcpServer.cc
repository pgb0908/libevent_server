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
#include "event_server/event/DispatcherImp.h"

using namespace muduo;
using namespace muduo::net;


TcpServer::TcpServer(Event::DispatcherImp *dispatcher,
                     const InetAddress &listenAddr,
                     const string &nameArg, int threadNum,
                     Option option)
        : dispatcher_(dispatcher),
          ipPort_(listenAddr.toIpPort()),
          name_(nameArg),
          threadPool_(new EventLoopThreadPool(threadNum, name_)),
          acceptor_(new Acceptor(dispatcher, listenAddr, option == kReusePort)),
          threadInitCallback_([](Event::DispatcherImp* dispatcher){
              LOG(INFO) <<  "Thread init. " << "thread-id : " <<dispatcher->getThreadId();
          }),
          connectionCallback_(defaultConnectionCallback),
          messageCallback_(defaultMessageCallback),
          nextConnId_(1) {
    acceptor_->setNewConnectionCallback(
            std::bind(&TcpServer::newConnection, this, _1, _2));
    threadPool_->start();
    LOG(INFO) << "TcpServer created. " << "thread-id : " <<dispatcher_->getThreadId();
}

TcpServer::~TcpServer() {
    //loop_->assertInLoopThread();
    LOG(INFO) << "TcpServer::~TcpServer [" << name_ << "] destructing";

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
    LOG(INFO) << "TcpServer start. ";

    if (started_.getAndSet(1) == 0) {
       // threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listening());
        acceptor_->listen();

/*        dispatcher_->post([this](){
            dispatcher_->printRegistEvent();
        });*/

        dispatcher_->dispatch_loop(Event::Dispatcher::RunType::RunUntilExit);
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    //loop_->assertInLoopThread();
    Event::DispatcherImp* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    string connName = name_ + buf;

    LOG(INFO) << "TcpServer::newConnection [" << name_
              << "] - new connection [" << connName
              << "] from " << peerAddr.toIpPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));

    conn->setWriteCompleteCallback(
            [this](const TcpConnectionPtr &conn) {
                doWriteCompleteDefault(conn);
            });
    conn->setMessageCallback(
            [this](const TcpConnectionPtr &conn, Buffer *buffer) {
                doMessageDefault(conn, buffer);
            });
    conn->setCloseCallback(
            [this](const TcpConnectionPtr &conn) {
                removeConnection(conn);
            }); // FIXME: unsafe

    // main-loop >> io-loop 에서 처리하도록 하는 명령함수가 필요함
    ioLoop->post([conn](){
        conn->connectEstablished();
    });

    connections_[connName] = std::move(conn);

}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    // FIXME: unsafe
    //loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
    auto temp = connections_[conn->name()];
    //temp->getReadEventPtr()->cancelEvent();

    LOG(INFO) << "TcpServer::removeConnectionInLoop [" << name_
              << "] - connection " << temp->name();

    size_t n = connections_.erase(conn->name());
    (void) n;
    assert(n == 1);

}

void TcpServer::doMessageDefault(const TcpConnectionPtr &conn, muduo::net::Buffer *buf) {
    //size_t len = buf->readableBytes();
    LOG(INFO) << "doMessageDefault";
    //conn->outputBuffer()->append(buf->peek(), buf->readableBytes());
    conn->send(buf);
}

void TcpServer::doWriteCompleteDefault(const TcpConnectionPtr &conn) {
    auto size = conn->inputBuffer()->readableBytes();
    auto data = conn->inputBuffer()->peek();
    LOG(INFO) << "WriteComplete is done : " << std::string(data, size);
    conn->inputBuffer()->retrieveAll();
}

void TcpServer::doThreadInitDefault() {
    LOG(INFO) << "Thread init";
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



