//
// Created by zkl on 18-6-9.
//

#include "ServiceModuleManager.h"

using namespace zkl_server;

ServiceModuleManager::ServiceModuleManager(Logger *logger,
                                                     const list<LibInfo *> &moduleList) : ModuleManager(
        logger, moduleList) {}

int (*serviceFunction)(string &content, void **obj, string &errorMsg, bool &accessFlag,
                       const string &percolatorContent, const void *percolatorObj,
                       const string &decodeContent, const void *decodeObj);

void ServiceModuleManager::withServer(DecodeModuleInfo &decodeModule,
                                                PercolatorModuleInfo &percolatorModule,
                                                ServiceModuleInfo &serviceModuleInfo) {
    void * obj = nullptr;

    for(LibInfo * info : moduleList){
        if (info->getWorkFunction().length() > 0 && (info->getRoute().empty() || // 如果没有配置路由,进入
            info->getRoute().compare(decodeModule.getRoute()))) { // 如果路由匹配上了,进入
            cout << "server in" << endl;

            DL_HANDLER dl = getLibHander(info);
            serviceFunction = (int (*)(string &, void **, string &, bool &,
                                       const string &, const void *,
                                       const string &, const void *)) dlsym(dl,info->getWorkFunction().c_str());
            if (serviceFunction) {
                string content;
                string errorMsg;
                bool accessFlag = false;

                int flag = serviceFunction(content, &obj, errorMsg, accessFlag,
                                           percolatorModule.getContent(), percolatorModule.getObj(),
                                           decodeModule.getContent(), decodeModule.getObj());
                if (flag < 0) {
                    logger->D("service module callback error with flag : " + to_string(flag));
                    continue;
                }

                serviceModuleInfo.setData(content, obj, errorMsg, accessFlag);

            } else {
                logger->E("service dlsym error with check function, lib name : " + info->getWorkFunction() + " " +
                         string(dlerror()));
                continue;
            }
        }
    }

}
