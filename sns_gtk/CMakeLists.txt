cmake_minimum_required(VERSION 4.0)
project(UI_test C)

set(CMAKE_C_STANDARD 23)

add_executable(UI_test main.c
        sns.c)
