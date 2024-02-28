//
// Created by bont on 24. 2. 28.
//

#include <iostream>
#include <event2/event.h>
#include "event_server/event/Dispatcher.h"

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    auto dispatcher = Dispatcher();
    auto testCallback = dispatcher.createSchedulableCallback(
            []() {
                std::cout << "hello" << std::endl;
            }
    );

    event_base_dump_events(&dispatcher.base(), stdout);

    testCallback->enabled();

/*    dispatcher.post(
            []() {
                std::cout << "hello" << std::endl;
            }

            );*/

    dispatcher.dispatch_loop();

    return 0;
}