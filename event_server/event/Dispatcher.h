//
// Created by bont on 24. 2. 19.
//

#ifndef LIBEVENT_EXERCISE_DISPATCHER_H
#define LIBEVENT_EXERCISE_DISPATCHER_H

#include "Libevent.h"
#include "FileEvent.h"
#include "event_server/thread/CurrentThread.h"
#include <glog/logging.h>

class Dispatcher {
public:

    Dispatcher();
    FileEventPtr createFileEvent(int fd,  const FileReadyCb& cb, FileTriggerType trigger, uint32_t events);

    void dispatch_loop();
    event_base& base() { return *libevent_; }

    void assertInLoopThread();
    bool isInLoopThread() const { return threadId_ == muduo::CurrentThread::tid(); }

private:
    void abortNotInLoopThread();

    bool looping_; /* atomic */
    std::atomic<bool> quit_;
    BasePtr libevent_;
    const pid_t threadId_;


};


#endif //LIBEVENT_EXERCISE_DISPATCHER_H
