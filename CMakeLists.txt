cmake_minimum_required(VERSION 3.10)
project(simulator)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(simulator src/main.cpp)
target_link_libraries(simulator Boost::program_options)

