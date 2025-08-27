//
// Created by bont on 24. 2. 28.
//

#include <iostream>
#include <event2/event.h>
#include "event_server/event/Dispatcher.h"
#include <gtest/gtest.h>
#include "event_server/event/deferred_task.h"
#include "event_server/event/deferred_deletable.h"

DEFINE_string(gtest_color, "", "");
DEFINE_string(gtest_filter, "", "");


TEST(dispatcher_test, scheduleCallbackCurrentIteration){
    int rtn = 0;

    auto dispatcher = new Event::Dispatcher();
    auto testCallback = dispatcher->createSchedulableCallback(
            [&rtn]() {
                std::cout << "hello" << std::endl;
                rtn = 1;
            }
    );

    testCallback->scheduleCallbackCurrentIteration();

    event_base_dump_events(&dispatcher->base(), stdout);
    dispatcher->run(Event::RunType::Block);
    dispatcher->exit();

    EXPECT_EQ(rtn, 1);
}


TEST(dispatcher_test, currentIterOrNextIter){
    int rtn = 0;

    auto dispatcher = new Event::Dispatcher();

    auto callbackCurrent = dispatcher->createSchedulableCallback(
            [&rtn]() {
                std::cout << "current" << std::endl;
                rtn = 1;
            }
    );
    auto callbackCurrent2 = dispatcher->createSchedulableCallback(
            [&rtn]() {
                std::cout << "current2" << std::endl;
                rtn = 1;
            }
    );

    auto callbackNext = dispatcher->createSchedulableCallback(
            [&rtn]() {
                std::cout << "next" << std::endl;
                rtn = 1;
            }
    );


    callbackNext->scheduleCallbackNextIteration();
    callbackCurrent->scheduleCallbackCurrentIteration();
    callbackCurrent2->scheduleCallbackCurrentIteration();


    event_base_dump_events(&dispatcher->base(), stdout);
    dispatcher->run(Event::RunType::Block);
    dispatcher->exit();

}

TEST(dispatcher_test, post){
    int rtn = 0;

    auto dispatcher = new Event::Dispatcher();

    dispatcher->post([]{
        std::cout << "hello" << std::endl;

    });

    event_base_dump_events(&dispatcher->base(), stdout);
    dispatcher->run(Event::RunType::Block);
    dispatcher->exit();

}

class TestDeferredDeletable : public Event::DeferredDeletable {
public:
    TestDeferredDeletable(std::function<void()> on_destroy) : on_destroy_(on_destroy) {}
    ~TestDeferredDeletable() override { on_destroy_(); }

private:
    std::function<void()> on_destroy_;
};



TEST(dispatcher_test, deferredDelete){
    int rtn = 0;

    auto dispatcher = new Event::Dispatcher();

    dispatcher->deferredDelete(
            Event::DeferredDeletablePtr{
                new TestDeferredDeletable([&]() -> void { std::cout << "deferredDelete"; })
            });



    event_base_dump_events(&dispatcher->base(), stdout);
    dispatcher->run(Event::RunType::Block);
    dispatcher->exit();

}


int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Event::Libevent::Global::initialize();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}