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

#include "event_server/common/Mutex.h"
#include "event_server/common/Condition.h"
#include "event_server/common/CountDownLatch.h"
#include "event_server/event/Dispatcher.h"

#include "Thread.h"

namespace muduo {
    namespace net {

        class EventLoopThread : noncopyable {
        public:
            typedef std::function<void(Dispatcher *)> ThreadInitCallback;

            EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                            const string &name = string());

            ~EventLoopThread();

            Dispatcher *startLoop();

        private:
            void threadFunc();

            Dispatcher loop_;
            bool exiting_;
            Thread thread_;
            MutexLock mutex_;
            Condition cond_ GUARDED_BY(mutex_);
            ThreadInitCallback callback_;
        };

    }  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOPTHREAD_H

