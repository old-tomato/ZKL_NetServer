#include <iostream>
#include "Utils/Logger.h"
#include "ZServer.h"
#include "Utils/ZThreadPool.h"
#include <unistd.h>

using namespace zkl_server;

// 下期的任务目标
// 1. 让每一个种模块类都有一个管理类
// 2. 线程池修改为不需要锁的模型

int main(int argc , char * argv[]) {
    std::cout << "Hello, World! " << std::endl;

    if(argc != 2){
        cout << "argv count error " << endl;
        return -1;
    }

    Logger &logger = Logger::getInstance();

    char * logPath = argv[1];

    ZServer server(logPath);
    server.serverStart();

    return 0;
}