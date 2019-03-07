enable_testing()

find_package(Threads REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})

add_executable(server server.c)
add_executable(client client.c)

target_link_libraries(server ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
