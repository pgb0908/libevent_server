//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_DISPATCHER_H
#define LIBEVENT_EXERCISE_DISPATCHER_H

#include "event_server/thread/CurrentThread.h"
#include "event_server/common/Mutex.h"
#include <glog/logging.h>
#include <list>
#include <mutex>
#include "libevent.h"
#include "libevent_scheduler.h"
#include "real_time_system.h"
#include "file_event.h"

#include "deferred_deletable.h"


namespace Event {

    /**
     * Callback invoked when a dispatcher post() runs.
     */
    using PostCb = std::function<void()>;
    using PostCbSharedPtr = std::shared_ptr<PostCb>;

    /**
     * Runs the event loop. This will not return until exit() is called either from within a callback
     * or from a different thread.
     * @param type specifies whether to run in blocking mode (run() will not return until exit() is
     *              called) or non-blocking mode where only active events will be executed and then
     *              run() will return.
     */
    enum class RunType {
        Block,       // Runs the event-loop until there are no pending events.
        NonBlock,    // Checks for any pending events to activate, executes them,
        // then exits. Exits immediately if there are no pending or
        // active events.
        RunUntilExit // Runs the event-loop until loopExit() is called, blocking
        // until there are pending or active events.
    };

    class DispatcherThreadDeletable {
    public:
        virtual ~DispatcherThreadDeletable() = default;
    };

    using DispatcherThreadDeletableConstPtr = std::unique_ptr<const DispatcherThreadDeletable>;



    class Dispatcher {
    public:
        Dispatcher();
        ~Dispatcher();

        void assertInLoopThread();
        const pid_t getThreadId() const;
        bool isInLoopThread() const {
            return threadId_ == muduo::CurrentThread::tid();
        }
        void printRegistEvent();

        event_base &base() {
            return base_scheduler_.base();
        }
        const std::string& name() { return name_; }
        TimeSource& timeSource() { return time_source_; }
        void clearDeferredDeleteList();
        FileEventPtr createFileEvent(int fd, const FileReadyCb &cb, FileTriggerType trigger, uint32_t events);
        TimerPtr createTimer(TimerCb cb);


        SchedulableCallbackPtr createSchedulableCallback(const PostCb &);
        void deferredDelete(DeferredDeletablePtr&& to_delete);
        void exit();
        void post(PostCb callback);
        bool isThreadSafe() const;
        void deleteInDispatcherThread(DispatcherThreadDeletableConstPtr deletable);
        void run(RunType type);

        MonotonicTime approximateMonotonicTime() const;
        void updateApproximateMonotonicTime();

    private:

        const std::string name_;

        TimerPtr createTimerInternal(TimerCb cb);
        void updateApproximateMonotonicTimeInternal();

        void abortNotInLoopThread();

        void runPostCallbacks();
        void runThreadLocalDelete();

        LibeventScheduler base_scheduler_;
        SchedulerPtr scheduler_;

        const pid_t threadId_;

        bool shutdown_called_{false};

        std::mutex post_lock_;
        std::list<PostCb> post_callbacks_;
        SchedulableCallbackPtr post_cb_;

        MonotonicTime approximate_monotonic_time_;
        Event::RealTimeSystem time_source_;

        SchedulableCallbackPtr deferred_delete_cb_;
        std::vector<DeferredDeletablePtr> to_delete_1_;
        std::vector<DeferredDeletablePtr> to_delete_2_;
        std::vector<DeferredDeletablePtr>* current_to_delete_;
        bool deferred_deleting_{};


    };
}

#endif //LIBEVENT_EXERCISE_DISPATCHER_H
