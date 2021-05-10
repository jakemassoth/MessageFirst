add_library(MessageFirst include/messagefirst_api.h include/messagefirst.h messagefirst_api_poll.c
        include/util.h messagefirst.c include/thread_pool.h thread_pool.c messagefirst_api_send.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(MessageFirst Threads::Threads)
target_include_directories(MessageFirst PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})