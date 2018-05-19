//
// Created by zkl on 18-5-19.
//

#ifndef ZKL_NETSERVER_DECODEMODULEINFO_H
#define ZKL_NETSERVER_DECODEMODULEINFO_H

#include <string>
#include "ModuleInfo.h"

using namespace std;

namespace zkl_server {

    class DecodeModuleInfo : public ModuleInfo{
    private:
        // 解码后数据
        const string content = "";
        // 命令数据
        const string cmd = "";
        // 对象地址
        const void * obj = nullptr;
        const int type = LibInfo::DECODE_MODULE;

    public:

        const string &getContent() const {
            return content;
        }

        const string &getCmd() const {
            return cmd;
        }

        const void *getObj() const {
            return obj;
        }

        const int getType() const {
            return type;
        }
    };

}

#endif //ZKL_NETSERVER_DECODEMODULEINFO_H
