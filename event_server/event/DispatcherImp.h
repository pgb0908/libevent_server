//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_DISPATCHER_H
#define LIBEVENT_EXERCISE_DISPATCHER_H

#include "event_server/thread/CurrentThread.h"
#include "event_server/common/Mutex.h"
#include <glog/logging.h>
#include <list>
#include "dispatcher.h"
#include "libevent.h"
#include "libevent_scheduler.h"
#include "real_time_system.h"


namespace Event {

    class DispatcherImp {
    public:

        DispatcherImp();

        ~DispatcherImp();

        /**
         * 다른 io 쓰레드에게 명령
         * @param callback
         */
        void post(PostCb callback);

        bool isThreadSafe();

        FileEventPtr createFileEvent(int fd, const FileReadyCb &cb, FileTriggerType trigger, uint32_t events);
        SchedulableCallbackPtr createSchedulableCallback(const PostCb &);

        /**
         * run event-loop
         */
        void dispatch_loop(Dispatcher::RunType type);

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
