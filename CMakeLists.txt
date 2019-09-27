cmake_minimum_required(VERSION 3.14)
project(9cc C)

set(CMAKE_C_STANDARD 11)

add_executable(9cc
        main.c
        tokenize.c
        parse.c
        types.c
        codegen.c)
