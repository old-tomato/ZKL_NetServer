//
// Created by zkl on 5/2/18.
//

#include "ZServer.h"

using namespace zkl_server;

bool ZServer::serverStop() {
    return false;
}

bool ZServer::serverStart() {

    return true;
}

ZServer::ZServer(string configPath) {

    if(configPath.length() == 0){
        logger.E("config path is null");
        return;
    }
    bool configFlag = initConfig(configPath);
    if(configFlag){
        // 配置文件初始化出现问题
        logger.E("config init error");
        return;
    }
    // 虽然不知道有没有用,但是会尝试调用每个动态库中设定的初始化函数,或许可以用来初始化数据库的链接操作吧
    initLibFunc();

}

void (*initFunctionForServere)();

void ZServer::initLibFunc(){

    for(LoadLibInfo * loadLibInfo : decodeLib){
        // 打开库文件
        // 到了这里保证所有的都是库都可以找到的文件
        LibInfo *libInfo = loadLibInfo->getLibInfo();
        void * dl = dlopen(libInfo->getLibPath().c_str() , RTLD_NOW);
        if(dl == nullptr){
            logger.E("dlopen lib error : " + libInfo->getLibPath() + "  with name : " + libInfo->getLibName());
            continue;
        }
        loadLibInfo->setHander(dl);
        if(libInfo->getInitFunction().length() > 0){
            // 打开初始化函数
            initFunctionForServere = (void (*)())dlsym(dl , libInfo->getInitFunction().c_str());
            if(initFunctionForServere){
                initFunctionForServere();
                logger.D("calling function : " + libInfo->getInitFunction() + " with module name : " + libInfo->getLibName());
            }else{
                logger.E("dlsym error with check function, lib name : " + libInfo->getInitFunction() + " " + string(dlerror()));
            }
        }
    }
}

bool ZServer::initConfig(string configPath){

    struct stat statBuf;
    int flag = stat(configPath.c_str() , &statBuf);
    if(flag == -1){
        logger.E("get config file stat error with flag : " + flag);
        return false;
    }
    if(!S_ISREG(statBuf.st_mode)){
        logger.E(configPath + " file type is not a file");
        return false;
    }
    long fileSize = statBuf.st_size + 1;

    FILE * file = fopen(configPath.c_str() , "r");
    if(file == nullptr){
        logger.E("open file error");
        return false;
    }
    char * jsonStr = (char *)malloc(static_cast<size_t>(fileSize));
    memset(jsonStr , 0 , fileSize);

    size_t readSize = fread(jsonStr , 1 , statBuf.st_size , file);
    if(readSize <= 0){
        logger.E("read file error with read size : " + readSize);
        return false;
    }

    fclose(file);

    bool parseFlag = parseJson(jsonStr);
    if(!parseFlag){
        logger.E("parse json error");
        return false;
    }

    return true;
}

bool ZServer::parseJson(const char * json){
    cJSON *cjson = cJSON_Parse(json);
    this->port = cJSON_GetObjectItem(cjson , "port")->valueint;
    if(this->port <= 0){
        logger.E("port set error , use default port 8899");
        this->port = 8899;
    }
    string logPath = cJSON_GetObjectItem(cjson , "logPath")->valuestring;
    if(logPath.length() <= 0){
        logger.E("log path error , use default file ./zserver.log");
        logPath = "./zserver.log";
    }
    logger.setPath(logPath);
    cJSON *modules = cJSON_GetObjectItem(cjson , "modules");
    if(modules == nullptr){
        logger.E("get modules size error");
        return false;
    }
    int moduleSize = cJSON_GetArraySize(modules);
    logger.D("module size is " + moduleSize);
    logger.D("==============ignore weight================");

    for(int x = 0 ; x < moduleSize ; ++ x){
        cJSON* module = cJSON_GetArrayItem(modules , x);
        if(module != nullptr){
            char * libName = cJSON_GetObjectItem(module , "libName")->valuestring;
            char * libPath = cJSON_GetObjectItem(module , "libPath")->valuestring;
            int libType = cJSON_GetObjectItem(module , "libType")->valueint;
            char * initFunction = cJSON_GetObjectItem(module , "initFunction")->valuestring;

            // 检查库的路径以及文件类型是否符合要求
            struct stat libStat;
            int statFlag = stat(libPath , &libStat);
            if(statFlag != 0 || !S_ISREG(libStat.st_mode)){
                logger.E("lib stat check error with name : " + string(libName) + "  with path : " + string(libPath));
                continue;
            }

            LibInfo * libInfo = new LibInfo(libName , libPath , libType , 0 , initFunction);
            LoadLibInfo * loadLibInfo = new LoadLibInfo(libInfo , nullptr);
            if(libType == LibInfo::DECODE_MODULE){
                // 必须存在一个解码模块,可以空实现
                decodeLib.push_back(loadLibInfo);
            }else if(libType == LibInfo::ENCODE_MODULE){
                // 必须存在一个编码模块.可以空实现
                encodeLib.push_back(loadLibInfo);
            }else if(libType == LibInfo::PERCOLATOR_MODULE){
                // 必须存在一个过滤器模块,可以空实现
                percolatorLib.push_back(loadLibInfo);
            }else if(libType == LibInfo::SERVER_MODULE){
                // 必须存在一个业务模块
                serverLib.push_back(loadLibInfo);
            }
            logger.D("get module : " + string(libName));
        }
    }

    if(decodeLib.size() == 0 || encodeLib.size() == 0 || percolatorLib.size() == 0 || serverLib.size() == 0){
        cout << decodeLib.size() << endl;
        logger.E("lib size error decodeLib size: " + to_string(decodeLib.size()));
        logger.E("lib size error encodeLib size: " + to_string(encodeLib.size()));
        logger.E("lib size error percolatorLib size: " + to_string(percolatorLib.size()));
        logger.E("lib size error serverLib size: " + to_string(serverLib.size()));
    }

}

ZServer::~ZServer() {

}

ZServer::LoadLibInfo::LoadLibInfo(LibInfo *libInfo, void *hander) : libInfo(libInfo), hander(hander) {}

LibInfo *ZServer::LoadLibInfo::getLibInfo() const {
    return libInfo;
}

void *ZServer::LoadLibInfo::getHander() const {
    return hander;
}

void ZServer::LoadLibInfo::setHander(void *hander) {
    LoadLibInfo::hander = hander;
}
