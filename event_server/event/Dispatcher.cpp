//
// Created by bont on 24. 2. 19.
//


#include "Dispatcher.h"

#include <event2/event.h>
#include <iostream>

#include "event_server/common/Mutex.h"
#include "file_event_impl.h"
#include "schedulable_cb_impl.h"
#include "real_time_system.h"

namespace Event {

    Dispatcher::Dispatcher() :
            name_(""),
            time_source_(RealTimeSystem()),
            //scheduler_(time_system.createScheduler(base_scheduler_, base_scheduler_)),
            threadId_(muduo::CurrentThread::tid()),
            deferred_delete_cb_(base_scheduler_.createSchedulableCallback([this]() -> void { clearDeferredDeleteList(); })),
            post_cb_(base_scheduler_.createSchedulableCallback([this]() {runPostCallbacks();})),
            current_to_delete_(&to_delete_1_)
         {

        updateApproximateMonotonicTimeInternal();
        base_scheduler_.registerOnPrepareCallback(
                std::bind(&Dispatcher::updateApproximateMonotonicTime, this));

    }


    FileEventPtr
    Dispatcher::createFileEvent(int fd, const FileReadyCb &cb, FileTriggerType trigger, uint32_t events) {
        auto fileEvent = std::make_unique<FileEventImpl>(*this, fd,
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
            std::lock_guard<std::mutex> guard(post_lock_);
            do_post = post_callbacks_.empty();
            post_callbacks_.push_back(std::move(callback));
        }

        if (do_post) {
            post_cb_->scheduleCallbackCurrentIteration();
        }
    }

    bool Dispatcher::isThreadSafe() const {
        return threadId_ == muduo::CurrentThread::tid();
    }


    SchedulableCallbackPtr Dispatcher::createSchedulableCallback(const PostCb &cb) {
        assert(cb);
        auto callback = base_scheduler_.createSchedulableCallback(cb);

        return callback;
    }

    void Dispatcher::printRegistEvent() {
        event_base_dump_events(&base(), stdout);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        createSchedulableCallback([this]() {
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
            std::lock_guard<std::mutex> guard(post_lock_);
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

    void Dispatcher::exit() {
        base_scheduler_.loopExit();
    }

    void Dispatcher::updateApproximateMonotonicTime() {
        updateApproximateMonotonicTimeInternal();
    }

    void Dispatcher::updateApproximateMonotonicTimeInternal() {
        approximate_monotonic_time_ = time_source_.monotonicTime();
    }

    void Dispatcher::runThreadLocalDelete() {

    }

    TimerPtr Dispatcher::createTimerInternal(TimerCb cb) {
        return scheduler_->createTimer(
                [this, cb]() {
                    cb();
                },
                *this);
    }

    MonotonicTime Dispatcher::approximateMonotonicTime() const {
        return approximate_monotonic_time_;
    }

    void Dispatcher::run(RunType type) {
        LOG(INFO) << "loop start";
        //run_tid_ = thread_factory_.currentThreadId();
        // Flush all post callbacks before we run the event loop. We do this because there are post
        // callbacks that have to get run before the initial event loop starts running. libevent does
        // not guarantee that events are run in any particular order. So even if we post() and call
        // event_base_once() before some other event, the other event might get called first.
        runPostCallbacks();
        base_scheduler_.run(type);
    }

    void Dispatcher::deleteInDispatcherThread(DispatcherThreadDeletableConstPtr deletable) {
/*        bool need_schedule;
        {
            Thread::LockGuard lock(thread_local_deletable_lock_);
            need_schedule = deletables_in_dispatcher_thread_.empty();
            deletables_in_dispatcher_thread_.emplace_back(std::move(deletable));
            // TODO(lambdai): Enable below after https://github.com/envoyproxy/envoy/issues/15072
            // ASSERT(!shutdown_called_, "inserted after shutdown");
        }

        if (need_schedule) {
            thread_local_delete_cb_->scheduleCallbackCurrentIteration();
        }*/
    }

    void Dispatcher::deferredDelete(DeferredDeletablePtr &&to_delete) {
        //ASSERT(isThreadSafe());
        if (to_delete != nullptr) {
            //to_delete->deleteIsPending();
            current_to_delete_->emplace_back(std::move(to_delete));
           // ENVOY_LOG(trace, "item added to deferred deletion list (size={})", current_to_delete_->size());
            if (current_to_delete_->size() == 1) {
                deferred_delete_cb_->scheduleCallbackCurrentIteration();
            }
        }
    }

    TimerPtr Dispatcher::createTimer(TimerCb cb) {
        //ASSERT(isThreadSafe());
        return createTimerInternal(cb);
    }

    void Dispatcher::clearDeferredDeleteList() {
        //ASSERT(isThreadSafe());
        std::vector<DeferredDeletablePtr>* to_delete = current_to_delete_;

        size_t num_to_delete = to_delete->size();
        if (deferred_deleting_ || !num_to_delete) {
            return;
        }

        //ENVOY_LOG(trace, "clearing deferred deletion list (size={})", num_to_delete);

        // Swap the current deletion vector so that if we do deferred delete while we are deleting, we
        // use the other vector. We will get another callback to delete that vector.
        if (current_to_delete_ == &to_delete_1_) {
            current_to_delete_ = &to_delete_2_;
        } else {
            current_to_delete_ = &to_delete_1_;
        }

        //touchWatchdog();
        deferred_deleting_ = true;

        // Calling clear() on the vector does not specify which order destructors run in. We want to
        // destroy in FIFO order so just do it manually. This required 2 passes over the vector which is
        // not optimal but can be cleaned up later if needed.
        for (size_t i = 0; i < num_to_delete; i++) {
            (*to_delete)[i].reset();
        }

        to_delete->clear();
        deferred_deleting_ = false;
    }
}