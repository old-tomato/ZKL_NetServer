//
// Created by zkl on 18-6-9.
//

#include "ModuleManager.h"

using namespace zkl_server;

ModuleManager::ModuleManager(Logger * logger , const list<LibInfo *> &moduleList){
    if(logger != nullptr){
        this->logger = logger;
    }else{
        cout << "logger is null " << endl;
        exit(-1);
    }
    if(!moduleList.empty()){
        this->moduleList = moduleList;
        this->moduleCount = static_cast<int>(moduleList.size());
    }
}

void (*initFunctionForService)();

bool ModuleManager::doInitFunction(){

    for(LibInfo * info : moduleList){
        if(!info->getInitFunction().empty()){
            DL_HANDLER dl = dlopen(info->getLibPath().c_str() , RTLD_NOW);
            initFunctionForService = (void (*)() )(dlsym(dl , info->getInitFunction().c_str()));
            if(initFunctionForService != nullptr){
                initFunctionForService();
            }else{
                logger->E("module init function is null : " + info->getLibName() + "  function name is : " + info->getInitFunction());
                return false;
            }
        }else{
            logger->E("get init function error with : " + info->getLibName());
            return false;
        }
    }
    return true;
}

DL_HANDLER ModuleManager::getLibHander(LibInfo * info){
    return dlopen(info->getLibPath().c_str() , RTLD_NOW);;
}