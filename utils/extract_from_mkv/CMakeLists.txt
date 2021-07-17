cmake_minimum_required(VERSION 3.9)
project(extract_from_mkv)

add_executable(extract_from_mkv main.cpp ../../depthundistorthelper.cpp)
find_package(k4a)
find_package(k4arecord)
find_package(OpenCV)
target_link_libraries(extract_from_mkv 
    k4a::k4a
    k4a::k4arecord
    ${OpenCV_LIBRARIES}
    libturbojpeg.so
    )