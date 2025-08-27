#include "timer_impl.h"
#include <chrono>
#include "event2/event.h"
#include "cassert"
#include "Dispatcher.h"

namespace Event {

TimerImpl::TimerImpl(Libevent::BasePtr& libevent, TimerCb cb, Dispatcher& dispatcher)
    : cb_(cb), dispatcher_(dispatcher) {
  assert(cb_);
  evtimer_assign(
      &raw_event_, libevent.get(),
      [](evutil_socket_t, short, void* arg) -> void {
        TimerImpl* timer = static_cast<TimerImpl*>(arg);
        timer->cb_();
      },
      this);
}

void TimerImpl::disableTimer() {
//  assert(dispatcher_.isThreadSafe());
  event_del(&raw_event_);
}



void TimerImpl::internalEnableTimer(const timeval& tv) {
  assert(dispatcher_.isThreadSafe());

  event_add(&raw_event_, &tv);
}

bool TimerImpl::enabled() {
  assert(dispatcher_.isThreadSafe());
  return 0 != evtimer_pending(&raw_event_, nullptr);
}

} // namespace Event
