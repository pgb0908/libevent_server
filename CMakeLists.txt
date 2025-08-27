cmake_minimum_required(VERSION 3.26)
project(libevent_server)

find_package(Libevent REQUIRED)
find_package(absl REQUIRED)
find_package(glog REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(libevent_server main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

set(event_server_lib
        event_server/common/Condition.cc
        event_server/common/CountDownLatch.cc
        event_server/common/Date.cc
        event_server/common/Exception.cc
        event_server/common/Timestamp.cc
        event_server/common/TimeZone.cc

        event_server/event/Dispatcher.cpp
        event_server/event/event_impl_base.cc
        event_server/event/file_event_impl.cc
        event_server/event/libevent.cc
        event_server/event/libevent_scheduler.cc
        event_server/event/real_time_system.cc
        event_server/event/schedulable_cb_impl.cc
        event_server/event/timer_impl.cc


        event_server/network/Acceptor.cc
        event_server/network/Buffer.cc
        event_server/network/Connector.cc
        event_server/network/InetAddress.cc
        event_server/network/Socket.cc
        event_server/network/SocketsOps.cc
        event_server/network/TcpConnection.cc
        event_server/network/TcpServer.cc

        event_server/thread/CurrentThread.cc
        event_server/thread/EventLoopThread.cc
        event_server/thread/EventLoopThreadPool.cc
        event_server/thread/ThreadPool.cc
        event_server/thread/Thread.cc
        event_server/thread/WorkerImpl.cpp

)


add_library(event ${event_server_lib})
target_link_libraries(event
        libevent::pthreads
        libevent::core
        glog::glog
        Threads::Threads
)

target_link_libraries(libevent_server
        event)


add_subdirectory(test)