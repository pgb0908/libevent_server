// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNECTION_H

#include "event_server/common/noncopyable.h"
#include "event_server/common/StringPiece.h"
#include "event_server/common/Types.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "InetAddress.h"
#include "event_server/event/Dispatcher.h"

#include <memory>

#include <boost/any.hpp>

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace muduo {
    namespace net {
        class Channel;
        class Socket;

        ///
        /// TCP connection, for both client and server usage.
        ///
        /// This is an interface class, so don't expose too much details.
        class TcpConnection : noncopyable,
                              public std::enable_shared_from_this<TcpConnection> {
        public:
            /// Constructs a TcpConnection with a connected sockfd
            ///
            /// User should not create this object.
            TcpConnection(Event::Dispatcher *dispatcher,
                          const string &name,
                          int sockfd,
                          const InetAddress &localAddr,
                          const InetAddress &peerAddr);

            ~TcpConnection();

            Event::Dispatcher *getLoop() const { return dispatcher_; }
            const string &name() const { return name_; }
            const InetAddress &localAddress() const { return localAddr_; }
            const InetAddress &peerAddress() const { return peerAddr_; }
            bool connected() const { return state_ == kConnected; }
            bool disconnected() const { return state_ == kDisconnected; }
            // return true if success.
            bool getTcpInfo(struct tcp_info *) const;
            string getTcpInfoString() const;

            // void send(string&& message); // C++11
            void send(const void *message, int len);
            void send(const StringPiece &message);
            // void send(Buffer&& message); // C++11
            void send(Buffer *message);  // this one will swap data
            void shutdown(); // NOT thread safe, no simultaneous calling
            //void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no simultaneous calling
            void forceClose();
            void forceCloseWithDelay(double seconds);
            void setTcpNoDelay(bool on);

            bool isReading() const { return reading_; }; // NOT thread safe, may race with start/stopReadInLoop
            void setContext(const boost::any &context) { context_ = context; }
            const boost::any &getContext() const { return context_; }
            boost::any *getMutableContext() { return &context_; }

            //void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
            void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
            void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
            void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark) {
                highWaterMarkCallback_ = cb;
                highWaterMark_ = highWaterMark;
            }

            /// Advanced interface
            Buffer *inputBuffer() { return &inputBuffer_; }
            Buffer *outputBuffer() { return &outputBuffer_; }

            /// Internal use only.
            void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

            // called when TcpServer accepts a new connection
            void connectEstablished();   // should be called only once
            // called when TcpServer has removed me from its map
            void connectDestroyed();  // should be called only once

        private:
            enum StateE {
                kDisconnected, kConnecting, kConnected, kDisconnecting
            };

            void handleRead(uint32_t events);
            void handleWrite(uint32_t events);
            void handleClose();
            void handleError();

            void setState(StateE s) { state_ = s; }
            const char *stateToString() const;

            Event::Dispatcher *dispatcher_;
            const string name_;
            StateE state_;  // FIXME: use atomic variable
            bool reading_;

            // we don't expose those classes to client.
            std::unique_ptr<Socket> socket_;
            const InetAddress localAddr_;
            const InetAddress peerAddr_;
            //ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            HighWaterMarkCallback highWaterMarkCallback_;
            CloseCallback closeCallback_;
            size_t highWaterMark_;
            Buffer inputBuffer_;
            Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
            boost::any context_;
            Event::FileEventPtr readEventPtr_;
            Event::FileEventPtr writeEventPtr_;
        };

        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

    }  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_TCPCONNECTION_H
