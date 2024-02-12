#include <iostream>
#include <memory>
#include "event_server/event/dispatcher_impl.h"
#include "event_server/thread/thread_impl.h"
#include "event_server/event/real_time_system.h"
#include "event_server/event/dispatcher.h"
#include "event_server/event/scaled_range_timer_manager_impl.h"
#include "event_server/network/TcpServer.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::unique_ptr<Envoy::Thread::ThreadFactoryImplPosix> threadFactoryImplPosix =
            std::make_unique<Envoy::Thread::ThreadFactoryImplPosix>();

    Envoy::RealTimeSource this_time = Envoy::RealTimeSource();
    Envoy::Event::RealTimeSystem realTimeSystem = Envoy::Event::RealTimeSystem();

    auto dispatcherImpl = Envoy::Event::DispatcherImpl("tcp_server", *threadFactoryImplPosix,
                                               this_time, realTimeSystem, [](Envoy::Event::Dispatcher& dispatcher) {
        return std::make_unique<Envoy::Event::ScaledRangeTimerManagerImpl>(dispatcher);
    });

    muduo::net::InetAddress listenAddr(2007);
    muduo::net::TcpServer tcpServer = muduo::net::TcpServer(&dispatcherImpl, listenAddr, "tcp-server");

    tcpServer.start();

    return 0;
}
