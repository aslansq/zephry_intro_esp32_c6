# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(demo)

include(demo.cmake)
target_sources(app PRIVATE "bsp.c")
target_include_directories(app PUBLIC "${CMAKE_SOURCE_DIR}")
target_include_directories(app PUBLIC "${CMAKE_SOURCE_DIR}/../modules/hal/espressif/components/soc/esp32c6/include/soc")