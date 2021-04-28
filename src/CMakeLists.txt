add_library(MessageFirst include/messagefirst_api.h include/messagefirst.h messagefirst_api.c
        include/util.h messagefirst.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(MessageFirst Threads::Threads)
target_include_directories(MessageFirst PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})