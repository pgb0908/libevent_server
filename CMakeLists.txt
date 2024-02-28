cmake_minimum_required(VERSION 3.26)
project(libevent_server)

set(CMAKE_CXX_STANDARD 17)

find_package(Libevent REQUIRED)
find_package(absl REQUIRED)
find_package(glog REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(libevent_server main.cpp)

set(event_server_lib
        event_server/common/Condition.cc
        event_server/common/CountDownLatch.cc
        event_server/common/Date.cc
        event_server/common/Exception.cc
        event_server/common/Timestamp.cc
        event_server/common/TimeZone.cc

        event_server/event/Dispatcher.cpp
        event_server/event/FileEvent.cpp
        event_server/event/ImplBase.cpp
        event_server/event/Libevent.cpp
        event_server/event/LibEventSchedular.cpp
        event_server/event/SchedularCallback.cpp


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

)

add_library(event ${event_server_lib})
target_link_libraries(event
        libevent::core
        glog::glog)

target_link_libraries(libevent_server
        event)


add_subdirectory(test/event)