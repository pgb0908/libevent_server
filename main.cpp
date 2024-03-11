#include <iostream>
#include "event_server/network/TcpServer.h"
#include <glog/logging.h>
#include <iomanip>

using namespace std;

void MyPrefixFormatter(std::ostream& s, const google::LogMessage& m, void* /*data*/) {
    s << google::GetLogSeverityName(m.severity())[0]
      << setw(4) << 1900 + m.time().year()
      << setw(2) << 1 + m.time().month()
      << setw(2) << m.time().day()
      << ' '
      << setw(2) << m.time().hour() << ':'
      << setw(2) << m.time().min()  << ':'
      << setw(2) << m.time().sec() << "."
      << setw(4) << m.time().usec()
      //<< ' '
      << setfill(' ') << setw(2)
      << " THREAD["
      << m.thread_id() << setfill('0') << "]"
      << ' '
      << m.basename() << ':' << m.line() << " | ";
}


int main(int argc, char* argv[]) {

/*    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);*/
    // 기본 로그 설정 변경
    FLAGS_logtostderr = true; // stderr에 로그 출력
    google::SetLogDestination(google::INFO, ""); // 모든 로그 레벨의 로그를 stderr에 출력
    google::SetLogDestination(google::WARNING, ""); // 모든 로그 레벨의 로그를 stderr에 출력
    google::SetLogDestination(google::ERROR, ""); // 모든 로그 레벨의 로그를 stderr에 출력
    google::SetLogDestination(google::FATAL, ""); // 모든 로그 레벨의 로그를 stderr에 출력

    // 사용자 정의 로그 메시지 핸들러 설정
    google::InstallPrefixFormatter(MyPrefixFormatter);

    // glog 초기화
    google::InitGoogleLogging(argv[0]);


    Event::Libevent::Global::initialize();

    LOG(INFO) << "TCP server start";
    auto dispatcherImpl = Event::DispatcherImp();
    muduo::net::InetAddress listenAddr("192.168.15.130",9990);
    muduo::net::TcpServer tcpServer = muduo::net::TcpServer(&dispatcherImpl, listenAddr, "tcp-server", 1);
    //tcpServer.setThreadNum(2);
    tcpServer.start();


    return 0;
}
