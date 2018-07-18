# General
cmake_minimum_required(VERSION 3.10)
set(MY_PROJECT_NAME RapidImageProcessor)
project(${MY_PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)
add_definitions("-Wall")

# Packages
find_package(X11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${X11_MODULE_PATH})
set(MY_INCLUDE_DIRS ${MY_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
set(MY_LINK_LIBRARIES ${MY_LINK_LIBRARIES} ${X11_LIBRARIES})

# Files
set(MY_SOURCE_FILES ${MY_SOURCE_FILES} src/main.cpp)

# Directories
set(MY_INCLUDE_DIRS ${MY_INCLUDE_DIRS} src)

# Target
add_executable(${MY_PROJECT_NAME} ${MY_SOURCE_FILES})
target_include_directories(${MY_PROJECT_NAME} PRIVATE ${MY_INCLUDE_DIRS})
target_link_libraries(${MY_PROJECT_NAME} ${MY_LINK_LIBRARIES})
