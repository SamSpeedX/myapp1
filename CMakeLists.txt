cmake_minimum_required(VERSION 3.16)
project(myapp1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(myspec
    main.cpp
    system/cpu.cpp
    system/ram.cpp
    system/disk.cpp
    system/network.cpp
    system/process.cpp
    system/settings.cpp
    system/history.cpp
    system/alerts.cpp
    system/status.cpp
)

target_include_directories(myspec PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(myspec PRIVATE Qt6::Widgets)
