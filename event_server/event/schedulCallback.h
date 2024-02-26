//
// Created by bont on 24. 2. 26.
//

#ifndef LIBEVENT_SERVER_SCHEDULCALLBACK_H
#define LIBEVENT_SERVER_SCHEDULCALLBACK_H


#include <functional>
#include "ImplBase.h"
#include <memory>

class Dispatcher;

class schedulCallback : public ImplBase {
public:
    schedulCallback(Dispatcher& dispatcher, std::function<void()> cb);

    // SchedulableCallback implementation.
    void scheduleCallbackCurrentIteration();
    void scheduleCallbackNextIteration();
    void cancel();
    bool enabled();
private:
    Dispatcher& dispatcher_;
    std::function<void()> cb_;
};

using SchedulCallbackPtr = std::unique_ptr<schedulCallback>;

#endif //LIBEVENT_SERVER_SCHEDULCALLBACK_H
