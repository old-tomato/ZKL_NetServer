//
// Created by zkl on 18-6-9.
//

#ifndef ZKL_NETSERVER_DECODEMANAGER_H
#define ZKL_NETSERVER_DECODEMANAGER_H

#include "../Dao/DecodeModuleInfo.h"
#include "ModuleManager.h"
#include "../Utils/Logger.h"
#include <list>
#include <string>
#include <iostream>
#include <map>
#include <string.h>
#include <sys/socket.h>

using namespace std;

namespace zkl_server{

    class DecodeModuleManager : public ModuleManager{
    public:
        DecodeModuleManager(Logger *logger, const list<LibInfo *> &moduleList);

        /**
         * 如果需要进行真正的处理,需要调用这个函数进行处理,如果需要调用初始化函数,需要显示手动调用父类的方法
         * @param fd
         * @param moduleInfo
         */
        void withDecode(int fd , DecodeModuleInfo & moduleInfo);

        ~DecodeModuleManager();
    };
}

#endif //ZKL_NETSERVER_DECODEMANAGER_H
