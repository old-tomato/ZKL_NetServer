//
// Created by zkl on 18-5-19.
//

#ifndef ZKL_NETSERVER_DECODEMODULEINFO_H
#define ZKL_NETSERVER_DECODEMODULEINFO_H

#include <string>
#include "ModuleInfo.h"
#include "LibInfo.h"

using namespace std;

namespace zkl_server {

    class DecodeModuleInfo : public ModuleInfo{
    private:
        // 解码后数据
        string content = "";
        // 命令数据
        int cmd = 0;
        // 对象地址
        void * obj = nullptr;
        // 路由设定,默认情况下是所有都匹配,/就是通配符
        string route = "/";
        int type = LibInfo::DECODE_MODULE;

    public:

        const string &getContent() const {
            return content;
        }

        const int getCmd() const {
            return cmd;
        }

        const void *getObj() const {
            return obj;
        }

        const int getType() const {
            return type;
        }

        void setCmd(int cmd){
            this->cmd = cmd;
        }

        void setInfo(string & content , int cmd , void * obj, string & errorMsg , bool accessFlag){
            this->content = content;
            this->cmd = cmd;
            this->obj = obj;
            this->errorMsg = errorMsg;
            this->accessFlag = accessFlag;
        }

        void setCmdAccessFlag(int cmd , bool accessFlag){
            this->cmd = cmd;
            this->accessFlag = accessFlag;
        }

        const string getRoute(){
            return route;
        }

    };

}

#endif //ZKL_NETSERVER_DECODEMODULEINFO_H
