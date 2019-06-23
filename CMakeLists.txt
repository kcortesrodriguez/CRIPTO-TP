cmake_minimum_required(VERSION 3.10)
project(CRIPTO_TP C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS, "-pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -D_POSIX_C_SOURCE=200112L")

set(SOURCE_FILES src/main.c src/io.h src/io.c src/modular.h src/modular.c src/distribution.h src/distribution.c src/random.h src/random.c src/matrix.h src/matrix.c src/bitmap.h src/recovery.h src/recovery.c)

add_executable(CRIPTO_TP ${SOURCE_FILES})

target_link_libraries(CRIPTO_TP m)
add_definitions("-Wall --pedantic -g")