// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "TcpConnection.h"

#include "WeakCallback.h"

#include "Socket.h"
#include "SocketsOps.h"

#include <cerrno>
#include <iostream>
#include <event2/event.h>
#include <glog/logging.h>
#include <fstream>

using namespace muduo;
using namespace muduo::net;

void muduo::net::defaultConnectionCallback(const TcpConnectionPtr &conn) {
/*  LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");*/
    // do not call conn->forceClose(), because some users want to register message callback only.
}

void muduo::net::defaultMessageCallback(const TcpConnectionPtr &,
                                        Buffer *buf) {
    buf->retrieveAll();
}

TcpConnection::TcpConnection(Event::DispatcherImp *dispatcher,
                             const string &nameArg,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
        : dispatcher_(dispatcher),
          name_(nameArg),
          state_(kConnecting),
          reading_(true),
          socket_(new Socket(sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr),
          highWaterMark_(64 * 1024 * 1024) {

    LOG(INFO) << "TcpConnection::ctor[" << name_ << "] at " << this
              << " fd=" << sockfd;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
/*    std::cout << "TcpConnection::dtor[" << name_ << "] at " << this
              << " fd=" << socket_->fd()
              << " state=" << stateToString() << std::endl;*/
    //assert(state_ == kDisconnected);
    LOG(INFO) << "TcpConnection DESTRUCTURE";
}

bool TcpConnection::getTcpInfo(struct tcp_info *tcpi) const {
    return socket_->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString() const {
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof buf);
    return buf;
}

void TcpConnection::send(const void *data, int len) {
    send(StringPiece(static_cast<const char *>(data), len));
}

void TcpConnection::send(const StringPiece &message) {
    if (state_ == kConnected) {
/*    if (loop_->isInLoopThread())
    {
      sendInLoop(message);
    }
    else
    {
      void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
      loop_->runInLoop(
          std::bind(fp,
                    this,     // FIXME
                    message.as_string()));
                    //std::forward<string>(message)));
    }*/


    }
}

// FIXME efficiency!!!
void TcpConnection::send(Buffer *buf) {
    assert(state_ == kConnected);
    ssize_t nwrote = 0;
    auto data = buf->peek();
    size_t len = buf->readableBytes();
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        LOG(INFO) << "disconnected, give up writing";
        return;
    }
    // if no thing in output queue, try writing directly
    if (!writeEventPtr_ && outputBuffer_.readableBytes() == 0)
    {
        nwrote = sockets::write(socket_->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            LOG(INFO) << "remaining : " << remaining << "   len : "<< len;
            // event를 걸어줌
            if (remaining == 0 && writeCompleteCallback_)
            {
                //loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                writeCompleteCallback_(shared_from_this());
            }
        }
        else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG(INFO) << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
/*    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }*/
}


void TcpConnection::shutdown() {
    // FIXME: use compare and swap
    if (state_ == kConnected) {
        setState(kDisconnecting);
        // FIXME: shared_from_this()?
        //loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}



// void TcpConnection::shutdownAndForceCloseAfter(double seconds)
// {
//   // FIXME: use compare and swap
//   if (state_ == kConnected)
//   {
//     setState(kDisconnecting);
//     loop_->runInLoop(std::bind(&TcpConnection::shutdownAndForceCloseInLoop, this, seconds));
//   }
// }

// void TcpConnection::shutdownAndForceCloseInLoop(double seconds)
// {
//   loop_->assertInLoopThread();
//   if (!channel_->isWriting())
//   {
//     // we are not writing
//     socket_->shutdownWrite();
//   }
//   loop_->runAfter(
//       seconds,
//       makeWeakCallback(shared_from_this(),
//                        &TcpConnection::forceCloseInLoop));
// }

void TcpConnection::forceClose() {
    // FIXME: use compare and swap
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        // loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
/*    loop_->runAfter(
        seconds,
        makeWeakCallback(shared_from_this(),
                         &TcpConnection::forceClose));  // not forceCloseInLoop to avoid race condition*/
    }
}



const char *TcpConnection::stateToString() const {
    switch (state_) {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}


void TcpConnection::connectEstablished() {
    dispatcher_->assertInLoopThread();
    //connectionCallback_(shared_from_this())
    //assert(state_ == kConnecting);
    LOG(INFO) << "TCP connection Established";
    setState(kConnected);
    readEventPtr_ = dispatcher_->createFileEvent(socket_->fd(),
                                              [this](uint32_t events) {
                                                  handleRead(events);
                                              },
                                                 Event::FileTriggerType::Level,
                                                 Event::FileReadyType::Read);
}

void TcpConnection::connectDestroyed() {
    //loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        //channel_->disableAll();

    }
    //channel_->remove();
}

void TcpConnection::handleRead(uint32_t events) {
    //loop_->assertInLoopThread();

    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(socket_->fd(), &savedErrno);
    LOG(INFO) << "read from buffer size : " << n;

    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_);
    } else if (n == 0) {
        handleClose();
        return;
    } else {
        errno = savedErrno;
        LOG(ERROR) << "TcpConnection::handleRead";
        handleError();
        return;
    }

    if(outputBuffer_.readableBytes() > 0){
        writeEventPtr_ = dispatcher_->createFileEvent(socket_->fd(),
                                                      [this](uint32_t events) {
                                                          handleWrite(events);
                                                      },
                                                      Event::FileTriggerType::Level,
                                                      Event::FileReadyType::Write);
    }
}

void TcpConnection::handleWrite(uint32_t events) {
    LOG(INFO)<< "TcpConnection::handleWrite() occurred";
    //loop_->assertInLoopThread();

    if (writeEventPtr_) {
        ssize_t n = sockets::write(socket_->fd(),
                                   outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());

        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                LOG(INFO)<< "TcpConnection::handleWrit() - sockets::write is to function normal. cancel the write event";
                // todo writeEventPtr_ canceal??;
                if (writeCompleteCallback_) {
                    // loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                    writeCompleteCallback_(shared_from_this());
                }
                if (state_ == kDisconnecting) {
                    //shutdownInLoop();
                }
            }
        } else {
            LOG(ERROR) << "TcpConnection::handleWrite Error";
            // if (state_ == kDisconnecting)
            // {
            //   shutdownInLoop();
            // }
        }
    } else {
        LOG(INFO)<< "Connection fd = " << socket_->fd() << " is down, no more writing";
    }
}

void TcpConnection::handleClose() {
    // loop_->assertInLoopThread();
    LOG(INFO) << "fd = " << socket_->fd() << " state = " << stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);

    TcpConnectionPtr guardThis(shared_from_this());
    closeCallback_(guardThis);

    //size_t size = 1024;
    //char *buffer = (char *)malloc(size); // 예시로 1024 바이트 할당
    // 파일 디스크립터 생성
/*    FILE *memstream = open_memstream(&buffer, &size);
    event_base_dump_events(&dispatcher_->base(), memstream);
    // 메모리 스트림 닫기
    fclose(memstream);
    LOG(INFO) << buffer ;
    delete buffer;*/

}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(socket_->fd());
    LOG(ERROR) << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err << " "; //strerror_tl(err);
}


