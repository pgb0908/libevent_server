//
// Created by bont on 24. 2. 26.
//

#include "schedulCallback.h"

#include <utility>
#include <event2/event.h>

schedulCallback::schedulCallback(Dispatcher &dispatcher, std::function<void()> cb):
    dispatcher_(dispatcher), cb_(std::move(cb)){

}

void schedulCallback::scheduleCallbackCurrentIteration() {
    if (enabled()) {
        return;
    }
    // event_active directly adds the event to the end of the work queue so it executes in the current
    // iteration of the event loop.
    event_active(&raw_event_, EV_TIMEOUT, 0);
}

void schedulCallback::scheduleCallbackNextIteration() {
    if (enabled()) {
        return;
    }
    // libevent computes the list of timers to move to the work list after polling for fd events, but
    // iteration through the work list starts. Zero delay timers added while iterating through the
    // work list execute on the next iteration of the event loop.
    const timeval zero_tv{};
    event_add(&raw_event_, &zero_tv);
}

void schedulCallback::cancel() {
    event_del(&raw_event_);
}

bool schedulCallback::enabled() {
    return 0 != evtimer_pending(&raw_event_, nullptr);
}


