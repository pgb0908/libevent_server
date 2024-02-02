cmake_minimum_required(VERSION 3.26)
project(libevent_server)

set(CMAKE_CXX_STANDARD 11)

find_package(Libevent REQUIRED)

# Import Abseil's CMake targets
find_package(absl REQUIRED)


set(event_server_lib
        event_server/event/dispatcher_impl.cc
        event_server/event/event_impl_base.cc
        event_server/event/file_event_impl.cc
        event_server/event/libevent.cc
        event_server/event/libevent_scheduler.cc
        event_server/event/real_time_system.cc
        event_server/event/scaled_range_timer_manager_impl.cc
        event_server/event/schedulable_cb_impl.cc
        event_server/event/timer_impl.cc
        event_server/event/signal_impl.cc
)

add_library(event ${event_server_lib})

add_executable(libevent_server main.cpp)

target_link_libraries(libevent_server event)
