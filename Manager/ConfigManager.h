//
// Created by zkl on 18-5-27.
//

#ifndef ZKL_NETSERVER_CONFIGMANAGER_H
#define ZKL_NETSERVER_CONFIGMANAGER_H

#include <list>
#include <string>
#include <map>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <string.h>
#include <dlfcn.h>
#include "../Utils/Logger.h"
#include "../Dao/LibInfo.h"
#include "DecodeModuleManager.h"
#include "PercolatorModuleManager.h"
#include "ServiceModuleManager.h"
#include "EncodeModuleManager.h"

using namespace std;

/**
 * 用于读取配置文件中的信息数据
 */
namespace zkl_server {
    // TODO 这个类本来应该是内部类的,但是现在由于友元函数的问题放在外面,这点需要调整
    class ConfigManager {
    private:

        enum {
            CONNET_END, ERROR_CONNET, ADMIN_CONNET, UNKNOWN_CMD
        };
        list<LibInfo *> decodeLib;
        list<LibInfo *> encodeLib;
        list<LibInfo *> percolatorLib;
        list<LibInfo *> serverLib;

        ConfigManager() = default;;

        static ConfigManager *manager;

        static Logger *logger;

        char *configBuf = nullptr;

        int port;

        string serverIp;

        string logPath;

        int threadCount = 10;
        int epollMaxEvents = 5;
        int epollTimeOut = 5000;

        DecodeModuleManager * decodeManager = nullptr;
        PercolatorModuleManager * percolatorManager = nullptr;
        ServiceModuleManager * serviceManager = nullptr;
        EncodeModuleManager * encodeManager = nullptr;

        /**
         * 服务器的最大的连接数
         */
        int maxListen = 10;

        /**
         *  读取配置文件内容到内存中
         */
        bool loadConfigToBuf(string configPath);

        /**
         * 具体的解析配置中的信息,使用的内容是之前设定的configBuf
         * @return
         */
        bool parseConfig();

        /**
         * 按照权重的位置排序当读取的LIB信息
         * @return
         */
        bool sortLib();

    public:

        friend bool sortFunc(LibInfo *info1, LibInfo *info2);

        static ConfigManager *getInstance();

        bool loadConfig(string configPath);

        /**
         * 如果被调用,会去调用所有的初始化函数,但是是否需要终止服务器由调用者决定
         * @return
         */
        bool initLibFunction();

        int getPort() const;

        int getMaxListen() const ;

        const string &getServerIp() const;

        DecodeModuleManager * getDecodeModuleManager();

        ~ConfigManager();

        DecodeModuleManager *getDecodeManager();

        PercolatorModuleManager *getPercolatorManager();

        ServiceModuleManager *getServiceManager();

        EncodeModuleManager *getEncodeManager();

        int getThreadCount() const;

        int getEpollMaxEvents() const;

        int getEpollTimeOut() const;
    };
}


#endif //ZKL_NETSERVER_CONFIGMANAGER_H
