//
// Created by zkl on 5/2/18.
//

#ifndef Z_SERVER_LIBINFO_H
#define Z_SERVER_LIBINFO_H

#include <string>

using namespace std;

namespace zkl_server {

    // 存放模块数据
    class LibInfo {

    private:
        // 库的名称,必填
        string libName;
        // 库的路径,必填
        string libPath;
        // 库的类型，如果类型是-1存在问题,必填
        int libType = -1;
        // 权重,当多个模块相同时,使用JSON文件中的顺序作为标准,上限为INT类型
        int weight = 0;
        // 初始化函数名称,这个函数会在初始化的时候被调用,传入和传出都是void类型,这个字段允许为空
        string initFunction = "";
        // 业务函数,调用业务时会调用这个函数
        string workFunction = "";
        // 路由名称,现在只允许在server模块中存在
        string route = "";

    public:
        // 解码模块
        static int DECODE_MODULE;
        // 编码模块
        static int ENCODE_MODULE;
        // 过滤器模块
        static int PERCOLATOR_MODULE;
        // 业务模块
        static int SERVER_MODULE;

        LibInfo(string libname, string libPath, int libType, int weight, string initFunction, string workFunction) :
                libName(libname), libPath(libPath) , libType(libType) , weight(weight),initFunction(initFunction),workFunction(workFunction){};

        const string &getLibName() const {
                return libName;
        }

        const string &getLibPath() const {
                return libPath;
        }

        int getLibType() const {
                return libType;
        }

        int getWeight() const {
                return weight;
        }

        const string &getInitFunction() const {
                return initFunction;
        }

        const string &getWorkFunction() const {
            return workFunction;
        }

        const string &getRoute() const{
            return this->route;
        }

        void setRoute(const string &route){
            this->route = route;
        }
    };

}

#endif //Z_SERVER_LIBINFO_H
