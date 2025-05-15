//
// Created by bont on 24. 3. 5.
//

#ifndef LIBEVENT_SERVER_WORKERIMPL_H
#define LIBEVENT_SERVER_WORKERIMPL_H


#include "event_server/event/Dispatcher.h"
#include "Thread.h"

class WorkerImpl {
    using dispatcherPtr = std::unique_ptr<Event::Dispatcher>;
    using threadPtr = std::unique_ptr<muduo::Thread>;
public:
    explicit WorkerImpl(const std::function<void()>& cb, std::string name);
    ~WorkerImpl();

    void start();


private:
    void threadRoutine(const std::function<void()>& cb);

    dispatcherPtr dispatcherPtr_;
    threadPtr threadPtr_;
    std::function<void()> cb_;
    std::string name_;
};


#endif //LIBEVENT_SERVER_WORKERIMPL_H
