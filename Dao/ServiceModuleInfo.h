//
// Created by zkl on 18-5-19.
//

#ifndef ZKL_NETSERVER_SERVICEMODULEINFO_H
#define ZKL_NETSERVER_SERVICEMODULEINFO_H

#include <string>
#include "ModuleInfo.h"

using namespace std;

namespace zkl_server {

    class ServiceModuleInfo : public ModuleInfo{
    private:
        // 字符串信息
        string content = "";
        // 对象地址
        void * obj = nullptr;
        int type = LibInfo::SERVER_MODULE;

    public:
        const string &getContent() const {
            return content;
        }

        void setContent(const string &content) {
            ServiceModuleInfo::content = content;
        }

        void *getObj() const {
            return obj;
        }

        void setObj(void *obj) {
            ServiceModuleInfo::obj = obj;
        }

        const int getType() const {
            return type;
        }
    };

}

#endif //ZKL_NETSERVER_SERVICEMODULEINFO_H
