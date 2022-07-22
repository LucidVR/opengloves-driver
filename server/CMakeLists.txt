cmake_minimum_required(VERSION 3.7.1)
set(CMAKE_CXX_STANDARD 20)

project(opengloves_server)


add_library("${PROJECT_NAME}" STATIC include/opengloves_interface.h)

target_sources("${PROJECT_NAME}" PRIVATE src/opengloves_server.cpp)