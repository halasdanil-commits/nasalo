cmake_minimum_required(VERSION 3.10.2)
project("CloudMenu")

add_library(CloudMenu SHARED main.cpp)
target_link_libraries(CloudMenu log android)
