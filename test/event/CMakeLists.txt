
include_directories(${PROJECT_SOURCE_DIR})
add_executable(test_event main.cpp
)

target_link_libraries(test_event
        event)