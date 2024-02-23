// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H

#include "event_server/common/Types.h"
#include "event_server/common/noncopyable.h"
#include "event_server/event/Dispatcher.h"

#include <functional>
#include <memory>
#include <vector>

namespace muduo {

    namespace net {
        class EventLoopThread;

        class EventLoopThreadPool : noncopyable {
        public:
            typedef std::function<void(Dispatcher *)> ThreadInitCallback;

            EventLoopThreadPool(Dispatcher *baseLoop, string nameArg);
            ~EventLoopThreadPool();

            void setThreadNum(int numThreads) { numThreads_ = numThreads; }
            void start(const ThreadInitCallback &cb = ThreadInitCallback());

            // valid after calling start()
            /// round-robin
            Dispatcher *getNextLoop();

            /// with the same hash code, it will always return the same EventLoop
            Dispatcher *getLoopForHash(size_t hashCode);

            std::vector<Dispatcher *> getAllLoops();
            bool started() const { return started_; }
            const string &name() const { return name_; }

        private:
            Dispatcher *baseLoop_;
            string name_;
            bool started_;
            int numThreads_;
            int next_;
            std::vector<std::unique_ptr<EventLoopThread>> threads_;
            std::vector<Dispatcher *> loops_;
        };

    }  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOPTHREADPOOL_H
