add_executable(thread_pool_test thread_pool_test.cpp)

target_link_libraries(thread_pool_test
        event
        GTest::gtest_main
        GTest::gtest)