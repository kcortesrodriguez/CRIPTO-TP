cmake_minimum_required(VERSION 3.10)
project(CRIPTO_TP C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c io.h io.c modular.h modular.c distribution.h distribution.c random.h random.c matrix.h matrix.c bitmap.h recovery.h recovery.c)

add_executable(CRIPTO_TP ${SOURCE_FILES})

add_definitions("-Wall --pedantic -g")