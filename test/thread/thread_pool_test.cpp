
#include <iostream>
#include <event2/event.h>
#include "event_server/event/DispatcherImp.h"
#include "event_server/thread/EventLoopThreadPool.h"
#include "event_server/thread/WorkerImpl.h"
#include <gtest/gtest.h>

DEFINE_string(gtest_color, "", "");
DEFINE_string(gtest_filter, "", "");


/*TEST(worker, test01){
    std::vector<std::unique_ptr<WorkerImpl>> workers;

    for(int i=0; i < 2; i++){
        std::string name  = "worker";
        auto worker = std::make_unique<WorkerImpl>([](){
            std::cout << "hello worker" << std::endl;
        }, name+std::to_string(i));
        workers.push_back(std::move(worker));
    }

    for(auto& worker : workers){
        worker->start();
    }

}*/


TEST(thread_pool, test01){
    auto dispatcher = Event::DispatcherImp();
    std::string name = "loop";
    auto eventLoopThreadPool = muduo::net::EventLoopThreadPool(&dispatcher, name);
    eventLoopThreadPool.setThreadNum(1);

    eventLoopThreadPool.start([this](Event::DispatcherImp* dispatcherImp){
        std::cout << "hello" << std::endl;
        std::cout << dispatcherImp->getThreadId() << std::endl;
    });

    for(auto loop : eventLoopThreadPool.getAllLoops()){
       std::cout << loop->getThreadId() <<  " ";
    }std::cout << std::endl;

    //std::this_thread::sleep_for(std::chrono::seconds(2));

}




int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Event::Libevent::Global::initialize();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}