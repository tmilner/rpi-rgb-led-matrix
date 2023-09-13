cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_TARGET_TRIPLET arm-linux)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

project(ScreenTest VERSION 0.1)

include_directories(lib)
include_directories(include)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/lib/*.c*)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h)

find_package(CURL REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(unofficial-graphicsmagick CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_executable(ScreenTest screen-test-1.cc ${SRC_FILES} ${HEADER_FILES})

target_link_libraries(ScreenTest -Wl,--no-as-needed -latomic -Wl,--as-needed -lcppgpio CURL::libcurl JsonCpp::JsonCpp OpenSSL::SSL OpenSSL::Crypto unofficial::graphicsmagick::graphicsmagick yaml-cpp)
