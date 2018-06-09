//
// Created by zkl on 18-6-9.
//

#include "PercolatorModuleManager.h"

using namespace zkl_server;

PercolatorModuleManager::PercolatorModuleManager(zkl_server::Logger *logger,
                                                             const list<zkl_server::LibInfo *> &moduleList)
        : ModuleManager(logger, moduleList) {}

int (*percolatorFunction)(string &content, void **obj, string &errorMsg, bool &accessFlag,
                          const string &decodeContent, const void *decodeObj);

void PercolatorModuleManager::withPercolator(DecodeModuleInfo &decodeModule, PercolatorModuleInfo & moduleInfo){

    void * obj = nullptr;

    for(LibInfo * info : moduleList){
        if(!info->getWorkFunction().empty()){
            DL_HANDLER dl = getLibHander(info);
            percolatorFunction = (int (*)(string &, void **, string &, bool &,const string &, const void *))
            (dlsym(dl , info->getWorkFunction().c_str()));
            if(percolatorFunction != nullptr){

                string content;
                string errorMsg;
                bool accessFlag = false;

                int flag = percolatorFunction(content, &obj, errorMsg, accessFlag, decodeModule.getContent(),
                                              decodeModule.getObj());
                if (flag < 0) {
                    logger->D("decode module callback error with flag : " + to_string(flag));
                    continue;
                }

                moduleInfo.setData(content, obj, errorMsg, accessFlag);
                cout << "access flag is : " + to_string(accessFlag) << endl;

            }else{
                logger->E("module init function is null : " + info->getLibName() + "  function name is : " + info->getInitFunction());
            }
        }else{
            logger->E("get init function error with : " + info->getLibName());
        }
    }
}

PercolatorModuleManager::~PercolatorModuleManager(){
    // TODO 这里需要处理
}
