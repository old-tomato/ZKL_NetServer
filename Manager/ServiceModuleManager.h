//
// Created by zkl on 18-6-9.
//

#ifndef ZKL_NETSERVER_SERVICEMODULEMNAGER_H
#define ZKL_NETSERVER_SERVICEMODULEMNAGER_H

#include "../Dao/PercolatorModuleInfo.h"
#include "../Dao/DecodeModuleInfo.h"
#include "../Dao/ServiceModuleInfo.h"
#include "ModuleManager.h"
#include "../Utils/Logger.h"
#include <list>
#include <string>
#include <iostream>
#include <map>
#include <string.h>
#include <sys/socket.h>

using namespace std;

namespace zkl_server {

    class ServiceModuleManager : public ModuleManager  {
    public:
        ServiceModuleManager(Logger *logger, const list<LibInfo *> &moduleList);

        void withServer(DecodeModuleInfo & decodeModule , PercolatorModuleInfo & percolatorModule, ServiceModuleInfo & serviceModuleInfo);

        ~ServiceModuleManager() = default;

    };
}


#endif //ZKL_NETSERVER_SERVICEMODULEMNAGER_H
