//
// Created by zkl on 18-6-9.
//

#ifndef ZKL_NETSERVER_MODULEMANAGER_H
#define ZKL_NETSERVER_MODULEMANAGER_H

#include "../Dao/ModuleInfo.h"
#include "../Utils/Logger.h"
#include "../Dao/LibInfo.h"
#include <list>
#include <string>
#include <iostream>
#include <map>
#include <dlfcn.h>

using namespace std;

namespace zkl_server {

#define DL_HANDLER void *

    class ModuleManager {

    protected:

        list<LibInfo *> moduleList;

        /**
         * 这里面都是模块和对应加载的LIB块的关系
         */
        // map<LibInfo *, void *> moduleLibMap;

        Logger *logger = nullptr;

        int moduleCount = 0;

    public:
        ModuleManager(Logger *logger, const list<LibInfo *> &moduleList);

        /**
         * 调用初始化函数,其中任何的一个错误都会导致函数退出
         * @return
         */
        bool doInitFunction();

        DL_HANDLER getLibHander(LibInfo * info);
    };
}


#endif //ZKL_NETSERVER_MODULEMANAGER_H
