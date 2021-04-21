cmake_minimum_required(VERSION 3.19)
project(MessageFirst C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wjump-misses-init -Wlogical-op")

add_executable(server include/messagefirst_api.h include/messagefirst.h messagefirst_api.c server.c include/util.h messagefirst.c)

add_executable(client include/messagefirst_api.h include/messagefirst.h messagefirst_api.c client.c include/util.h messagefirst.c)