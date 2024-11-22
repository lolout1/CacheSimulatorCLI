cmake_minimum_required(VERSION 3.10)
project(cache_simulator VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(cache_simulator
    src/main.cpp
    src/cache_set.cpp
    src/cache.cpp
    src/address_parser.cpp
    src/visualization.cpp
)

# Include directories
target_include_directories(cache_simulator PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
