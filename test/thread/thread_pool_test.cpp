
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
    std::string name = "thread_pool_test";
    auto eventLoopThreadPool = new muduo::net::EventLoopThreadPool(3, name);
    eventLoopThreadPool->start();

    auto loops = eventLoopThreadPool->getLoops();
    std::cout << loops.size() << std::endl;

    for(auto loop : eventLoopThreadPool->getLoops()){
       std::cout << loop->getThreadId() <<  " ";
    }std::cout << std::endl;

    auto next_loop = eventLoopThreadPool->getNextLoop();
    auto next_loop2 = eventLoopThreadPool->getNextLoop();
    auto next_loop3 = eventLoopThreadPool->getNextLoop();
    auto next_loop4 = eventLoopThreadPool->getNextLoop();
    auto next_loop5 = eventLoopThreadPool->getNextLoop();
    auto next_loop6 = eventLoopThreadPool->getNextLoop();



    //eventLoopThreadPool->wait();
}




int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Event::Libevent::Global::initialize();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}