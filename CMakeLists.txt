cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(k4a_capture)
SET(CMAKE_BUILD_TYPE "Release")
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


# Find installed Open3D, this imports
#   Open3D_INCLUDE_DIRS
#   Open3D_LIBRARIES
#   Open3D_LIBRARY_DIRS
#   Open3D_C_FLAGS
#   Open3D_CXX_FLAGS
#   Open3D_EXE_LINKER_FLAGS
if(WIN32)
    find_package(Open3D HINTS ${CMAKE_INSTALL_PREFIX}/CMake)
else()
    find_package(Open3D HINTS ${CMAKE_INSTALL_PREFIX}/lib/CMake)
    list(APPEND Open3D_LIBRARIES dl)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${Open3D_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Open3D_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${Open3D_EXE_LINKER_FLAGS}")

# Set OS-specific things here
if(WIN32)
    if (ENABLE_GUI)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE")
        # Filament is built with /MT[d], but by default CMake automatically
        # sets "/MD" and "/MDd". They can't coexist, so we need to replace them.
        string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
        string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
        string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
        string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    endif()
elseif(CYGWIN)
elseif(APPLE)
elseif(UNIX)
        add_definitions(-DUNIX)
        add_compile_options(-Wno-deprecated-declarations)
        add_compile_options(-Wno-unused-result)
    add_definitions(-O3)
endif(WIN32)

# Open3D
if (Open3D_FOUND)
    message(STATUS "Found Open3D ${Open3D_VERSION}")

    # link_directories must be before add_executable
    link_directories(${Open3D_LIBRARY_DIRS})

#    add_executable(k4a_capture TestVisualizer.cpp)

#    target_link_libraries(k4a_capture ${Open3D_LIBRARIES})

#    target_include_directories(k4a_capture PUBLIC ${Open3D_INCLUDE_DIRS})

    # Hot fix windows dll not found issue, assumming we're using the Release build
    option(BUILD_SHARED_LIBS "Whether Open3D was build as shared library" OFF)
    if(WIN32 AND BUILD_SHARED_LIBS)
        message("Will copy Open3D.dll to ${CMAKE_CURRENT_BINARY_DIR}/Release")
        add_custom_command(TARGET TestVisualizer POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                                ${CMAKE_INSTALL_PREFIX}/bin/Open3D.dll
                                ${CMAKE_CURRENT_BINARY_DIR}/Release)
    endif()
    if(WIN32 AND ENABLE_GUI)
        message("Will copy SDL2.dll to ${CMAKE_CURRENT_BINARY_DIR}/Release")
        add_custom_command(TARGET TestVisualizer POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                                ${CMAKE_INSTALL_PREFIX}/lib/SDL2.dll
                                ${CMAKE_CURRENT_BINARY_DIR}/Release)
    endif()

else ()
    message(SEND_ERROR "Open3D not found")
endif ()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(k4a)
find_package(OpenCV)
list(APPEND CMAKE_PREFIX_PATH "/home/cly/libtorch/share/cmake/Torch/")
find_package(Torch)

include_directories(${K4A_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${TORCH_INCLUDE_DIRS})

add_executable(k4a_capture
    mainwindow.ui
    mainwindow.cpp
    k4adevice.cpp
    devmanager.cpp
    depthcolorizer.cpp
    bgmatting.cpp
    main.cpp
)

target_link_libraries(k4a_capture
    Qt5::Widgets
    k4a::k4a
    ${Open3D_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${TORCH_LIBRARIES}
)
target_include_directories(k4a_capture PUBLIC ${Open3D_INCLUDE_DIRS})
