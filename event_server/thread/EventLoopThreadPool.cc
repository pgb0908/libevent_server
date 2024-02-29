// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

#include <stdio.h>

#include <utility>

using namespace muduo;
using namespace muduo::net;

EventLoopThreadPool::EventLoopThreadPool(Event::DispatcherImp *baseLoop, string nameArg)
        : baseLoop_(baseLoop),
          name_(std::move(nameArg)),
          started_(false),
          numThreads_(0),
          next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
    LOG(INFO) << "EventLoopThreadPool - start";
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        auto *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

Event::DispatcherImp *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    Event::DispatcherImp *loop = baseLoop_;

    if (!loops_.empty()) {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (implicit_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

Event::DispatcherImp *EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    baseLoop_->assertInLoopThread();
    Event::DispatcherImp *loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<Event::DispatcherImp *> EventLoopThreadPool::getAllLoops() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty()) {
        return std::vector<Event::DispatcherImp *>(1, baseLoop_);
    } else {
        return loops_;
    }
}
