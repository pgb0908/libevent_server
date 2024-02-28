//
// Created by bont on 24. 2. 19.
//

#include <event2/event.h>
#include <iostream>
#include "Dispatcher.h"
#include "event_server/common/Mutex.h"
#include "SchedularCallback.h"

Dispatcher::Dispatcher() :
        looping_(false),
        quit_(false),
        post_cb_(createSchedulableCallback([this](){
            LOG(INFO) << "create schedule callback";
            runPostCallbacks();
        })),
        threadId_(muduo::CurrentThread::tid()) {
    event_base *event_base = event_base_new();
    libevent_ = BasePtr(event_base);
}


void Dispatcher::dispatch_loop() {
    LOG(INFO) << "loop start";
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
    auto callback = std::make_unique<SchedularCallback>(*this, cb);

    return callback;
}

void Dispatcher::printRegistEvent() {
    event_base_dump_events(&base(), stdout);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    createSchedulableCallback([this](){
        printRegistEvent();
    });
}


void Dispatcher::runPostCallbacks() {
    // Clear the deferred delete list before running post callbacks to reduce non-determinism in
    // callback processing, and more easily detect if a scheduled post callback refers to one of the
    // objects that is being deferred deleted.
    //clearDeferredDeleteList();

    std::list<PostCb> callbacks;
    {
        // Take ownership of the callbacks under the post_lock_. The lock must be released before
        // callbacks execute. Callbacks added after this transfer will re-arm post_cb_ and will execute
        // later in the event loop.
        //Thread::LockGuard lock(post_lock_);
        muduo::MutexLockGuard guard(mutex_);
        callbacks = std::move(post_callbacks_);
        // post_callbacks_ should be empty after the move.
        assert(post_callbacks_.empty());
    }
    // It is important that the execution and deletion of the callback happen while post_lock_ is not
    // held. Either the invocation or destructor of the callback can call post() on this dispatcher.
    while (!callbacks.empty()) {
        // Touch the watchdog before executing the callback to avoid spurious watchdog miss events when
        // executing a long list of callbacks.
        //touchWatchdog();
        // Run the callback.
        callbacks.front()();
        // Pop the front so that the destructor of the callback that just executed runs before the next
        // callback executes.
        callbacks.pop_front();
    }
}

Dispatcher::~Dispatcher() {
    LOG(INFO) << "EventLoop " << this << " of thread " << threadId_
              << " destructs in thread " << muduo::CurrentThread::tid();
}
