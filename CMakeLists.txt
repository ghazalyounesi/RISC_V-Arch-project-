cmake_minimum_required(VERSION 3.31)
project(Arch)

set(CMAKE_CXX_STANDARD 20)

add_executable(Arch main.cpp
        common.h
        symbol_table.cpp
        symbol_table.h
        encoder.cpp
        encoder.h
        parser.cpp
        parser.h
        pseudo.cpp
        pseudo.h
        comm.h
        register_file.cpp
        register_file.h
        memory.cpp
        memory.h
        instruction.cpp
        instruction.h
        cpu.cpp
        cpu.h
)

