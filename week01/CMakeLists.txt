enable_testing()

find_package(Threads REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})

add_executable(week1_ex1 stack_client_server.c)

target_link_libraries(week1_ex1 ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
