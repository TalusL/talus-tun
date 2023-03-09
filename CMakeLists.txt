cmake_minimum_required(VERSION 3.6)

project(TalusTun)


set(ENABLE_OPENSSL OFF CACHE BOOL "")

add_subdirectory(3rd/ZLToolKit)
add_subdirectory(3rd/libtuntap)

include_directories(3rd/ZLToolKit/src)
include_directories(3rd/libtuntap/bindings/cpp)
include_directories(src)


file(GLOB_RECURSE SRC_LIST "src/*.cpp" "src/*.c")
add_executable(${CMAKE_PROJECT_NAME} ${SRC_LIST})
set(CMAKE_EXE_LINKER_FLAGS "-static")
target_link_libraries(${CMAKE_PROJECT_NAME} ZLToolKit_static pthread tuntap++)