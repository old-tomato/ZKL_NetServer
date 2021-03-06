#include <iostream>
#include "Utils/Logger.h"
#include "ZServer.h"
#include "Utils/ZThreadPool.h"
#include <unistd.h>

using namespace zkl_server;

// 下期的任务目标
// 1. 线程池修改为不需要锁的模型
// 2. 日志类的写入感觉过于频繁了

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