#pragma once

#include "timer.h"

namespace Event {

    /**
     * Real-world time implementation of TimeSource.
     */
    class RealTimeSource : public TimeSource {
    public:
        // TimeSource
        SystemTime systemTime() override { return std::chrono::system_clock::now(); }
        MonotonicTime monotonicTime() override { return std::chrono::steady_clock::now(); }
    };


    /**
     * Real-world time implementation of TimeSystem.
     */
    class RealTimeSystem : public TimeSystem {
    public:
        // TimeSystem
        SchedulerPtr createScheduler(Scheduler &, CallbackScheduler &) override;

        // TimeSource
        SystemTime systemTime() override { return time_source_.systemTime(); }
        MonotonicTime monotonicTime() override { return time_source_.monotonicTime(); }

    private:
        RealTimeSource time_source_;
    };

} // namespace Event
