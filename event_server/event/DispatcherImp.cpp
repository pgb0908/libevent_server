//
// Created by bont on 24. 2. 19.
//


#include "DispatcherImp.h"

#include <event2/event.h>
#include <iostream>

#include "event_server/common/Mutex.h"
#include "file_event_impl.h"
#include "schedulable_cb_impl.h"

namespace Event {

    DispatcherImp::DispatcherImp() :
            looping_(false),
            quit_(false),
            post_cb_(base_scheduler_.createSchedulableCallback([this]() {
                LOG(INFO) << "create schedule callback";
                runPostCallbacks();
            })),
            threadId_(muduo::CurrentThread::tid()) {

    }


    void DispatcherImp::dispatch_loop(Dispatcher::RunType type) {
        LOG(INFO) << "loop start";
        base_scheduler_.run(type);
        //event_base_dispatch(&base_scheduler_.base());

    }

    FileEventPtr
    DispatcherImp::createFileEvent(int fd, const FileReadyCb &cb, FileTriggerType trigger, uint32_t events) {
        auto fileEvent = std::make_unique<FileEventImpl>(*this, fd,
                                                     [this, cb](uint32_t events) {
                                                         cb(events);
                                                     },
                                                     trigger,
                                                     events);

        return fileEvent;
    }

    void DispatcherImp::assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    void DispatcherImp::abortNotInLoopThread() {
        LOG(ERROR) << "EventLoop::abortNotInLoopThread - EventLoop " << this
                   << " was created in threadId_ = " << threadId_
                   << ", current thread id = " << muduo::CurrentThread::tid();
    }

    const pid_t DispatcherImp::getThreadId() const {
        return threadId_;
    }

    void DispatcherImp::post(PostCb callback) {
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

    bool DispatcherImp::isThreadSafe() {
        return threadId_ == muduo::CurrentThread::tid();
    }


    SchedulableCallbackPtr DispatcherImp::createSchedulableCallback(const PostCb &cb) {
        assert(cb);
        auto callback = base_scheduler_.createSchedulableCallback(cb);

        return callback;
    }

    void DispatcherImp::printRegistEvent() {
        event_base_dump_events(&base(), stdout);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        createSchedulableCallback([this]() {
            printRegistEvent();
        });
    }


    void DispatcherImp::runPostCallbacks() {
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

    DispatcherImp::~DispatcherImp() {
        LOG(INFO) << "EventLoop " << this << " of thread " << threadId_
                  << " destructs in thread " << muduo::CurrentThread::tid();
    }

    void DispatcherImp::exit() {
        base_scheduler_.loopExit();
    }
}