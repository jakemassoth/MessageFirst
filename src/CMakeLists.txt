add_library(MessageFirst include/messagefirst_api.h include/messagefirst.h messagefirst_api.c
        include/util.h messagefirst.c)

target_include_directories(MessageFirst PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})