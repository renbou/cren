# extended precision integers library for C
# Copyright (C) 2020, Artem Mikheev <c@renbou.ru>.
# Licensed under the Apache License, Version 2.0.

# main library cmakelists

cmake_minimum_required(VERSION 3.12)

project(integers VERSION 1.0.0 DESCRIPTION "Extended precision/size integers library by @renbou for the C language, until compilers add official support" LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -O4")

set(INTEGERS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(INTEGERS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

# This library is defined as an interface since there's pretty much no need for compiling it as a huge separate library,
# but I can add that support if it will be needed by someone
add_library(integers INTERFACE)
add_library(ints ALIAS integers)
target_sources(integers
        INTERFACE
            ${INTEGERS_SOURCE_DIR}/uint128.c)
target_include_directories(integers INTERFACE ${INTEGERS_INCLUDE_DIR})

add_subdirectory(tests)