cmake_minimum_required(VERSION 3.9)
project(playback_external_sync)

add_executable(playback_external_sync main.cpp)
find_package(k4a)
find_package(k4arecord)
find_package(OpenCV)
target_link_libraries(playback_external_sync 
    k4a::k4a
    k4a::k4arecord
    ${OpenCV_LIBRARIES}
    libturbojpeg.so
    )