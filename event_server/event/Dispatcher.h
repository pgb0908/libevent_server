//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_DISPATCHER_H
#define LIBEVENT_EXERCISE_DISPATCHER_H

#include "Libevent.h"
#include "FileEvent.h"
#include "event_server/thread/CurrentThread.h"
#include "event_server/common/Mutex.h"
#include "schedulCallback.h"
#include <glog/logging.h>
#include <list>

using PostCb = std::function<void()>;

class Dispatcher {
public:

    Dispatcher();

    /**
     * 다른 io 쓰레드에게 명령
     * @param callback
     */
    void post(PostCb callback);

    bool isThreadSafe();


    FileEventPtr createFileEvent(int fd,  const FileReadyCb& cb, FileTriggerType trigger, uint32_t events);

    SchedulCallbackPtr createSchedulableCallback(const PostCb&);
    /**
     * run event-loop
     */
    void dispatch_loop();

    void assertInLoopThread();

    const pid_t getThreadId() const;

    event_base &base() {
        return *libevent_;
    }

    bool isInLoopThread() const {
        return threadId_ == muduo::CurrentThread::tid();
    }

    const std::string &getName() const;

    void printRegistEvent();


private:
    void abortNotInLoopThread();

    bool looping_; /* atomic */
    std::atomic<bool> quit_;
    BasePtr libevent_;
    const pid_t threadId_;
    std::list<PostCb> post_callbacks_;
    muduo::MutexLock mutex_;
    SchedulCallbackPtr post_cb_;


};


#endif //LIBEVENT_EXERCISE_DISPATCHER_H
