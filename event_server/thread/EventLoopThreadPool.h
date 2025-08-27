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

        class EventLoopThreadPool {
        public:
            typedef std::function<void(Event::Dispatcher *)> ThreadInitCallback;

            EventLoopThreadPool(size_t threadNum, string nameArg);

            ~EventLoopThreadPool();

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
            size_t size() {
                return loopThreadVector_.size();
            }

            /**
             * @brief Get the next event loop in the pool.
             *
             * @return EventLoop*
             */
            Event::Dispatcher *getNextLoop();

            /**
             * @brief Get the event loop in the `id` position in the pool.
             *
             * @param id The id of the first event loop is zero. If the id >= the number
             * of event loops, nullptr is returned.
             * @return EventLoop*
             */
            Event::Dispatcher *getLoop(size_t id);

            /**
             * @brief Get all event loops in the pool.
             *
             * @return std::vector<EventLoop *>
             */
            std::vector<Event::Dispatcher *> getLoops() const;

        private:
            std::vector<std::shared_ptr<EventLoopThread>> loopThreadVector_;
            std::atomic<size_t> loopIndex_{0};
            string name_;
            bool started_;
        };

    }  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOPTHREADPOOL_H
