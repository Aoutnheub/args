cmake_minimum_required(VERSION 3.16)

project(args)

add_library(args STATIC src/args.cpp)
set_target_properties(args PROPERTIES PUBLIC_HEADER "headers/args.h")
INSTALL(TARGETS args PUBLIC_HEADER DESTINATION include)
