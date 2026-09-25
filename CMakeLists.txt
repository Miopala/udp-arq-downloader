cmake_minimum_required(VERSION 3.16)
project(Transport LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_ASAN "Enable AddressSanitizer" OFF)

add_executable(transport
    src/main.cpp
    src/socket.cpp
    src/downloader.cpp
    src/ip_validator.cpp
)

target_compile_options(transport PRIVATE
    -O3 -g -fno-omit-frame-pointer
    -Wall -Wextra -Wpedantic 
    -Werror -Wshadow 
    -Wnon-virtual-dtor 
    -Wold-style-cast
)

if(USE_ASAN)
    target_compile_options(transport PRIVATE -fsanitize=address)
    target_link_options(transport PRIVATE -fsanitize=address)
endif()

target_include_directories(transport PRIVATE include)