cmake_minimum_required(VERSION 3.10)

# set the project name
project(lab1)

# add the executable
add_executable(lab1 lab1.cc spm.cc)
add_subdirectory(tests)
