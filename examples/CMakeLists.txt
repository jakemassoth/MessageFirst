add_executable(server echo-server/server.c)
add_executable(echo-client echo-server/client.c)

target_link_libraries(server LINK_PUBLIC MessageFirst)
target_link_libraries(echo-client LINK_PUBLIC MessageFirst)