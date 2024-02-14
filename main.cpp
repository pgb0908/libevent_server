#include <iostream>
#include <memory>
#include <thread>
#include "event_server/event/dispatcher_impl.h"
#include "event_server/thread/thread_impl.h"
#include "event_server/event/real_time_system.h"
#include "event_server/event/dispatcher.h"
#include "event_server/event/scaled_range_timer_manager_impl.h"
#include "event_server/network/TcpServer.h"

int main() {

    Envoy::Event::Libevent::Global::initialize();
    auto threadFactoryImplPosix =
            std::make_unique<Envoy::Thread::ThreadFactoryImplPosix>();

    auto this_time = std::make_unique<Envoy::RealTimeSource>();
    auto realTimeSystem = std::make_unique<Envoy::Event::RealTimeSystem>();

    auto dispatcherImpl = std::make_unique<Envoy::Event::DispatcherImpl>("tcp_server", *threadFactoryImplPosix,
                                                       *this_time, *realTimeSystem,
                                                       [](Envoy::Event::Dispatcher &dispatcher) {
                                                           return std::make_unique<Envoy::Event::ScaledRangeTimerManagerImpl>(
                                                                   dispatcher);
                                                       });

    muduo::net::InetAddress listenAddr("192.168.15.127",9990);

    muduo::net::TcpServer tcpServer = muduo::net::TcpServer(dispatcherImpl.get(), listenAddr, "tcp-server");
    tcpServer.start();


    dispatcherImpl->shutdown();

    return 0;
}
