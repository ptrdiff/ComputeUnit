# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project(Executor)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(${PROJECT_NAME} STATIC
    Executor.h
    Executor.cpp
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
    FanucAdapter
    RCAConnector
    SensorAdapter
    MultiThreadingWorker
)

add_subdirectory ("Tests")