//
// Created by zkl on 18-5-19.
//

#ifndef ZKL_NETSERVER_ENCODEMODULEINFO_H
#define ZKL_NETSERVER_ENCODEMODULEINFO_H

#include <string>
#include "ModuleInfo.h"

using namespace std;

namespace zkl_server {

    class EncodeModuleInfo : public ModuleInfo{
    private:
        const void * obj = nullptr;
        string content = "";
        int type = LibInfo::ENCODE_MODULE;
    public:
        const void *getObj() const {
            return obj;
        }

        void setObj(const void *obj) {
            EncodeModuleInfo::obj = obj;
        }

        const string &getContent() const {
            return content;
        }

        void setContent(const string &content) {
            EncodeModuleInfo::content = content;
        }

        const int getType() const {
            return type;
        }
    };

}

#endif //ZKL_NETSERVER_ENCODEMODULEINFO_H
