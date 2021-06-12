add_executable(server-test echo-test/server.c)
add_executable(client-test echo-test/client.c)

target_link_libraries(server-test LINK_PUBLIC MessageFirst)
target_link_libraries(client-test LINK_PUBLIC MessageFirst)

add_test(NAME MessageFirst-tests
        COMMAND python -m unittest discover
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )