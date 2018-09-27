# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project(RoboModelGeneral)

if(WIN32)
    set(OpenCV_DIR "D:/Common Files/OpenCV/build/x64/vc14/lib")
endif()

find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME} STATIC
    RoboModelGeneral.h
    RoboModelGeneral.cpp
)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})


target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_MAIN_DIR}
)
