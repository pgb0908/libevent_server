// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <iostream>
#include "EventLoopThread.h"

using namespace muduo;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const string& name)
  : loop_(NULL),
    exiting_(false),
    thread_([this] { threadFunc(); }, name),
    mutex_(),
    cond_(mutex_),
    callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
  exiting_ = true;
/*  if (loop_ != NULL) // not 100% race-free, eg. threadFunc could be running callback_.
  {
    // still a tiny chance to call destructed object, if threadFunc exits just now.
    // but when EventLoopThread destructs, usually programming is exiting anyway.
    //loop_->quit();
    thread_.join();
  }*/

    thread_.join();
}

Event::DispatcherImp* EventLoopThread::startLoop()
{
    LOG(INFO) << "EventLoopThread,startLoop : " << thread_.name() <<std::endl;
    assert(!thread_.started());
    thread_.start(); // thread_local init

    std::cout << "startLoop before mutex  : "<< thread_.name() << std::endl;

    Event::DispatcherImp *loop = nullptr;
    {

        MutexLockGuard lock(mutex_);
        std::cout << "startLoop in mutex  : "<< thread_.name() << std::endl;
        while (loop_ == nullptr) {
            std::cout << "startLoop loop_ != nullptr : " << thread_.name() << std::endl;
            cond_.wait();
        }
        loop = loop_;
    }

    std::cout << "startLoop after mutex  : "<< thread_.name() << std::endl;

    return loop;
}

void EventLoopThread::threadFunc()
{
    LOG(INFO) << "EventLoopThread - threadFunc" << "[" << thread_.name() << "]";
    //loop_->dispatch_loop(Event::Dispatcher::RunType::Block);
    std::cout << "threadFunc start    : "<< thread_.name() << std::endl;

    Event::DispatcherImp loop;

    if (callback_) {
        callback_(&loop);
    }

    std::cout << "threadFunc mutex before    : "<< thread_.name() << std::endl;
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        std::cout << "threadFunc cond-notify   : " << thread_.name() << std::endl;
        cond_.notify();
    }
    std::cout << "threadFunc mutex after    : "<< thread_.name() << std::endl;

    loop_->dispatch_loop(Event::Dispatcher::RunType::Block);

    //assert(!exiting_);
    MutexLockGuard lock(mutex_);
    std::cout << "threadFunc loop_ = nullptr  : " << thread_.name() << std::endl;
    loop_ = nullptr;
    std::cout << "threadFunc end  : "<< thread_.name() << std::endl;
}

