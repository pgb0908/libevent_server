//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_DISPATCHER_H
#define LIBEVENT_EXERCISE_DISPATCHER_H

#include "event_server/thread/CurrentThread.h"
#include "event_server/common/Mutex.h"
#include <glog/logging.h>
#include <list>
#include "libevent.h"
#include "libevent_scheduler.h"
#include "real_time_system.h"
#include "file_event.h"


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


    class DispatcherImp {
    public:

        DispatcherImp();
        ~DispatcherImp();

        // dispatcherBase
        void post(PostCb callback);
        bool isThreadSafe() const;

        FileEventPtr createFileEvent(int fd, const FileReadyCb &cb, FileTriggerType trigger, uint32_t events);
        SchedulableCallbackPtr createSchedulableCallback(const PostCb &);

        /**
         * run event-loop
         */
        void dispatch_loop(RunType type);

        void assertInLoopThread();

        const pid_t getThreadId() const;

        event_base &base() {
            return base_scheduler_.base();
        }

        bool isInLoopThread() const {
            return threadId_ == muduo::CurrentThread::tid();
        }

        void printRegistEvent();

        void exit();


    private:
        void updateApproximateMonotonicTime();
        void updateApproximateMonotonicTimeInternal();

        void abortNotInLoopThread();

        void runPostCallbacks();

        bool looping_; /* atomic */
        std::atomic<bool> quit_;
        LibeventScheduler base_scheduler_;
        const pid_t threadId_;
        std::list<PostCb> post_callbacks_;
        muduo::MutexLock mutex_;
        SchedulableCallbackPtr post_cb_;
        MonotonicTime approximate_monotonic_time_;
        Event::RealTimeSystem time_source_;


    };
}

#endif //LIBEVENT_EXERCISE_DISPATCHER_H
