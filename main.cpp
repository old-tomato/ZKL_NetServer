#include <iostream>
#include "Utils/Logger.h"
#include "ZServer.h"
#include "Utils/ZThreadPool.h"
#include <unistd.h>

using namespace zkl_server;

int func(ThreadJob * job){
    cout << "x " << (int)(intptr_t )job->getJob() << endl;
    for(int x = 0 ; x < (int)(intptr_t )job->getJob() ; ++ x){
        sleep((1));
        cout << job->getThreadId() << "     " << (int)(intptr_t )job->getJob() << endl;
    }
    return 0;
}

int main() {
    std::cout << "Hello, World!" << std::endl;
    Logger &logger = Logger::getInstance();
    cout << &logger << endl;

    // TODO 这里的位置应该是从命令行参数中传入的
    ZServer server("/home/zkl/CLionProjects/ZKL_NetServer/cmake-build-debug/config.json");
    server.serverStart();

//    ZThreadPool &pool = ZThreadPool::getInstance(4 , &logger);
//
//
//    pool.startPool();
//
//    for(int x = 0 ; x < 10 ; ++ x){
//        ThreadJob * threadJob = new ThreadJob((void *)(intptr_t)x , 10 , func);
//        cout << x << endl;
//        pool.setJob(threadJob);
//    }
//
//    sleep(10);
//    cout << "end==================" << endl;
//    // 如果这里沉睡XX秒
//    pool.stopPool();

    return 0;
}