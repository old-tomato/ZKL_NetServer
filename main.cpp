#include <iostream>
#include "Utils/Logger.h"
#include "ZServer.h"

using namespace zkl_server;

int main() {
    std::cout << "Hello, World!" << std::endl;
    Logger &logger = Logger::getInstance();
    cout << &logger << endl;

    ZServer * s = new ZServer("/home/zkl/CLionProjects/zkl_server/cmake-build-debug/config.json");

    return 0;
}