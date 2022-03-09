cmake_minimum_required(VERSION 3.10)

# set the project name
project(processor)

# add the executable
add_executable(processor error.cc main.cc memory.cc opcode.cc proc.cc register.cc robbin.cc util.cc)
add_subdirectory(tests)
