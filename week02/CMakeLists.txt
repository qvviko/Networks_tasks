enable_testing()

find_package(Threads REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})

add_executable(week2_ex1 server.c)

target_link_libraries(week2_ex1 ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
