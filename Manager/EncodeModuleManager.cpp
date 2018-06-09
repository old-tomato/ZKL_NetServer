//
// Created by zkl on 18-6-9.
//

#include "EncodeModuleManager.h"

using namespace zkl_server;

EncodeModuleManager::EncodeModuleManager(Logger *logger,
                                                     const list<LibInfo *> &moduleList) : ModuleManager(
        logger, moduleList) {}

int (*encodeFunction)(string &content, void **obj, string &errorMsg, bool &accessFlag, bool &disConnet,
                      const string &sendMessage,
                      const void *sendObj, bool successFlag);

void EncodeModuleManager::withEncode(const string &sendMessage, const void *sendObj, bool successFlag ,
                                     EncodeModuleInfo & moduleInfo){
    void * obj = nullptr;
    for(LibInfo * info : moduleList){
        if(!info->getWorkFunction().empty()){
            DL_HANDLER dl = getLibHander(info);
            encodeFunction = (int (*)(string &, void **, string &, bool &, bool &, const string &, const void *,bool))
            (dlsym(dl , info->getWorkFunction().c_str()));
            if(encodeFunction != nullptr){

                string content;
                string errorMsg;
                bool accessFlag = false;
                bool disConnect = false;

                int flag = encodeFunction(content, &obj, errorMsg, accessFlag, disConnect, sendMessage, sendObj,
                                          successFlag);

                if (flag < 0) {
                    logger->D("service module callback error with flag : " + to_string(flag));
                } else {
                    cout << "encode content : " << content << endl;
                    moduleInfo.setData(content, obj, errorMsg, accessFlag, disConnect);
                }
            }else{
                logger->E("module init function is null : " + info->getLibName() + "  function name is : " + info->getInitFunction());
            }
        }else{
            logger->E("get init function error with : " + info->getLibName());
        }
    }
}