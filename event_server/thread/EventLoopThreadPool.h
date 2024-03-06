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
#include "event_server/event/DispatcherImp.h"

#include <functional>
#include <memory>
#include <vector>

namespace muduo {

    namespace net {
        class EventLoopThread;

        class EventLoopThreadPool {
        public:
            typedef std::function<void(Event::DispatcherImp *)> ThreadInitCallback;

            EventLoopThreadPool(size_t threadNum, string nameArg);
            ~EventLoopThreadPool();

/*
            void setThreadNum(int numThreads) { numThreads_ = numThreads; }
            void start(const ThreadInitCallback &cb = ThreadInitCallback());

            // valid after calling start()
            /// round-robin
            Event::DispatcherImp *getNextLoop();

            /// with the same hash code, it will always return the same EventLoop
            Event::DispatcherImp *getLoopForHash(size_t hashCode);

            std::vector<Event::DispatcherImp *> getAllLoops();
            bool started() const { return started_; }
            const string &name() const { return name_; }
*/


            /**
              * @brief Run all event loops in the pool.
              * @note This function doesn't block the current thread.
              */
            void start();

            /**
             * @brief Wait for all event loops in the pool to quit.
             *
             * @note This function blocks the current thread.
             */
            void wait();

            /**
             * @brief Return the number of the event loop.
             *
             * @return size_t
             */
            size_t size()
            {
                return loopThreadVector_.size();
            }

            /**
             * @brief Get the next event loop in the pool.
             *
             * @return EventLoop*
             */
            Event::DispatcherImp *getNextLoop();

            /**
             * @brief Get the event loop in the `id` position in the pool.
             *
             * @param id The id of the first event loop is zero. If the id >= the number
             * of event loops, nullptr is returned.
             * @return EventLoop*
             */
            Event::DispatcherImp *getLoop(size_t id);

            /**
             * @brief Get all event loops in the pool.
             *
             * @return std::vector<EventLoop *>
             */
            std::vector<Event::DispatcherImp *> getLoops() const;

        private:
            std::vector<std::shared_ptr<EventLoopThread>> loopThreadVector_;
            std::atomic<size_t> loopIndex_{0};
            //Event::DispatcherImp *baseLoop_;
            string name_;
            bool started_;
        };

    }  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOPTHREADPOOL_H
