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
#include <fcntl.h>
#include "Dao/LibInfo.h"
#include "Utils/Logger.h"
#include "Dao/DecodeModuleInfo.h"
#include "Dao/EncodeModuleInfo.h"
#include "Dao/PercolatorModuleInfo.h"
#include "Dao/ServiceModuleInfo.h"
#include "Utils/ZThreadPool.h"
#include "Manager/ConfigManager.h"

using namespace std;

namespace zkl_server {

#define STRERR string(strerror(errno))

    class ZServer {

        friend int doServer(ThreadJob *job);

    private:
        enum {NORMAL_MESSAGE , ERROR_MASSAGE};

        int epollFd = -1;

        int currentConn = 0;

        // 初始化表示,读取基本配置是否出错,如果出错就不会启动服务器
        bool initFlag = true;

        Logger &logger = Logger::getInstance();

        ConfigManager * configManager = nullptr;

        /**
         * 创建服务器
         * @return
         */
        int createTcpServer();

        bool epollAdd(int epollFd, int fd, uint32_t event = EPOLLIN);

        bool epollRmv(int epollFd, int fd);

        void sendWithAccessDenied(int fd , ModuleInfo * moduleInfo);

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

        void sendStr(int fd,EncodeModuleInfo & moduleInfo);
    public:
        virtual ~ZServer();

        ZServer(string configPath);

        bool serverStart();

        bool serverStop();
    };
}

#endif //Z_SERVER_ZSERVER_H
