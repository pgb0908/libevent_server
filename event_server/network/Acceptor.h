// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include <functional>

#include "Socket.h"
#include "event_server/event/Dispatcher.h"

namespace muduo {
    namespace net {

        class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
        class Acceptor : noncopyable {
        public:
            typedef std::function<void(int sockfd, const InetAddress &)> NewConnectionCallback;

            Acceptor(Event::Dispatcher *dispatcher, const InetAddress &listenAddr, bool reuseport);

            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; }

            void listen();

            bool listening() const { return listening_; }

            // Deprecated, use the correct spelling one above.
            // Leave the wrong spelling here in case one needs to grep it for error messages.
            // bool listenning() const { return listening(); }

            void handleRead();

        private:
            Event::Dispatcher *dispatcher_;
            Event::FileEventPtr accept_event_;
            Socket acceptSocket_;
            NewConnectionCallback newConnectionCallback_;
            bool listening_;
            int idleFd_;
        };

    }  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_ACCEPTOR_H
