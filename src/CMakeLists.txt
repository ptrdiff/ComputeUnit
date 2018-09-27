# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("ComputeUnit")

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/5.11.0/msvc2017_64/")
else(UNIX)
    set(CMAKE_PREFIX_PATH "/usr/include/x86_64-linux-gnu/qt5")
endif()

# Set compiler options.
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(PROJECT_MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
find_package(Qt5 COMPONENTS Core Network Test REQUIRED)

# Include sub-projects.
add_subdirectory ("ComputerVisionSystem")
add_subdirectory ("RobotConnector")
add_subdirectory ("RCAConnector")
add_subdirectory ("SensorAdapter")
add_subdirectory ("MathClass")
add_subdirectory ("Executor")
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Network
        RobotConnector
        RCAConnector
        SensorAdapter
        Executor
        MathClass
)

add_test(Name tst COMMAND ExecutorTesting)