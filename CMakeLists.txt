cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ScreenTest VERSION 0.1)

include_directories(lib)
include_directories(include)
include_directories(/usr/include/GraphicsMagick)
include_directories(/usr/lib)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/lib/*.c*)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h)

find_package(CURL REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(OpenSSL REQUIRED)    
find_package(PahoMqttCpp REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(ScreenTest screen-test-1.cc ${SRC_FILES} ${HEADER_FILES})

target_link_libraries(ScreenTest -Wl,--no-as-needed -latomic -Wl,--as-needed -lcppgpio CURL::libcurl )
target_link_libraries(ScreenTest JsonCpp::JsonCpp)
target_link_libraries(ScreenTest OpenSSL::SSL)
target_link_libraries(ScreenTest OpenSSL::Crypto)
target_link_libraries(ScreenTest GraphicsMagick++)
target_link_libraries(ScreenTest GraphicsMagick)
target_link_libraries(ScreenTest yaml-cpp::yaml-cpp)
target_link_libraries(ScreenTest PahoMqttCpp::paho-mqttpp3-static)
