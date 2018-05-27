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
        string content = "";
        // 命令数据
        string cmd = "";
        // 对象地址
        void * obj = nullptr;
        // 路由设定,默认情况下是所有都匹配,/就是通配符
        string route = "/";
        int type = LibInfo::DECODE_MODULE;

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

        void setInfo(string & content , string & cmd , void * obj, string & errorMsg , bool accessFlag){
            this->content = content;
            this->cmd = cmd;
            this->obj = obj;
            this->errorMsg = errorMsg;
            this->accessFlag = accessFlag;
        }

        const string getRoute(){
            return route;
        }

    };

}

#endif //ZKL_NETSERVER_DECODEMODULEINFO_H
