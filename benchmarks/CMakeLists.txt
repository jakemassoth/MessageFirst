add_executable(echo-benchmark-server echo-benchmark/server.c)
add_executable(echo-benchmark-client echo-benchmark/client.c)

target_link_libraries(echo-benchmark-server LINK_PUBLIC MessageFirst)
target_link_libraries(echo-benchmark-client LINK_PUBLIC MessageFirst)