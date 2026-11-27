cmake_minimum_required(VERSION 3.24)
project(OldVersion LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(stackjunkie
  GIT_REPOSITORY https://github.com/stack-junkie/stack-junkie-style.git
  GIT_TAG v3.0.0-preview.7
)
FetchContent_MakeAvailable(stackjunkie)

add_executable(OldVersion WIN32 main.cpp)
