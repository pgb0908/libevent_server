//
// Created by bont on 24. 2. 19.
//

#include <event2/event.h>
#include <iostream>
#include "Dispatcher.h"
#include "event_server/common/Mutex.h"

Dispatcher::Dispatcher() :
        looping_(false),
        quit_(false),
        threadId_(muduo::CurrentThread::tid()) {
    event_base *event_base = event_base_new();
    libevent_ = BasePtr(event_base);
}


void Dispatcher::dispatch_loop() {
    std::cout << "event loop~~~" << std::endl;
    event_base_dispatch(libevent_.get());

}

FileEventPtr Dispatcher::createFileEvent(int fd, const FileReadyCb &cb, FileTriggerType trigger, uint32_t events) {
    auto fileEvent = std::make_unique<FileEvent>(*this, fd,
                                                 [this, cb](uint32_t events) {
                                                     cb(events);
                                                 },
                                                 trigger,
                                                 events);

    return fileEvent;
}

void Dispatcher::assertInLoopThread() {
    if (!isInLoopThread()) {
        abortNotInLoopThread();
    }
}

void Dispatcher::abortNotInLoopThread() {
    LOG(ERROR) << "EventLoop::abortNotInLoopThread - EventLoop " << this
               << " was created in threadId_ = " << threadId_
               << ", current thread id = " << muduo::CurrentThread::tid();
}

const pid_t Dispatcher::getThreadId() const {
    return threadId_;
}

void Dispatcher::post(PostCb callback) {
    bool do_post;
    {
        //Thread::LockGuard lock(post_lock_);
        muduo::MutexLockGuard guard(mutex_);
        do_post = post_callbacks_.empty();
        post_callbacks_.push_back(std::move(callback));
    }

    if (do_post) {
        post_cb_->scheduleCallbackCurrentIteration();
    }
}

bool Dispatcher::isThreadSafe() {
    return threadId_ == muduo::CurrentThread::tid();
}


SchedulCallbackPtr Dispatcher::createSchedulableCallback(const PostCb& cb) {
    assert(cb);
    auto callback = std::make_unique<schedulCallback>(*this, cb);

    return callback;
}

void Dispatcher::printRegistEvent() {
    event_base_dump_events(&base(), stdout);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    createSchedulableCallback([this](){
        printRegistEvent();
    });
}
