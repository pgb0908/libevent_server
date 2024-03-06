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

EventLoopThreadPool::EventLoopThreadPool(size_t threadNum, string nameArg)
        :
          name_(std::move(nameArg)),
          started_(false),loopIndex_(0){

    for (size_t i = 0; i < threadNum; ++i)
    {
        std::string thread_name = name_ + "_" + std::to_string(i);
        loopThreadVector_.emplace_back(std::make_shared<EventLoopThread>(thread_name));
    }
}

EventLoopThreadPool::~EventLoopThreadPool() {
}

void EventLoopThreadPool::start() {
    assert(!started_);

    for (unsigned int i = 0; i < loopThreadVector_.size(); ++i)
    {
        loopThreadVector_[i]->run();
    }
    started_ = true;
}

void EventLoopThreadPool::wait() {
    for (unsigned int i = 0; i < loopThreadVector_.size(); ++i)
    {
        loopThreadVector_[i]->wait();
    }
    started_ = false;
}

Event::DispatcherImp *EventLoopThreadPool::getNextLoop() {
    if (loopThreadVector_.size() > 0)
    {
        size_t index = loopIndex_.fetch_add(1, std::memory_order_relaxed);
        Event::DispatcherImp *loop =
                loopThreadVector_[index % loopThreadVector_.size()]->getLoop();
        return loop;
    }
    return nullptr;
}

Event::DispatcherImp *EventLoopThreadPool::getLoop(size_t id) {
    if (id < loopThreadVector_.size())
        return loopThreadVector_[id]->getLoop();
    return nullptr;
}

std::vector<Event::DispatcherImp *> EventLoopThreadPool::getLoops() const {
    std::vector<Event::DispatcherImp *> ret;
    for (auto &loopThread : loopThreadVector_)
    {
        ret.push_back(loopThread->getLoop());
    }
    return ret;
}
