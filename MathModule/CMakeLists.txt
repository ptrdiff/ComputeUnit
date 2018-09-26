# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project(MathModule)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32)
    set(OpenCV_DIR "D:/Common Files/OpenCV/build/x64/vc14/lib")
endif()

find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME} STATIC
    MathModule.h
    MathModule.cpp
)

add_library(FanucModel STATIC FanucModel/RoboModelGeneral/RoboModelGeneral.cpp FanucModel/FanucModel.cpp)
target_link_libraries(FanucModel ${OpenCV_LIBS})


target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_MAIN_DIR}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Network
    FanucModel
)