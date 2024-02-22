#include <iostream>
#include <memory>
#include <thread>
#include "event_server/network/TcpServer.h"




int main() {



    auto dispatcherImpl = Dispatcher();

    muduo::net::InetAddress listenAddr("192.168.15.130",9990);
    muduo::net::TcpServer tcpServer = muduo::net::TcpServer(&dispatcherImpl, listenAddr, "tcp-server");

    tcpServer.start();


    return 0;
}
