cmake_minimum_required(VERSION 3.19)
project(MessageFirst C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wjump-misses-init -Wlogical-op")

add_executable(server src/include/messagefirst_api.h src/include/messagefirst.h src/messagefirst_api.c
        examples/echo-server/server.c src/include/util.h src/messagefirst.c)

add_executable(client src/include/messagefirst_api.h src/include/messagefirst.h src/messagefirst_api.c
        examples/echo-server/client.c src/include/util.h src/messagefirst.c)