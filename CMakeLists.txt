# Extended C library by renbou
# Copyright (C) 2020, Artem Mikheev <c@renbou.ru>.
# Licensed under the Apache License, Version 2.0.

# main library cmakelists

cmake_minimum_required(VERSION 3.12)

project(integers VERSION 0.1.0 DESCRIPTION "Extended precision/size integers library by @renbou for the C language, until compilers add official support" LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Ofast")

set(CREN_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CREN_TESTS_DIR ${PROJECT_SOURCE_DIR}/tests)
set(CREN_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

# The libraries along with their specific tests are defined in the specific CMakeLists in this directory
# they are all originally defined as interface libraries, so that they can simply be used for building separate projects,
# but it isn't hard to add a global library which includes all of the smaller libraries and compiles it into a proper library object
# If that is needed you can request it and I will do it, but currently I don't need it
add_subdirectory(cmake)