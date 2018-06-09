//
// Created by zkl on 18-5-27.
//

#include "ConfigManager.h"

using namespace zkl_server;

ConfigManager *ConfigManager::manager = nullptr;
Logger *ConfigManager::logger = nullptr;

void (*initFunctionForServere)();

ConfigManager *ConfigManager::getInstance() {
    logger = &Logger::getInstance();
    if (manager == nullptr) {
        manager = new ConfigManager;
    }
    return manager;
}

bool ConfigManager::loadConfigToBuf(string configPath){
    //检查路径的可用性
    struct stat statBuf;
    int flag = stat(configPath.c_str(), &statBuf);
    if (flag == -1) {
        logger->E("get config file stat error with flag : " + flag);
        return false;
    }
    if (!S_ISREG(statBuf.st_mode)) {
        logger->E(configPath + " file type is not a file");
        return false;
    }
    long fileSize = statBuf.st_size + 1;

    FILE *file = fopen(configPath.c_str(), "r");
    if (file == nullptr) {
        logger->E("open file error");
        return false;
    }
    // 文件一次性全部读取到内存中
    configBuf = new char[fileSize];
    if (configBuf == nullptr) {
        logger->E("new config buffer error !!!!!!");
        return false;
    }
    memset(configBuf, 0, static_cast<size_t>(fileSize));
    size_t readSize = fread(configBuf, 1, static_cast<size_t>(statBuf.st_size), file);
    if (readSize <= 0) {
        logger->E("read file error with read size : " + readSize);
        delete configBuf;
        configBuf = nullptr;
        return false;
    }

    fclose(file);
    return true;
}

bool ConfigManager::loadConfig(string configPath) {
    if (configPath.length() == 0) {
        logger->E("config file path is null");
        return false;
    }
    bool loadToBufFlag = loadConfigToBuf(configPath);
    if(!loadToBufFlag){
        logger->E("config load to buffer error");
        return false;
    }
    bool parseFlag = parseConfig();
    if (!parseFlag) {
        logger->E("parse config error");
        return false;
    }
    bool sortFlag = sortLib();
    if (!sortFlag) {
        logger->E("sort module error");
        return false;
    }
    return true;
}

/**
 * 统一解析json配置文件中的内容
 * @return
 */
bool ConfigManager::parseConfig() {
    cJSON *root = cJSON_Parse(configBuf);
    if (root == nullptr) {
        logger->E("parse config error at root");
        return false;
    }
    // 第一个先读取log的日志位置
    logPath = cJSON_GetObjectItem(root, "logPath")->valuestring;
    if (logPath.length() <= 0) {
        logger->E("log path error , use default file ./zserver.log");
        logPath = "./zserver.log";
    }
    logger->setPath(logPath);

    this->port = cJSON_GetObjectItem(root, "port")->valueint;
    if (this->port <= 0) {
        logger->E("port set error , use default port 8899");
        // TODO 这里的具体数字应该被替换成全局变量
        this->port = 8899;
    }

    this->serverIp = cJSON_GetObjectItem(root, "serverIp")->valuestring;
    if (this->serverIp.length() == 0) {
        this->serverIp = "127.0.0.1";
        logger->E("serverIp error , use default ip 127.0.0.1");
    }

    cJSON *maxListenObj = cJSON_GetObjectItem(root, "maxListen");
    if(maxListenObj != nullptr){
        this->maxListen = (maxListenObj->valueint > 0 ? maxListenObj->valueint : 10);
    }else{
        logger->E("get max listen error use default value 10");
    }

    cJSON *modules = cJSON_GetObjectItem(root, "modules");
    if (modules == nullptr) {
        logger->E("get modules size error");
        return false;
    }
    int moduleSize = cJSON_GetArraySize(modules);
    logger->D("module size is " + to_string(moduleSize));

    // 权重开始被使用
    for (int x = 0; x < moduleSize; ++x) {
        cJSON *module = cJSON_GetArrayItem(modules, x);
        if (module != nullptr) {
            char *libName = cJSON_GetObjectItem(module, "libName")->valuestring;
            char *libPath = cJSON_GetObjectItem(module, "libPath")->valuestring;
            int libType = cJSON_GetObjectItem(module, "libType")->valueint;
            char *initFunction = cJSON_GetObjectItem(module, "initFunction")->valuestring;
            char *workFunction = cJSON_GetObjectItem(module, "workFunction")->valuestring;

            int weight = 0;
            cJSON *weightObj = cJSON_GetObjectItem(module, "weight");
            if(weightObj != nullptr){
                weight = weightObj->valueint;
            }

            // 检查库的路径以及文件类型是否符合要求
            struct stat libStat;
            int statFlag = stat(libPath, &libStat);
            if (statFlag != 0 || !S_ISREG(libStat.st_mode)) {
                logger->E("lib stat check error with name : " + string(libName) + "  with path : " + string(libPath));
                continue;
            }

            if (strlen(initFunction) == 0 || strlen(workFunction) == 0) {
                logger->E("lib initFunction or workFunction length is 0");
                continue;
            }

            LibInfo *libInfo = new LibInfo(libName, libPath, libType, weight, initFunction, workFunction);

            // 现在handler没有起到任何作用
            LoadLibInfo *loadLibInfo = new LoadLibInfo(libInfo, nullptr);

            if (libType == LibInfo::DECODE_MODULE) {
                // 必须存在一个解码模块,可以空实现
                decodeLib.push_back(loadLibInfo);
            } else if (libType == LibInfo::ENCODE_MODULE) {
                // 必须存在一个编码模块.可以空实现
                encodeLib.push_back(loadLibInfo);
            } else if (libType == LibInfo::PERCOLATOR_MODULE) {
                // 必须存在一个过滤器模块,可以空实现
                percolatorLib.push_back(loadLibInfo);
            } else if (libType == LibInfo::SERVER_MODULE) {
                // 必须存在一个业务模块
                // 增加一个操作,读取路由
                cJSON *routeObj = cJSON_GetObjectItem(module, "route");
                string route = "";
                if(routeObj != nullptr){
                    route = routeObj->valuestring;
                }
                loadLibInfo->getLibInfo()->setRoute(route);
                serverLib.push_back(loadLibInfo);
            }
            logger->D("get module : " + string(libName));
        }
    }

    if (decodeLib.size() == 0 || encodeLib.size() == 0 || percolatorLib.size() == 0 || serverLib.size() == 0) {
        cout << decodeLib.size() << endl;
        logger->E("lib size error decodeLib size: " + to_string(decodeLib.size()));
        logger->E("lib size error encodeLib size: " + to_string(encodeLib.size()));
        logger->E("lib size error percolatorLib size: " + to_string(percolatorLib.size()));
        logger->E("lib size error serverLib size: " + to_string(serverLib.size()));
        return false;
    }
    return true;
}

bool ConfigManager::initLibFunction(){
    // 调用所有的初始化函数
    // TODO 这里的标识表示如果有一个函数的启动失败,就无法启动服务器,这个处理之后需要修改
    bool flag = true;
    for (LoadLibInfo *loadLibInfo : decodeLib) {
        bool callFlag = callLibInitFlag(loadLibInfo);
        if (!callFlag) {
            // 暂定虽然不会启动服务器,但是会检查所有配置的函数
            flag = callFlag;
            continue;
        }
    }
    for (LoadLibInfo *loadLibInfo : encodeLib) {
        bool callFlag = callLibInitFlag(loadLibInfo);
        if (!callFlag) {
            // 暂定虽然不会启动服务器,但是会检查所有配置的函数
            flag = callFlag;
            continue;
        }
    }
    for (LoadLibInfo *loadLibInfo : percolatorLib) {
        bool callFlag = callLibInitFlag(loadLibInfo);
        if (!callFlag) {
            // 暂定虽然不会启动服务器,但是会检查所有配置的函数
            flag = callFlag;
            continue;
        }
    }
    for (LoadLibInfo *loadLibInfo : serverLib) {
        bool callFlag = callLibInitFlag(loadLibInfo);
        if (!callFlag) {
            // 暂定虽然不会启动服务器,但是会检查所有配置的函数
            flag = callFlag;
            continue;
        }
    }
    return flag;
}

bool ConfigManager::callLibInitFlag(LoadLibInfo *loadLibInfo) {
    bool flag = true;
    // 打开库文件
    // 到了这里保证所有的都是库都可以找到的文件
    LibInfo *libInfo = loadLibInfo->getLibInfo();
    void *dl = dlopen(libInfo->getLibPath().c_str(), RTLD_NOW);
    if (dl == nullptr) {
        logger->E("dlopen lib error : " + libInfo->getLibPath() + "  with name : " + libInfo->getLibName());
        flag = false;
    }
    loadLibInfo->setHander(dl);
    if (libInfo->getInitFunction().length() > 0) {
        // 打开初始化函数
        initFunctionForServere = (void (*)()) dlsym(dl, libInfo->getInitFunction().c_str());
        if (initFunctionForServere) {
            initFunctionForServere();
            logger->D("calling function : " + libInfo->getInitFunction() + " with module name : " +
                     libInfo->getLibName());
        } else {
            logger->E("dlsym error with check function, lib name : " + libInfo->getInitFunction() + " " +
                     string(dlerror()));
            flag = false;
        }
    }
    return flag;
}

bool sortFunc(ConfigManager::LoadLibInfo * info1 , ConfigManager::LoadLibInfo * info2){
    return info1->getLibInfo()->getWeight() >= info2->getLibInfo()->getWeight();
}

bool ConfigManager::sortLib() {
    if (!decodeLib.empty()) {
        decodeLib.sort(sortFunc);
    }
    if (!encodeLib.empty()) {
        encodeLib.sort(sortFunc);
    }
    if (!percolatorLib.empty()) {
        percolatorLib.sort(sortFunc);
    }
    if (!serverLib.empty()) {
        serverLib.sort(sortFunc);
    }
    return true;
}

ConfigManager::~ConfigManager() {
    if (configBuf != nullptr) {
        delete configBuf;
    }
    if (!decodeLib.empty()) {
        for (LoadLibInfo *info : decodeLib) {
            delete info;
            info = nullptr;
        }
    }
    if (!encodeLib.empty()) {
        for (LoadLibInfo *info : encodeLib) {
            delete info;
            info = nullptr;
        }
    }
    if (!percolatorLib.empty()) {
        for (LoadLibInfo *info : percolatorLib) {
            delete info;
            info = nullptr;
        }
    }
    if (!serverLib.empty()) {
        for (LoadLibInfo *info : serverLib) {
            delete info;
            info = nullptr;
        }
    }
}

int ConfigManager::getPort() const {
    return port;
}

int ConfigManager::getMaxListen() const{
    return maxListen;
}

const list<ConfigManager::LoadLibInfo *> &ConfigManager::getDecodeLib() const {
    return decodeLib;
}

const list<ConfigManager::LoadLibInfo *> &ConfigManager::getEncodeLib() const {
    return encodeLib;
}

const list<ConfigManager::LoadLibInfo *> &ConfigManager::getPercolatorLib() const {
    return percolatorLib;
}

const list<ConfigManager::LoadLibInfo *> &ConfigManager::getServerLib() const {
    return serverLib;
}

const string &ConfigManager::getServerIp() const {
    return serverIp;
}

ConfigManager::LoadLibInfo::LoadLibInfo(LibInfo *libInfo, void *hander) : libInfo(libInfo), hander(hander) {}

LibInfo *ConfigManager::LoadLibInfo::getLibInfo() const {
    return libInfo;
}

void *ConfigManager::LoadLibInfo::getHander() const {
    return hander;
}

void ConfigManager::LoadLibInfo::setHander(void *hander) {
    LoadLibInfo::hander = hander;
}

ConfigManager::LoadLibInfo::~LoadLibInfo() {
    if (libInfo != nullptr) {
        delete libInfo;
        libInfo = nullptr;
    }
    if (hander != nullptr) {
        // TODO handler暂时没有被使用
    }
}


