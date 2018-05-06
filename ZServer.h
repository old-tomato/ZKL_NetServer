//
// Created by zkl on 5/2/18.
//

#ifndef Z_SERVER_ZSERVER_H
#define Z_SERVER_ZSERVER_H

#include <string>
#include <map>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <list>
#include <cjson/cJSON.h>
#include <dlfcn.h>
#include "Dao/LibInfo.h"
#include "Utils/Logger.h"

using namespace std;

namespace zkl_server{

    class ZServer {

    private:

        class LoadLibInfo{
        private:
            LibInfo * libInfo;
            void * hander = nullptr;
        public:
            LoadLibInfo(LibInfo *libInfo, void *hander);

            void setHander(void *hander);

            LibInfo *getLibInfo() const;

            void *getHander() const;
        };

        int port;
        list<LoadLibInfo *> decodeLib;
        list<LoadLibInfo *> encodeLib;
        list<LoadLibInfo *> percolatorLib;
        list<LoadLibInfo *> serverLib;

        Logger & logger = Logger::getInstance();

        bool initConfig(string configPath);

        bool parseJson(const char * json);

        /**
         * 调用各个模块的初始化函数信息
         */
        void initLibFunc();

    public:
        virtual ~ZServer();

        ZServer(string configPath);

        bool serverStart();

        bool serverStop();

    };
}

#endif //Z_SERVER_ZSERVER_H
