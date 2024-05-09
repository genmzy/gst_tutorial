cmake_minimum_required(VERSION 3.10)

project(basic-tutorial-1)
set(CMAKE_CXX_STANDARD 11)

include(FindPkgConfig) # equals `pkg-config --cflags --libs gstreamer-1.0`
pkg_check_modules(GST   REQUIRED gstreamer-1.0)
pkg_check_modules(GSTAPP REQUIRED gstreamer-app-1.0)

include_directories(
    ${GST_INCLUDE_DIRS}
    ${GSTAPP_INCLUDE_DIRS}
)

link_directories(
    ${GST_LIBRARY_DIRS}
    ${GSTAPP_LIBRARY_DIRS}
)

add_executable(${PROJECT_NAME}
    basic-tutorial-1.c
)

target_link_libraries(${PROJECT_NAME}
    ${GST_LIBRARIES}
    ${GSTAPP_LIBRARIES}
)
