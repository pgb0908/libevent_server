//
// Created by bont on 24. 2. 26.
//

#ifndef LIBEVENT_SERVER_SCHEDULARCALLBACK_H
#define LIBEVENT_SERVER_SCHEDULARCALLBACK_H


#include "SchedularCallbackInterface.h"
#include "Dispatcher.h"

class Dispatcher;

class SchedularCallback : public SchedularCallback, ImplBase  {
public:
    SchedularCallback(Dispatcher& dispatcher, std::function<void()> cb);
    ~SchedularCallback();




private:
    Dispatcher& dispatcher_;
    std::function<void()> cb_;
};

#endif //LIBEVENT_SERVER_SCHEDULARCALLBACK_H
