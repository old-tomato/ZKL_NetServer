cmake_minimum_required(VERSION 3.8)
project(ZKL_NetServer)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ZServer.cpp ZServer.h
        Dao/LibInfo.cpp Dao/LibInfo.h
        Utils/Logger.cpp Utils/Logger.h
        Utils/ZThreadPool.cpp Utils/ZThreadPool.h
        Dao/ThreadJob.cpp Dao/ThreadJob.h
        Dao/DecodeModuleInfo.h
        Dao/PercolatorModuleInfo.h
        Dao/ModuleInfo.h
        Dao/EncodeModuleInfo.h
        Dao/ServiceModuleInfo.h
        Manager/ConfigManager.cpp Manager/ConfigManager.h
        Manager/DecodeModuleManager.cpp Manager/DecodeModuleManager.h Manager/ModuleManager.cpp Manager/ModuleManager.h Manager/PercolatorModuleManager.cpp Manager/PercolatorModuleManager.h Manager/ServiceModuleManager.cpp Manager/ServiceModuleManager.h Manager/EncodeModuleManager.cpp Manager/EncodeModuleManager.h)

add_executable(z_server ${SOURCE_FILES})

target_link_libraries(z_server pthread cjson dl)
