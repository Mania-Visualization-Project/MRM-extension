cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME MRM-extension)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -static")

file(GLOB SOURCE_FILES
        "main.cpp"
        "FileManager.cpp"
        "OsuManager.cpp"
        "ConfigManager.cpp"
        )
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

find_package(Threads)
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})