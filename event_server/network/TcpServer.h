// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include "Atomic.h"
#include "Types.h"
#include "TcpConnection.h"
#include "event_server/event/Dispatcher.h"

#include <map>
#include <iostream>
#include <glog/logging.h>

namespace muduo {
    namespace net {
        class Acceptor;

        ///
        /// TCP server, supports single-threaded and thread-pool models.
        ///
        /// This is an interface class, so don't expose too much details.
        class TcpServer : noncopyable {
        public:

            enum Option {
                kNoReusePort,
                kReusePort,
            };

            //TcpServer(EventLoop* loop, const InetAddress& listenAddr);
            TcpServer(Dispatcher *dispatcher,
                      const InetAddress &listenAddr,
                      const string &nameArg,
                      Option option = kNoReusePort);

            ~TcpServer();  // force out-line dtor, for std::unique_ptr members.

            const string &ipPort() const { return ipPort_; }
            const string &name() const { return name_; }
            Dispatcher *dispatcher() const { return dispatcher_; }

            /// Set the number of threads for handling input.
            ///
            /// Always accepts new connection in loop's thread.
            /// Must be called before @c start
            /// @param numThreads
            /// - 0 means all I/O in loop's thread, no thread will created.
            ///   this is the default value.
            /// - 1 means all I/O in another thread.
            /// - N means a thread pool with N threads, new connections
            ///   are assigned on a round-robin basis.
            void setThreadNum(int numThreads);

            /// valid after calling start()
            //std::shared_ptr<EventLoopThreadPool> threadPool(){ return threadPool_; }

            /// Starts the server if it's not listening.
            ///
            /// It's harmless to call it multiple times.
            /// Thread safe.
            void start();

        private:
            /// Not thread safe, but in loop
            void newConnection(int sockfd, const InetAddress &peerAddr);

            /// Thread safe.
            void removeConnection(const TcpConnectionPtr &conn);
            /// Not thread safe, but in loop
            // void removeConnectionInLoop(const TcpConnectionPtr& conn);

            void doMessageDefault(const TcpConnectionPtr& conn, muduo::net::Buffer* buf);
            void doWriteCompleteDefault(const TcpConnectionPtr& conn);

            using ConnectionMap = std::map<string, TcpConnectionPtr>;

            Dispatcher *dispatcher_;  // the acceptor loop
            const string ipPort_;
            const string name_;
            std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
            //std::shared_ptr<EventLoopThreadPool> threadPool_;
            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;

            AtomicInt32 started_;
            // always in loop thread
            int nextConnId_;
            ConnectionMap connections_;

        };

    }  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_TCPSERVER_H
