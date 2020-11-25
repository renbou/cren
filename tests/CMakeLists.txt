# extended precision integers library for C
# Copyright (C) 2020, Artem Mikheev <c@renbou.ru>.
# Licensed under the Apache License, Version 2.0.

# tests cmakelists

add_executable(uint128_test uint128_test.c)
target_link_libraries(uint128_test integers)