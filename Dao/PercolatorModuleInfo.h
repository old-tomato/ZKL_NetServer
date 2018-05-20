//
// Created by zkl on 18-5-19.
//

#ifndef ZKL_NETSERVER_PERCOLATORMODULEINFO_H
#define ZKL_NETSERVER_PERCOLATORMODULEINFO_H

#include <string>
#include "ModuleInfo.h"
#include "LibInfo.h"

using namespace std;

namespace zkl_server {

    class PercolatorModuleInfo : public ModuleInfo{

    private:
        // 传递一些问题信息
        string content = "";

        // 这个指针的作用是为了传递任何类型的数据
        void * obj = nullptr;

        int type = LibInfo::PERCOLATOR_MODULE;

    public:
        const string &getContent() const {
            return content;
        }

        void *getObj() const {
            return obj;
        }

        const int getType() const {
            return type;
        }

        void setData(string & content , void * obj , string & errorMsg , bool accessFlag){
            this->content = content;
            this->obj = obj;
            this->errorMsg = errorMsg;
            this->accessFlag = accessFlag;
        }

    };

}

#endif //ZKL_NETSERVER_PERCOLATORMODULEINFO_H
