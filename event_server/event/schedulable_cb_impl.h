#pragma once

#include "schedulable_cb.h"

#include "event_impl_base.h"
#include "libevent.h"

namespace Envoy {
namespace Event {

class DispatcherImpl;

/**
 * libevent implementation of SchedulableCallback.
 */
class SchedulableCallbackImpl : public SchedulableCallback, ImplBase {
public:
  SchedulableCallbackImpl(Libevent::BasePtr& libevent, std::function<void()> cb);

  // SchedulableCallback implementation.
  void scheduleCallbackCurrentIteration() override;
  void scheduleCallbackNextIteration() override;
  void cancel() override;
  bool enabled() override;

private:
  std::function<void()> cb_;
};

} // namespace Event
} // namespace Envoy
