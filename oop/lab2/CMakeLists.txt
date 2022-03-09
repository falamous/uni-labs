cmake_minimum_required(VERSION 3.10)

# set the project name
project(lab2)

# add the executable
add_executable(lab2 deltoid.cc lab2.cc)
add_subdirectory(tests)
