#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "scope_tracker.h"
#include "file_event.h"
#include "timer.h"
#include "scaled_timer.h"
#include "deferred_deletable.h"
#include "signal.h"
#include "dispatcher_thread_deletable.h"

namespace Event {

/**
 * Callback invoked when a dispatcher post() runs.
 */
using PostCb = std::function<void()>;
using PostCbSharedPtr = std::shared_ptr<PostCb>;

/**
 * Minimal interface to the dispatching loop used to create low-level primitives. See Dispatcher
 * below for the full interface.
 */
class DispatcherBase {
public:
  virtual ~DispatcherBase() = default;

  /**
   * Posts a functor to the dispatcher. This is safe cross thread. The functor runs in the context
   * of the dispatcher event loop which may be on a different thread than the caller.
   */
  virtual void post(PostCb callback) = 0;

  /**
   * Validates that an operation is thread-safe with respect to this dispatcher; i.e. that the
   * current thread of execution is on the same thread upon which the dispatcher loop is running.
   */
  virtual bool isThreadSafe() const = 0;
};

/**
 * Minimal interface to support ScopeTrackedObjects.
 */
class ScopeTracker {
public:
  virtual ~ScopeTracker() = default;

  /**
   * Appends a tracked object to the current stack of tracked objects operating
   * in the dispatcher.
   *
   * It's recommended to use ScopeTrackerScopeState to manage the object's tracking. If directly
   * invoking, there needs to be a subsequent call to popTrackedObject().
   */
  virtual void pushTrackedObject(const ScopeTrackedObject* object) = 0;

  /**
   * Removes the top of the stack of tracked object and asserts that it was expected.
   */
  virtual void popTrackedObject(const ScopeTrackedObject* expected_object) = 0;

  /**
   * Whether the tracked object stack is empty.
   */
  virtual bool trackedObjectStackIsEmpty() const = 0;
};

/**
 * Abstract event dispatching loop.
 */
class Dispatcher : public DispatcherBase, public ScopeTracker {
public:
  /**
   * Returns the name that identifies this dispatcher, such as "worker_2" or "main_thread".
   * @return const std::string& the name that identifies this dispatcher.
   */
  virtual const std::string& name() = 0;

  /**
   * Creates a file event that will signal when a file is readable or writable. On UNIX systems this
   * can be used for any file like interface (files, sockets, etc.).
   * @param fd supplies the fd to watch.
   * @param cb supplies the callback to fire when the file is ready.
   * @param trigger specifies whether to edge or level trigger.
   * @param events supplies a logical OR of FileReadyType events that the file event should
   *               initially listen on.
   */
  virtual FileEventPtr createFileEvent(int fd, FileReadyCb cb, FileTriggerType trigger,
                                       uint32_t events) = 0;

  /**
   * Allocates a timer. @see Timer for docs on how to use the timer.
   * @param cb supplies the callback to invoke when the timer fires.
   */
  virtual Event::TimerPtr createTimer(TimerCb cb) = 0;

  /**
   * Allocates a scaled timer. @see Timer for docs on how to use the timer.
   * @param timer_type the type of timer to create.
   * @param cb supplies the callback to invoke when the timer fires.
   */
  virtual Event::TimerPtr createScaledTimer(Event::ScaledTimerType timer_type, TimerCb cb) = 0;

  /**
   * Allocates a scaled timer. @see Timer for docs on how to use the timer.
   * @param minimum the rule for computing the minimum value of the timer.
   * @param cb supplies the callback to invoke when the timer fires.
   */
  virtual Event::TimerPtr createScaledTimer(Event::ScaledTimerMinimum minimum, TimerCb cb) = 0;

  /**
   * Allocates a schedulable callback. @see SchedulableCallback for docs on how to use the wrapped
   * callback.
   * @param cb supplies the callback to invoke when the SchedulableCallback is triggered on the
   * event loop.
   */
  virtual Event::SchedulableCallbackPtr createSchedulableCallback(std::function<void()> cb) = 0;

  /**
   * Register a watchdog for this dispatcher. The dispatcher is responsible for touching the
   * watchdog at least once per touch interval. Dispatcher implementations may choose to touch more
   * often to avoid spurious miss events when processing long callback queues.
   * @param min_touch_interval Touch interval for the watchdog.
   */
  //virtual void registerWatchdog(const Server::WatchDogSharedPtr& watchdog,std::chrono::milliseconds min_touch_interval) = 0;

  /**
   * Returns a time-source to use with this dispatcher.
   */
  virtual TimeSource& timeSource() = 0;

  /**
   * Returns a recently cached MonotonicTime value.
   */
  virtual MonotonicTime approximateMonotonicTime() const = 0;

  /**
   * Initializes stats for this dispatcher. Note that this can't generally be done at construction
   * time, since the main and worker thread dispatchers are constructed before
   * ThreadLocalStoreImpl::initializeThreading.
   * @param scope the scope to contain the new per-dispatcher stats created here.
   * @param prefix the stats prefix to identify this dispatcher. If empty, the dispatcher will be
   *               identified by its name.
   */
  //virtual void initializeStats(Stats::Scope& scope,const absl::optional<std::string>& prefix = absl::nullopt) = 0;

  /**
   * Clears any items in the deferred deletion queue.
   */
  virtual void clearDeferredDeleteList() = 0;

  /**
   * Wraps an already-accepted socket in an instance of Envoy's server Network::Connection.
   * @param socket supplies an open file descriptor and connection metadata to use for the
   *        connection. Takes ownership of the socket.
   * @param transport_socket supplies a transport socket to be used by the connection.
   * @param stream_info info object for the server connection
   * @return Network::ConnectionPtr a server connection that is owned by the caller.
   */
/*  virtual Network::ServerConnectionPtr
  createServerConnection(Network::ConnectionSocketPtr&& socket,
                         Network::TransportSocketPtr&& transport_socket,
                         StreamInfo::StreamInfo& stream_info) = 0;*/

  /**
   * Creates an instance of Envoy's Network::ClientConnection. Does NOT initiate the connection;
   * the caller must then call connect() on the returned Network::ClientConnection.
   * @param address supplies the address to connect to.
   * @param source_address supplies an address to bind to or nullptr if no bind is necessary.
   * @param transport_socket supplies a transport socket to be used by the connection.
   * @param options the socket options to be set on the underlying socket before anything is sent
   *        on the socket.
   * @param transport socket options used to create the transport socket.
   * @return Network::ClientConnectionPtr a client connection that is owned by the caller.
   */
/*  virtual Network::ClientConnectionPtr createClientConnection(
      Network::Address::InstanceConstSharedPtr address,
      Network::Address::InstanceConstSharedPtr source_address,
      Network::TransportSocketPtr&& transport_socket,
      const Network::ConnectionSocket::OptionsSharedPtr& options,
      const Network::TransportSocketOptionsConstSharedPtr& transport_options) = 0;*/

  /**
   * @return Filesystem::WatcherPtr a filesystem watcher owned by the caller.
   */
  //virtual Filesystem::WatcherPtr createFilesystemWatcher() = 0;

  /**
   * Submits an item for deferred delete. @see DeferredDeletable.
   */
  virtual void deferredDelete(DeferredDeletablePtr&& to_delete) = 0;

  /**
   * Exits the event loop.
   */
  virtual void exit() = 0;

  /**
   * Listens for a signal event. Only a single dispatcher in the process can listen for signals.
   * If more than one dispatcher calls this routine in the process the behavior is undefined.
   *
   * @param signal_num supplies the signal to listen on.
   * @param cb supplies the callback to invoke when the signal fires.
   * @return SignalEventPtr a signal event that is owned by the caller.
   */
  virtual SignalEventPtr listenForSignal(int signal_num, SignalCb cb) = 0;

  /**
   * Post the deletable to this dispatcher. The deletable objects are guaranteed to be destroyed on
   * the dispatcher's thread before dispatcher destroy. This is safe cross thread.
   */
  virtual void deleteInDispatcherThread(DispatcherThreadDeletableConstPtr deletable) = 0;

  /**
   * Runs the event loop. This will not return until exit() is called either from within a callback
   * or from a different thread.
   * @param type specifies whether to run in blocking mode (run() will not return until exit() is
   *              called) or non-blocking mode where only active events will be executed and then
   *              run() will return.
   */
  enum class RunType {
    Block,       // Runs the event-loop until there are no pending events.
    NonBlock,    // Checks for any pending events to activate, executes them,
                 // then exits. Exits immediately if there are no pending or
                 // active events.
    RunUntilExit // Runs the event-loop until loopExit() is called, blocking
                 // until there are pending or active events.
  };
  virtual void run(RunType type) = 0;

  /**
   * Returns a factory which connections may use for watermark buffer creation.
   * @return the watermark buffer factory for this dispatcher.
   */
  //virtual Buffer::WatermarkFactory& getWatermarkFactory() = 0;

  /**
   * Updates approximate monotonic time to current value.
   */
  virtual void updateApproximateMonotonicTime() = 0;

  /**
   * Shutdown the dispatcher by clear dispatcher thread deletable.
   */
  virtual void shutdown() = 0;
};

using DispatcherPtr = std::unique_ptr<Dispatcher>;

} // namespace Event
