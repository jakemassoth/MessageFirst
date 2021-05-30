add_executable(echo-benchmark-multi-thread-server echo-benchmark-multi-thread/server.c)
add_executable(echo-benchmark-multi-thread-client echo-benchmark-multi-thread/client.c)

add_executable(echo-benchmark-server echo-benchmark/server.c)
add_executable(echo-benchmark-client echo-benchmark/client.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(echo-benchmark-server LINK_PUBLIC MessageFirst)
target_link_libraries(echo-benchmark-client LINK_PUBLIC MessageFirst)

target_link_libraries(echo-benchmark-multi-thread-client LINK_PUBLIC MessageFirst Threads::Threads)
target_link_libraries(echo-benchmark-multi-thread-server LINK_PUBLIC MessageFirst)
