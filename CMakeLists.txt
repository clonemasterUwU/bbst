cmake_minimum_required(VERSION 3.22)
project(BBST)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fsanitize=undefined -O0")

#add_library(BBST SHARED tree_utils.h rb_tree.h)

add_executable(testing main.cpp rb_tree_custom_invoke.h)
