cmake_minimum_required(VERSION 3.18.1)

project("antifrida")

set(can_use_assembler TRUE)
enable_language(ASM)

add_library(antifrida SHARED syscall.S antifrida.cpp)

find_library(log-lib log)

target_link_libraries(antifrida ${log-lib})
