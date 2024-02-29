#include <iostream>
#include "event_server/network/TcpServer.h"
#include <glog/logging.h>



int main(int argc, char* argv[]) {

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    LOG(INFO) << "TCP server start";
    auto dispatcherImpl = DispatcherImp();
    muduo::net::InetAddress listenAddr("192.168.15.130",9990);
    muduo::net::TcpServer tcpServer = muduo::net::TcpServer(&dispatcherImpl, listenAddr, "tcp-server");
    tcpServer.setThreadNum(1);
    tcpServer.start();


    return 0;
}
