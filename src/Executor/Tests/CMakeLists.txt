cmake_minimum_required(VERSION 3.8)

project(ExecutorTesting)

enable_testing()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${PROJECT_NAME} ExecutorTest.cpp)
add_test(Name ExecutorTest COMMAND ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} 
    Qt5::Test
)