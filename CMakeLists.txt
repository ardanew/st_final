cmake_minimum_required(VERSION 2.8)
project(final)

set(SRCLIST
	final.cpp
	server.cpp
)

if(UNIX)
	set(EXTRA_COMPILER_OPTIONS "-std=c++11 -lpthread") 
endif(UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILER_OPTIONS}")

add_executable(final ${SRCLIST})

if(UNIX)
#link_libraries(final pthread)
endif(UNIX)
