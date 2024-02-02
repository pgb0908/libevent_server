#pragma once

#include <memory>

#include "dispatcher.h"
#include "deferred_deletable.h"

namespace Envoy {
namespace Event {

/**
 * A util to schedule a task to run in a future event loop cycle. One of the use cases is to run the
 * task after the previously DeferredDeletable objects are destroyed.
 */
class DeferredTaskUtil {
private:
  class DeferredTask : public DeferredDeletable {
  public:
      DeferredTask() = default;
    DeferredTask(std::function<void()>&& task) : task_(std::move(task)) {}
    ~DeferredTask() override { task_(); }

  private:
    std::function<void()> task_;
  };

public:
  /**
   * Submits an item for run deferred delete.
   */
  static void deferredRun(Dispatcher& dispatcher, std::function<void()>&& func) {
      std::unique_ptr<DeferredDeletable> deferredDeletable(new DeferredTask(std::move(func)));
      dispatcher.deferredDelete(std::move(deferredDeletable));
  }
};

} // namespace Event
} // namespace Envoy
