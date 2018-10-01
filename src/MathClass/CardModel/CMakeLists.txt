# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project(CardModel)

add_library(${PROJECT_NAME} STATIC
    CardModel.h
    CardModel.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_MAIN_DIR}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    FanucModel
)