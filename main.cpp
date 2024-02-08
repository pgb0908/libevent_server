#include <iostream>
#include "event_server/event/dispatcher_impl.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    auto aa = new Envoy::Event::DispatcherImpl(<#initializer#>, <#initializer#>);
    aa->post([](){ std::cout << "hello" << std::endl; return;});
    return 0;
}
