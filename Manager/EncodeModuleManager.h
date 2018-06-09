//
// Created by zkl on 18-6-9.
//

#ifndef ZKL_NETSERVER_ENCODEMODULEMANAGER_H
#define ZKL_NETSERVER_ENCODEMODULEMANAGER_H

#include "../Dao/EncodeModuleInfo.h"
#include "ModuleManager.h"
#include "../Utils/Logger.h"
#include <list>
#include <string>
#include <iostream>
#include <map>
#include <string.h>
#include <sys/socket.h>

namespace zkl_server{
    class EncodeModuleManager : public ModuleManager{

    public:
        EncodeModuleManager(Logger *logger, const list<LibInfo *> &moduleList);

        void withEncode(const string &sendMessage, const void *sendObj, bool successFlag , EncodeModuleInfo & moduleInfo);

        ~EncodeModuleManager() = default;
    };
}

#endif //ZKL_NETSERVER_ENCODEMODULEMANAGER_H
