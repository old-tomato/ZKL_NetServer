//
// Created by zkl on 18-5-19.
//

#ifndef ZKL_NETSERVER_MODULEINFO_H
#define ZKL_NETSERVER_MODULEINFO_H

#include <string>
#include "ThreadJob.h"

using namespace std;

namespace zkl_server {

    class ModuleInfo{
        friend int doServer(ThreadJob *job);
    protected:
        // 是否允许通过
        bool accessFlag = false;
        // 解码后数据
        string errorMsg = "";
    public:
        virtual bool isAccessFlag() const {
            return accessFlag;
        }

        virtual const string &getErrorMsg() const {
            return errorMsg;
        }
    };

}

#endif //ZKL_NETSERVER_MODULEINFO_H
