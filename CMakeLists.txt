cmake_minimum_required(VERSION 3.8)
project(ZKL_NetServer)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ZServer.cpp ZServer.h
        Dao/LibInfo.cpp Dao/LibInfo.h
        Utils/Logger.cpp Utils/Logger.h
        Utils/ZThreadPool.cpp Utils/ZThreadPool.h
        Dao/ThreadJob.cpp Dao/ThreadJob.h)

add_executable(z_server ${SOURCE_FILES})

target_link_libraries(z_server pthread cjson dl)
