cmake_minimum_required(VERSION 3.26)
project(libevent_server)

set(CMAKE_CXX_STANDARD 17)

find_package(Libevent REQUIRED)
find_package(absl REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})


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
        event_server/thread/thread.cc
        event_server/thread/thread_impl.cc
)

add_executable(libevent_server main.cpp ${event_server_lib})


target_link_libraries(libevent_server)