cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(pcl_viewer)
find_package(PCL 1.8 REQUIRED)
find_package(k4a REQUIRED)
find_package(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(${K4A_INCLUDE_DIRS})
include_directories(${PCL_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIR})

link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable(${PROJECT_NAME}
    "main.cpp"
    "k4a_grabber.cpp"
    )

target_link_libraries(${PROJECT_NAME}
    ${PCL_LIBRARIES}
    k4a::k4a
    ${OpenCV_LIBS}
    )
