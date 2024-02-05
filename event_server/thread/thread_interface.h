#pragma once

#include <functional>
#include <limits>
#include <memory>
#include <string>

#include "absl/base/thread_annotations.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace Envoy {
namespace Thread {

/**
 * An id for a thread.
 */
class ThreadId {
public:
  ThreadId() : id_(std::numeric_limits<int64_t>::min()) {}
  explicit ThreadId(int64_t id) : id_(id) {}

  int64_t getId() const { return id_; }
  std::string debugString() const { return std::to_string(id_); }
  bool isEmpty() const { return *this == ThreadId(); }
  friend bool operator==(ThreadId lhs, ThreadId rhs) { return lhs.id_ == rhs.id_; }
  friend bool operator!=(ThreadId lhs, ThreadId rhs) { return lhs.id_ != rhs.id_; }
  template <typename H> friend H AbslHashValue(H h, ThreadId id) {
    return H::combine(std::move(h), id.id_);
  }

private:
  int64_t id_;
};

class Thread {
public:
  virtual ~Thread() = default;

  /**
   * @return the name of the thread.
   */
  virtual std::string name() const = 0;

  /**
   * Blocks until the thread exits.
   */
  virtual void join() = 0;
};

using ThreadPtr = std::unique_ptr<Thread>;

// Options specified during thread creation.
struct Options {
  std::string name_; // A name supplied for the thread. On Linux this is limited to 15 chars.
};

using OptionsOptConstRef = const absl::optional<Options>&;

/**
 * Interface providing a mechanism for creating threads.
 */
class ThreadFactory {
public:
  virtual ~ThreadFactory() = default;

  /**
   * Creates a thread, immediately starting the thread_routine.
   *
   * @param thread_routine supplies the function to invoke in the thread.
   * @param options supplies options specified on thread creation.
   */
  virtual ThreadPtr createThread(std::function<void()> thread_routine,
                                 OptionsOptConstRef options = absl::nullopt) = 0;

  /**
   * Return the current system thread ID
   */
  virtual ThreadId currentThreadId() = 0;
};

using ThreadFactoryPtr = std::unique_ptr<ThreadFactory>;

/**
 * Like the C++11 "basic lockable concept" but a = 0 virtual interface vs. a template, and
 * with thread annotations.
 */
class ABSL_LOCKABLE BasicLockable {
public:
  virtual ~BasicLockable() = default;

  virtual void lock() ABSL_EXCLUSIVE_LOCK_FUNCTION() = 0;
  virtual bool tryLock() ABSL_EXCLUSIVE_TRYLOCK_FUNCTION(true) = 0;
  virtual void unlock() ABSL_UNLOCK_FUNCTION() = 0;
};

} // namespace Thread
} // namespace Envoy
