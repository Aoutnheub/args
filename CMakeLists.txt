cmake_minimum_required(VERSION 3.16)

project(args)

add_library(args STATIC src/args.cpp)
set(HEADERS "headers/args.h" "headers/Hargs.h")
set_target_properties(args PROPERTIES PUBLIC_HEADER "${HEADERS}")
INSTALL(TARGETS args PUBLIC_HEADER DESTINATION include)
