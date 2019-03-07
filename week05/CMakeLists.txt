enable_testing()

find_package(Threads REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})

add_executable(talker talker.c)
add_executable(listener listener.c)

target_link_libraries(talker ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(listener ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_executable(my_server my_server.c)
add_executable(my_client my_client.c)

target_link_libraries(my_server ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(my_client ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})


