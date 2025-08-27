// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_EVENTLOOPTHREAD_H
#define MUDUO_NET_EVENTLOOPTHREAD_H

#include <mutex>
#include <future>
#include "event_server/common/Mutex.h"
#include "event_server/common/Condition.h"
#include "event_server/common/CountDownLatch.h"
#include "event_server/event/Dispatcher.h"

#include "Thread.h"

namespace muduo {
    namespace net {

        class EventLoopThread : noncopyable {
        public:
            typedef std::function<void(Event::Dispatcher *)> ThreadInitCallback;

            EventLoopThread(const string &name = string());

            ~EventLoopThread();

            //Event::DispatcherImp *startLoop();

            /**
             * @brief Wait for the event loop to exit.
             * @note This method blocks the current thread until the event loop exits.
             */
            void wait();

            /**
             * @brief Get the pointer of the event loop of the thread.
             *
             * @return EventLoop*
             */
            Event::Dispatcher *getLoop() const {
                return loop_.get();
            }

            /**
             * @brief Run the event loop of the thread. This method doesn't block the
             * current thread.
             *
             */
            void run();

        private:
            void threadFunc();
            std::shared_ptr<Event::Dispatcher> loop_;
            std::mutex loopMutex_;
            std::thread thread_;
            std::string loopThreadName_;
            std::promise<std::shared_ptr<Event::Dispatcher>> promiseForLoopPointer_;
            std::promise<int> promiseForRun_;
            std::promise<int> promiseForLoop_;
            std::once_flag once_;
        };

    }  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOPTHREAD_H

