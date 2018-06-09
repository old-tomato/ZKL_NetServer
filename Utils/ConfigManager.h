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
#include "Logger.h"
#include "../Dao/LibInfo.h"

using namespace std;

/**
 * 用于读取配置文件中的信息数据
 */
namespace zkl_server {
    // TODO 这个类本来应该是内部类的,但是现在由于友元函数的问题放在外面,这点需要调整
    class ConfigManager {
    public:
        class LoadLibInfo {
        private:
            LibInfo *libInfo;
            void *hander = nullptr;
        public:
            LoadLibInfo(LibInfo *libInfo, void *hander);

            void setHander(void *hander);

            LibInfo *getLibInfo() const;

            void *getHander() const;

            ~LoadLibInfo();
        };

    private:

        enum {
            CONNET_END, ERROR_CONNET, ADMIN_CONNET, UNKNOWN_CMD
        };
        list<LoadLibInfo *> decodeLib;
        list<LoadLibInfo *> encodeLib;
        list<LoadLibInfo *> percolatorLib;
        list<LoadLibInfo *> serverLib;

        ConfigManager() = default;;

        static ConfigManager *manager;

        static Logger *logger;

        char *configBuf = nullptr;

        int port;

        string serverIp;

        string logPath;

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

        bool callLibInitFlag(LoadLibInfo *loadLibInfo);

    public:

        friend bool sortFunc(ConfigManager::LoadLibInfo *info1, ConfigManager::LoadLibInfo *info2);

        static ConfigManager *getInstance();

        bool loadConfig(string configPath);

        /**
         * 如果被调用,会去调用所有的初始化函数,但是是否需要终止服务器由调用者决定
         * @return
         */
        bool initLibFunction();

        int getPort() const;

        int getMaxListen() const ;

        const list<LoadLibInfo *> &getDecodeLib() const;

        const list<LoadLibInfo *> &getEncodeLib() const;

        const list<LoadLibInfo *> &getPercolatorLib() const;

        const list<LoadLibInfo *> &getServerLib() const;

        const string &getServerIp() const;

        ~ConfigManager();
    };
}


#endif //ZKL_NETSERVER_CONFIGMANAGER_H
