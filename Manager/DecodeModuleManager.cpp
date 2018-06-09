//
// Created by zkl on 18-6-9.
//

#include "DecodeModuleManager.h"

using namespace zkl_server;

#define STRERR string(strerror(errno))

DecodeModuleManager::DecodeModuleManager(Logger *logger, const list<LibInfo *> &moduleList) : ModuleManager(logger,
                                                                                                            moduleList) {}

int (*decodeFunction)(string &content, int & cmd, void **obj, string &errorMsg, bool &accessFlag, const char *buf);

void DecodeModuleManager::withDecode(int fd , DecodeModuleInfo & moduleInfo){
    if(fd < 0){
        logger->E("decode with decode fd error");
        moduleInfo.setCmdAccessFlag(ModuleInfo::CONNET_END , false);
        return;
    }

    char buf[10240];
    memset(buf, 0, sizeof(buf));
    ssize_t len = recv(fd, buf, sizeof(buf), 0);

    logger->D("recv data , len : " + len);
    if (len < 0) {
        logger->E("recv error with message : " + STRERR);
        moduleInfo.setCmdAccessFlag(ModuleInfo::CONNET_END , false);
        return;
    }

    void *obj = nullptr;

    for(LibInfo * info : moduleList){
        if(!info->getWorkFunction().empty()){
            DL_HANDLER dl = getLibHander(info);
            decodeFunction = (int (*)(string &, int &, void **, string &, bool &, const char *))
                                (dlsym(dl , info->getWorkFunction().c_str()));
            if(decodeFunction != nullptr){
                string content;
                int cmd;
                string errorMsg;
                bool accessFlag = false;
                int flag = decodeFunction(content, cmd, &obj, errorMsg, accessFlag, buf);
                if (flag < 0) {
                    logger->D("decode module callback error with flag : " + to_string(flag));
                    continue;
                }

                moduleInfo.setInfo(content, cmd, obj, errorMsg, accessFlag);
                cout << "access flag is : " + to_string(accessFlag) << endl;

            }else{
                logger->E("module init function is null : " + info->getLibName() + "  function name is : " + info->getInitFunction());
            }
        }else{
            logger->E("get init function error with : " + info->getLibName());
        }
    }

}

DecodeModuleManager::~DecodeModuleManager() {
    // TODO 需要实现
}
