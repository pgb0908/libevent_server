// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "EventLoopThread.h"

using namespace muduo;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const string& name)
  : loop_(Event::DispatcherImp()),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
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
    LOG(INFO) << "EventLoopThread,startLoop";
    assert(!thread_.started());
    thread_.start(); // thread_local init

/*    Dispatcher *loop = nullptr;
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == NULL) {
            cond_.wait();
        }
        loop = loop_;
    }*/

    return &loop_;
}

void EventLoopThread::threadFunc()
{
    LOG(INFO) << "EventLoopThread - threadFunc" << "[" << thread_.name() << "]";
    loop_.dispatch_loop();
    //loop_->dispatch_loop();
/*  Dispatcher loop;

  if (callback_)
  {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.dispatch_loop();
  assert(!exiting_);
  MutexLockGuard lock(mutex_);
  loop_ = NULL;*/
}

