//
// Created by bont on 24. 2. 28.
//

#include <iostream>
#include <event2/event.h>
#include "event_server/event/DispatcherImp.h"
#include <gtest/gtest.h>

DEFINE_string(gtest_color, "", "");
DEFINE_string(gtest_filter, "", "");


TEST(dispatcher_test, dispatcher_test_schedulableCallback01){
    int rtn = 0;

    auto dispatcher = new Event::DispatcherImp();
    auto testCallback = dispatcher->createSchedulableCallback(
            [&rtn]() {
                std::cout << "hello" << std::endl;
                rtn = 1;
            }
    );

    testCallback->scheduleCallbackNextIteration();
    //testCallback->scheduleCallbackCurrentIteration();
    //testCallback->enabled();

/*    dispatcher.post(
            []() {
                std::cout << "hi~~~~~" << std::endl;
            }
    );*/

    event_base_dump_events(&dispatcher->base(), stdout);
    dispatcher->dispatch_loop(Event::Dispatcher::RunType::Block);
    dispatcher->exit();

    EXPECT_EQ(rtn, 1);
}



int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Event::Libevent::Global::initialize();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}