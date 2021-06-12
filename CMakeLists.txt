cmake_minimum_required(VERSION 3.15)
project(MessageFirst C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wjump-misses-init -Wlogical-op")

enable_testing()

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(benchmarks)
add_subdirectory(test)