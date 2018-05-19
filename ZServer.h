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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "Dao/LibInfo.h"
#include "Utils/Logger.h"
#include "Dao/DecodeModuleInfo.h"
#include "Dao/EncodeModuleInfo.h"
#include "Dao/PercolatorModuleInfo.h"
#include "Dao/ServiceModuleInfo.h"
#include "Utils/ZThreadPool.h"

using namespace std;

namespace zkl_server {

    class ZServer {

        friend int doServer(ThreadJob *job);

    private:

        class LoadLibInfo {
        private:
            LibInfo *libInfo;
            void *hander = nullptr;
        public:
            LoadLibInfo(LibInfo *libInfo, void *hander);

            void setHander(void *hander);

            LibInfo *getLibInfo() const;

            void *getHander() const;
        };

        enum {
            CONNET_END, ERROR_CONNET, ADMIN_CONNET, UNKNOWN_CMD
        };

        int port;
        int epollFd = -1;
        list<LoadLibInfo *> decodeLib;
        list<LoadLibInfo *> encodeLib;
        list<LoadLibInfo *> percolatorLib;
        list<LoadLibInfo *> serverLib;

        int currentConn = 0;

        Logger &logger = Logger::getInstance();

        bool initConfig(string configPath);

        bool parseJson(const char *json);

        /**
         * 调用各个模块的初始化函数信息
         */
        void initLibFunc();

        /**
         * 创建服务器
         * @return
         */
        int createTcpServer();

        bool epollAdd(int epollFd, int fd, uint32_t event = EPOLLIN);

        bool epollRmv(int epollFd, int fd);

        void sendWithAccessDenied(int fd , ModuleInfo * moduleInfo);

        /**
         * 进行解码操作
         * @param fd
         * @return
         */
        DecodeModuleInfo withDecode(int fd);

        /**
         * 查看是否存在任务模块
         * @param decodeModule
         * @return
         */
        int checkCmd(DecodeModuleInfo decodeModule);

        /**
         * 进行过滤操作
         * @param decodeModule
         * @return
         */
        PercolatorModuleInfo withPercolator(DecodeModuleInfo decodeModule);

        /**
         * 处理具体的业务
         * @param decodeModule
         * @param percolatorModule
         * @return
         */
        ServiceModuleInfo withServer(DecodeModuleInfo decodeModule , PercolatorModuleInfo percolatorModule);

        /**
         * 当业务成功处理,并且需要返回时
         * @param serviceModuleInfo
         */
        void sendServerSuccess(int fd , ServiceModuleInfo serviceModuleInfo);

        /**
         * 当业务处理失败,并需要返回时
         * @param serviceModuleInfo
         */
        void sendServerError(int fd , ServiceModuleInfo serviceModuleInfo);

        void sendServer(int fd , ServiceModuleInfo serviceModuleInfo , int type);

        /**
         * 将编码模块中的数据变成字符串
         * @param encodeModuleInfo
         * @return
         */
        string withEncode(EncodeModuleInfo encodeModuleInfo);

    public:
        virtual ~ZServer();

        ZServer(string configPath);

        bool serverStart();

        bool serverStop();

    };
}

#endif //Z_SERVER_ZSERVER_H
