cmake_minimum_required(VERSION 3.17)
project(Boost_Echo_Client1)

set(CMAKE_CXX_STANDARD 11)
link_libraries("-lboost_system")
link_libraries(-pthread)
include_directories(include)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED   ON)
set(Boost_USE_STATIC_RUNTIME   OFF)
find_package(Boost REQUIRED COMPONENTS system)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
target_link_libraries( Boost::system)
add_executable(Boost_Echo_Client1
        include/connectionHandler.h
        src/Client.cpp
        src/connectionHandler.cpp
        )