//
// Created by bont on 24. 2. 26.
//

#include "SchedularCallback.h"

#include <utility>
#include <event2/event.h>
#include <cassert>
#include "Dispatcher.h"

SchedularCallback::SchedularCallback(Dispatcher &dispatcher, std::function<void()> cb):
    dispatcher_(dispatcher), cb_(std::move(cb)){
    assert(cb_);

    evtimer_assign(
            &raw_event_, &dispatcher.base(),
            [](evutil_socket_t, short, void* arg) -> void {
                SchedularCallback* cb = static_cast<SchedularCallback*>(arg);
                cb->cb_();
            },
            this);
}

SchedularCallback::~SchedularCallback() {

}




