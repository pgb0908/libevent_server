
#include <iostream>
#include <event2/event.h>
#include "event_server/event/DispatcherImp.h"
#include "event_server/thread/EventLoopThreadPool.h"
#include <gtest/gtest.h>

DEFINE_string(gtest_color, "", "");
DEFINE_string(gtest_filter, "", "");

TEST(thread_pool, test01){
    auto dispatcher = Event::DispatcherImp();
    std::string name = "loop";
    auto eventLoopThreadPool = muduo::net::EventLoopThreadPool(&dispatcher, name);
    eventLoopThreadPool.setThreadNum(5);

    eventLoopThreadPool.start([this](Event::DispatcherImp* dispatcherImp){
        std::cout << "hello" << std::endl;
        std::cout << dispatcherImp->getThreadId() << std::endl;
    });

/*    for(auto loop : eventLoopThreadPool.getAllLoops()){
        std::cout << loop->getThreadId() <<  " ";
    }std::cout << std::endl;*/


}


int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Event::Libevent::Global::initialize();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}