# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project(MathClass)

if(WIN32)
    set(OpenCV_DIR "D:/Common Files/OpenCV/build/x64/vc14/lib")
endif()

find_package(OpenCV REQUIRED)



add_subdirectory("FanucModel")

add_library(${PROJECT_NAME} STATIC
    MathClass.h
    MathClass.cpp
)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_MAIN_DIR}
)



target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    FanucModel
)