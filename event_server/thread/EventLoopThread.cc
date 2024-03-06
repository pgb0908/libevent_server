// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <iostream>
#include "EventLoopThread.h"
#include <sys/prctl.h>

using namespace muduo;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const string& name)
  : loop_(nullptr),
    thread_([this]() { threadFunc(); }),
    loopThreadName_(name)
{
    auto f = promiseForLoopPointer_.get_future();
    loop_ = f.get();
}

EventLoopThread::~EventLoopThread()
{
    run();
    std::shared_ptr<Event::DispatcherImp> loop;
    {
        std::unique_lock<std::mutex> lk(loopMutex_);
        loop = loop_;
    }
    if (loop)
    {
        loop->exit();
    }
    if (thread_.joinable())
    {
        thread_.join();
    }
}


void EventLoopThread::threadFunc()
{
    LOG(INFO) <<"worker entering dispatch loop";
    ::prctl(PR_SET_NAME, loopThreadName_.c_str());
    thread_local static std::shared_ptr<Event::DispatcherImp> loop =
            std::make_shared<Event::DispatcherImp>();
    //loop->queueInLoop([this]() { promiseForLoop_.set_value(1); });
    loop->post([this]() {
        promiseForLoop_.set_value(1); });
    promiseForLoopPointer_.set_value(loop);
    auto f = promiseForRun_.get_future();
    (void)f.get();
    loop->dispatch_loop(Event::Dispatcher::RunType::RunUntilExit);
    LOG(INFO) <<"worker exited dispatch loop";
    {
        std::unique_lock<std::mutex> lk(loopMutex_);
        //loop_->shutdown();
        loop_ = nullptr;
    }
}

void EventLoopThread::wait() {
    thread_.join();
}

void EventLoopThread::run() {
    std::call_once(once_, [this]() {
        auto f = promiseForLoop_.get_future();
        promiseForRun_.set_value(1);
        // Make sure the event loop loops before returning.
        (void)f.get();
    });
}

