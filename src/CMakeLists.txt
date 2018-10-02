# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(PROJECT_MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Include sub-projects.
add_subdirectory ("ComputerVisionSystem")
add_subdirectory ("RobotConnector")
add_subdirectory ("RCAConnector")
add_subdirectory ("SensorAdapter")
add_subdirectory ("MathClass")
add_subdirectory ("Executor")

add_test(Name tst COMMAND ExecutorTesting)