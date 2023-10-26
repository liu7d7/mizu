cmake_minimum_required(VERSION 3.26)
project(mizu C)

set(CMAKE_C_STANDARD 23)

add_executable(mizu main.c
        lex.h
        util.h
        util.c
        lex.c
        parse.h
        parse.c
        to_mizu.h
        to_mizu.c
        to_c.h
        dyn_arr.h
        dyn_arr.c)
