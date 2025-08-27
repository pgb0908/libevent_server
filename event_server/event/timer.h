#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include "schedulable_cb.h"


namespace Event {

    class Dispatcher;

    /**
     * Callback invoked when a timer event fires.
     */
    using TimerCb = std::function<void()>;

    /**
     * An abstract timer event. Free the timer to unregister any pending timeouts. Must be freed before
     * the dispatcher is torn down.
     */
    class Timer {
    public:
        virtual ~Timer() = default;

        /**
         * Disable a pending timeout without destroying the underlying timer.
         */
        virtual void disableTimer() = 0;

        /**
         * Return whether the timer is currently armed.
         */
        virtual bool enabled() = 0;
    };

    using TimerPtr = std::unique_ptr<Timer>;

    class Scheduler {
    public:
        virtual ~Scheduler() = default;

        /**
         * Creates a timer.
         */
        virtual TimerPtr createTimer(const TimerCb &cb, Dispatcher &dispatcher) = 0;
    };

    using SchedulerPtr = std::unique_ptr<Scheduler>;


    /**
     * Less typing for common system time and steady time type.
     *
     * SystemTime should be used when getting a time to present to the user, e.g. for logging.
     * MonotonicTime should be used when tracking time for computing an interval.
     */
    using Seconds = std::chrono::seconds;
    using SystemTime = std::chrono::time_point<std::chrono::system_clock>;
    using MonotonicTime = std::chrono::time_point<std::chrono::steady_clock>;

    /**
     * Captures a system-time source, capable of computing both monotonically increasing
     * and real time.
     */
    class TimeSource {
    public:
        virtual ~TimeSource() = default;

        /**
         * @return the current system time; not guaranteed to be monotonically increasing.
         */
        virtual SystemTime systemTime() = 0;

        /**
         * @return the current monotonic time.
         */
        virtual MonotonicTime monotonicTime() = 0;
    };


    /**
     * Interface providing a mechanism to measure time and set timers that run callbacks
     * when the timer fires.
     */
    class TimeSystem : public TimeSource {
    public:
        ~TimeSystem() override = default;

        using Duration = MonotonicTime::duration;
        using Nanoseconds = std::chrono::nanoseconds;
        using Microseconds = std::chrono::microseconds;
        using Milliseconds = std::chrono::milliseconds;
        using Seconds = std::chrono::seconds;

        /**
         * Creates a timer factory. This indirection enables thread-local timer-queue management,
         * so servers can have a separate timer-factory in each thread.
         */
        virtual SchedulerPtr createScheduler(Scheduler &base_scheduler, CallbackScheduler &cb_scheduler) = 0;
    };

} // namespace Event
