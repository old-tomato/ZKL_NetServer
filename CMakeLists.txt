cmake_minimum_required(VERSION 3.8)
project(z_server)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ZServer.cpp ZServer.h Dao/LibInfo.cpp Dao/LibInfo.h Utils/Logger.cpp Utils/Logger.h)
add_executable(z_server ${SOURCE_FILES})

target_link_libraries(z_server pthread cjson dl)