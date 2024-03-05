#include <iostream>
#include "event_server/network/TcpServer.h"
#include <glog/logging.h>



int main(int argc, char* argv[]) {

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Event::Libevent::Global::initialize();

    LOG(INFO) << "TCP server start";
    auto dispatcherImpl = Event::DispatcherImp();
    muduo::net::InetAddress listenAddr("192.168.15.130",9990);
    muduo::net::TcpServer tcpServer = muduo::net::TcpServer(&dispatcherImpl, listenAddr, "tcp-server");
    tcpServer.setThreadNum(2);
    tcpServer.start();


    return 0;
}
