//
// Created by zkl on 5/2/18.
//

#include "ZServer.h"

using namespace zkl_server;

pthread_mutex_t serverMutex = PTHREAD_MUTEX_INITIALIZER;

bool serverStopFlag = false;

bool ZServer::serverStop() {
    return true;
}

int ZServer::createTcpServer() {
    // 准备SOCKET
    // 使用的socket是IPV4标准的TCP连接
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        logger.E("socket failed with message : " + STRERR);
        return -1;
    }

    // 绑定
    struct sockaddr_in addr;
    // 设定绑定的端口
    addr.sin_port = htons(static_cast<uint16_t>(this->port));
    // 设定绑定的网络标准
    addr.sin_family = AF_INET;
    // 设定绑定的地址
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    int bindFlag = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    if (bindFlag != 0) {

        logger.E("bind failed with message : " + STRERR);
        return -1;
    }

    // TODO 需要在JSON配置中增加配置最大的连接数的功能
    int listenFlag = listen(sock, 500);
    if (listenFlag == -1) {
        logger.E("listen failed with message : " + STRERR);
        return -1;
    }

    return sock;
}

/**
 * 监听文件描述符,这里的监听模式默认是EPOLLIN
 * @param epollFd
 * @param fd 需要监听的文件描述符
 * @param event 监听模式
 * @return
 */
bool ZServer::epollAdd(int epollFd, int fd, uint32_t event) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = event;
    int addFlag = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
    if (addFlag == -1) {
        logger.E("epoll add error message : " + STRERR);
        return false;
    }
    return true;
}

bool ZServer::epollRmv(int epollFd, int fd) {
    struct epoll_event ev;
    // 这里的ev是可以为NULL的,但是如果内核版本在2.6.9以下可能存在问题,所有这里放置一个
    int delFlag = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ev);
    if (delFlag == -1) {
        logger.E("epoll remove error message : " + STRERR);
        return false;
    }
    return true;
}

void sigServerStop(int signal) {
    if (signal == SIGINT) {
        serverStopFlag = true;
    }
    if (signal == SIGPIPE) {
        // do nothing
    }
}

int (*decodeFunction)(string &content, string &cmd, void **obj, string &errorMsg, bool &accessFlag, const char *buf);

//int (*decodeFunction)(DecodeModuleInfo *decodeModuleInfo, const char *buf);

int ZServer::withDecode(int fd ,DecodeModuleInfo & moduleInfo) {
    // 获得字符串信息
    // 当前假定没有大文件需要传输
    char buf[10240];
    memset(buf, 0, sizeof(buf));
    ssize_t len = recv(fd, buf, sizeof(buf), 0);

    logger.D("recv data , len : " + len);
    if (len < 0) {
        logger.E("recv error with message : " + STRERR);
    }

    // 获得之前在配置文件中配置的解码库,调用函数进行处理
    for (LoadLibInfo *loadLibInfo : decodeLib) {
        LibInfo *libInfo = loadLibInfo->getLibInfo();
        void *dl = dlopen(libInfo->getLibPath().c_str(), RTLD_NOW);
        if (dl == nullptr) {
            logger.E("decode dlopen lib error : " + libInfo->getLibPath() + "  with name : " + libInfo->getLibName());
            continue;
        }
        if (libInfo->getWorkFunction().length() > 0) {
            decodeFunction = (int (*)(string &, string &, void **, string &, bool &, const char *)) dlsym(dl,
                                                                                                          libInfo->getWorkFunction().c_str());
            if (decodeFunction) {
                string content;
                string cmd;
                void *obj = nullptr;
                string errorMsg;
                bool accessFlag = false;

                int flag = decodeFunction(content, cmd, &obj, errorMsg, accessFlag, buf);
                if (flag < 0) {
                    logger.D("decode module callback error with flag : " + to_string(flag));
                    continue;
                }

                moduleInfo.setInfo(content, cmd, obj, errorMsg, accessFlag);
                cout << "access flag is : " + to_string(accessFlag) << endl;
            } else {
                logger.E("decode dlsym error with check function, lib name : " + libInfo->getWorkFunction() + " " +
                         string(dlerror()));
                continue;
            }
        }
    }

    return 0;
}

int ZServer::checkCmd(DecodeModuleInfo & decodeModule) {
    const string &cmd = decodeModule.getCmd();
    if(cmd.length() == 0){
        return UNKNOWN_CMD;
    }
    if (cmd.compare("") == 0) {
        return CONNET_END;
    } else if (cmd.compare("") == 0) {
        return ERROR_CONNET;
    } else if (cmd.compare("") == 0) {
        return ADMIN_CONNET;
    }
    return UNKNOWN_CMD;
}

int (*percolatorFunction)(string &content, void **obj, string &errorMsg, bool &accessFlag,
                          const string &decodeContent, const void *decodeObj);

int ZServer::withPercolator(DecodeModuleInfo & decodeModule , PercolatorModuleInfo & percolatorModuleInfo) {
    // TODO 为了支持多个过滤器,这里的逻辑需要修改
    for (LoadLibInfo *loadLibInfo : percolatorLib) {
        LibInfo *libInfo = loadLibInfo->getLibInfo();
        void *dl = dlopen(libInfo->getLibPath().c_str(), RTLD_NOW);
        if (dl == nullptr) {
            logger.E("percolator dlopen lib error : " + libInfo->getLibPath() + "  with name : " +
                     libInfo->getLibName());
            continue;
        }
        if (libInfo->getInitFunction().length() > 0) {
            percolatorFunction = (int (*)(string &, void **, string &, bool &,
                                          const string &, const void *)) dlsym(dl, libInfo->getWorkFunction().c_str());
            if (percolatorFunction) {
                // TODO 为了能够照顾多个过滤器使用,这里或许可以改成具体的数据从过滤器中获得
                string content;
                void *obj = nullptr;
                string errorMsg;
                bool accessFlag = false;

                int flag = percolatorFunction(content, &obj, errorMsg, accessFlag, decodeModule.getContent(),
                                              decodeModule.getObj());
                if (flag < 0) {
                    logger.D("percolator module callback error with flag : " + to_string(flag));
                    continue;
                }

                percolatorModuleInfo.setData(content, obj, errorMsg, accessFlag);

            } else {
                logger.E("percolator dlsym error with check function, lib name : " + libInfo->getWorkFunction() + " " +
                         string(dlerror()));
                continue;
            }
        }
    }
    return 0;
}

int (*serviceFunction)(string &content, void **obj, string &errorMsg, bool &accessFlag,
                       const string &percolatorContent, const void *percolatorObj,
                       const string &decodeContent, const void *decodeObj);

int ZServer::withServer(DecodeModuleInfo & decodeModule, PercolatorModuleInfo & percolatorModule, ServiceModuleInfo & serviceModuleInfo) {

    for (LoadLibInfo *loadLibInfo : percolatorLib) {
        LibInfo *libInfo = loadLibInfo->getLibInfo();
        void *dl = dlopen(libInfo->getLibPath().c_str(), RTLD_NOW);
        if (dl == nullptr) {
            logger.E("service dlopen lib error : " + libInfo->getLibPath() + "  with name : " +
                     libInfo->getLibName());
            continue;
        }
        if (libInfo->getInitFunction().length() > 0) {
            serviceFunction = (int (*)(string &content, void **obj, string &errorMsg, bool &accessFlag,
                                       const string &percolatorContent, const void *percolatorObj,
                                       const string &decodeContent, const void *decodeObj)) dlsym(dl,
                                                                                                  libInfo->getWorkFunction().c_str());
            if (serviceFunction) {

                string content;
                void *obj = nullptr;
                string errorMsg;
                bool accessFlag = false;

                int flag = serviceFunction(content, &obj, errorMsg, accessFlag,
                                           percolatorModule.getContent(), percolatorModule.getObj(),
                                           decodeModule.getContent(), decodeModule.getObj());
                if (flag < 0) {
                    logger.D("service module callback error with flag : " + to_string(flag));
                    continue;
                }

                serviceModuleInfo.setData(content, obj, errorMsg, accessFlag);

            } else {
                logger.E("service dlsym error with check function, lib name : " + libInfo->getWorkFunction() + " " +
                         string(dlerror()));
                continue;
            }
        }
    }

    return 0;
}

void ZServer::sendServerSuccess(int fd, ServiceModuleInfo serviceModuleInfo) {
    sendServer(fd, move(serviceModuleInfo), 1);
}

void ZServer::sendServerError(int fd, ServiceModuleInfo serviceModuleInfo) {
    sendServer(fd, move(serviceModuleInfo), 2);
}

void ZServer::sendServer(int fd, ServiceModuleInfo serviceModuleInfo, int type) {
    EncodeModuleInfo moduleInfo;
    if (type == 1) {
        moduleInfo = move(withEncode(serviceModuleInfo.getContent() , serviceModuleInfo.getObj(),true));
    } else if (type == 2) {
        moduleInfo = move(withEncode(serviceModuleInfo.getErrorMsg(), serviceModuleInfo.getObj(),false));
    }
    sendStr(fd , moduleInfo);
}

void ZServer::sendStr(int fd,EncodeModuleInfo & moduleInfo){
    cout << "sendStr " << fd << endl;
    pthread_mutex_lock(&serverMutex);
    cout << "sendStr in lock " << fd << endl;
    ssize_t sendFlag = send(fd, moduleInfo.getContent().c_str(), moduleInfo.getContent().length(), 0);
    if(sendFlag == -1){
        logger.E("send error with message : " + STRERR);
    }
//    if(moduleInfo.isDisConnect()){
//        epollRmv(epollFd , fd);
        close(fd);
//    }

    cout << "send length : " << sendFlag << endl;
    pthread_mutex_unlock(&serverMutex);
    cout << "sendStr unlock " << fd << endl;
}

int (*encodeFunction)(string &content, void **obj, string &errorMsg, bool &accessFlag, bool & disConnet, const string &sendMessage,
                      const void *sendObj, bool successFlag);

EncodeModuleInfo ZServer::withEncode(const string &sendMessage, const void *sendObj, bool successFlag) {

    EncodeModuleInfo encodeModuleInfo;

    LibInfo *libInfo  = encodeLib.back()->getLibInfo();

    void *dl = dlopen(libInfo->getLibPath().c_str(), RTLD_NOW);
    if (dl == nullptr) {
        logger.E("encode dlopen lib error : " + libInfo->getLibPath() + "  with name : " +
                 libInfo->getLibName());
        // TODO 如果处理出错,需要返回什么?
        return encodeModuleInfo;
    }
    if (libInfo->getInitFunction().length() > 0) {
        encodeFunction = (int (*)(string &, void **, string &, bool & , bool &, const string &, const void *,bool )) dlsym(dl,
                                                                                                      libInfo->getWorkFunction().c_str());
        if (encodeFunction) {

            string content;
            void *obj = nullptr;
            string errorMsg;
            bool accessFlag = false;
            bool disConnect = false;

            int flag = encodeFunction(content, &obj, errorMsg, accessFlag, disConnect, sendMessage, sendObj, successFlag);

            if (flag < 0) {
                logger.D("service module callback error with flag : " + to_string(flag));
            }else{
                cout << "encode content : " << content << endl;
                encodeModuleInfo.setData(content, obj, errorMsg, accessFlag, disConnect);
            }
        } else {
            logger.E("encode dlsym error with check function, lib name : " + libInfo->getWorkFunction() + " " +
                     string(dlerror()));
        }
    }
    return encodeModuleInfo;
}

namespace zkl_server {

    int doServer(ThreadJob *job) {

        auto *server = (ZServer *) job->getJob();
        int fd = job->getType();

        cout << "do server fd : " << fd << endl;

        // 普通流程
        // 调用解码模块
        // 这里面会调用recv函数获得数据
        DecodeModuleInfo decodeModule;
        server->withDecode(fd , decodeModule);
        // 查看是否是命令
        int flag = server->checkCmd(decodeModule);
        if (flag == server->CONNET_END) {
            server->logger.D("check cmd : connet_end");
            server->epollRmv(server->epollFd, fd);
            close(fd);
            return 0;

        } else if (flag == server->ERROR_CONNET) {
            server->logger.D("check cmd : error_connet");
            // 这个连接是错误的,不需要在进行处理
            return 0;

        } else if (flag == server->UNKNOWN_CMD) {
            server->logger.D("check cmd : unknown_cmd");
            // 未知命令,就认为这个请求是有问题的,不做任何处理
            // TODO 暂时注释
            // return 0;
        }

        if (!decodeModule.isAccessFlag()) {
            cout << "access denied" << endl;

            // 不允许连接,需要返回拒绝命令,同时断开连接
            server->sendWithAccessDenied(fd, &decodeModule);
            server->epollRmv(server->epollFd, fd);
            close(fd);
            return 0;
        }

        cout << "decode over" << endl;

        // 调用过滤器模块
        PercolatorModuleInfo percolatorModule ;
        server->withPercolator(decodeModule , percolatorModule);

        // 如果是管理员请求,不进行过滤器过滤,直接通过
        if (flag != server->ADMIN_CONNET) {
            if (!percolatorModule.isAccessFlag()) {
                // 不允许连接,需要返回拒绝命令,同时断开连接
                server->sendWithAccessDenied(fd, &percolatorModule);
                server->epollRmv(server->epollFd, fd);
                close(fd);
                return 0;
            }
        }

        // 调用业务模块,注意:业务处理中是否通过这个标志必须设置为true
        ServiceModuleInfo serviceModuleInfo;
        server->withServer(decodeModule, percolatorModule, serviceModuleInfo);

        if (serviceModuleInfo.isAccessFlag()) {
            // 编码准备发回
            server->sendServerSuccess(fd, serviceModuleInfo);
        } else {
            // 其他情况下,需要返回另外的信息
            server->sendServerError(fd, serviceModuleInfo);
        }
        cout << "work part over" << endl;
        return 0;
    }
}

void ZServer::sendWithAccessDenied(int fd, ModuleInfo *moduleInfo) {

    EncodeModuleInfo encodeModuleInfo = move(withEncode(moduleInfo->getErrorMsg() , nullptr , false));
    sendStr(fd , encodeModuleInfo);
}

bool ZServer::serverStart() {

    if (!initFlag) {
        // 如果之前读取配置文件时出现错误,这里需要停止
        logger.E("inint server error , can not start");
        return false;
    }

    // 启动服务器监听
    // 每一个新进入的请求,首先会经过一个过滤器,判断是否有效,如果有效,进入循环
    // 进入循环以后,每一次的数据进入,进行一次解码,解码之后的结果如果是命令,则执行命令,如果是业务操作,执行业务操作
    logger.D("server will start");

    signal(SIGINT, sigServerStop);
    signal(SIGPIPE, sigServerStop);

    // 一个标准的服务器启动过程 EPOLL+SERVER
    int sock = createTcpServer();
    if (sock == -1) {
        return false;
    }

    epollFd = epoll_create(1024);
    if (epollFd == -1) {
        return false;
    }

    // 添加服务器文件描述符到EPOLL中
    bool addFlag = epollAdd(epollFd, sock);
    if (!addFlag) {
        close(sock);
        return false;
    }

    // 准备线程池
    ZThreadPool &pool = ZThreadPool::getInstance(50, &logger);

    pool.startPool();

    cout << "epoll ready" << endl;

    for (;;) {
        struct epoll_event evs[80];
        int ret = epoll_wait(epollFd, evs, 80, 5000);
        if (serverStopFlag) {
            logger.D("get signal to exit");
            break;
        }
        if (ret < 0) {
            logger.E("epoll error message : " + STRERR);
            continue;
        }
        if (ret == 0) {
            continue;
        }

        for (int x = 0; x < ret; ++x) {
            int fd = evs[x].data.fd;
            if (fd == sock) {
                // TODO 这里应该记录所有的来访的数据的IP地址等多种信息,保存到数据库中,这里偷懒没有做
                int incomingFd = accept(sock, nullptr, nullptr);
                if (incomingFd == -1) {
                    logger.E("accept error with message : " + STRERR);
                    continue;
                }
                // 可能有人连接服务器了
                bool addFlag = epollAdd(epollFd, incomingFd);
                if (!addFlag) {
                    logger.E("epoll add error : " + STRERR);
                    continue;
                }
                ++currentConn;
                logger.D("current conn is : " + to_string(currentConn));
            } else {
                cout << "new come " << fd << endl;
                ThreadJob *threadJob = new ThreadJob((void *) this, fd, doServer);
                pool.setJob(threadJob);
            }
        }
    }

    close(sock);

    return true;
}

ZServer::ZServer(string configPath) {

    if (configPath.length() == 0) {
        logger.E("config path is null");
        initFlag = false;
        return;
    }
    bool configFlag = initConfig(configPath);
    if (!configFlag) {
        // 配置文件初始化出现问题
        logger.E("config init error");
        initFlag = false;
        return;
    }
    // 虽然不知道有没有用,但是会尝试调用每个动态库中设定的初始化函数,或许可以用来初始化数据库的链接操作吧
    bool flag = initLibFunc();
    if (initFlag) {
        initFlag = flag;
    }
}

void (*initFunctionForServere)();

bool ZServer::callLibInitFlag(LoadLibInfo *loadLibInfo) {
    bool flag = true;
    // 打开库文件
    // 到了这里保证所有的都是库都可以找到的文件
    LibInfo *libInfo = loadLibInfo->getLibInfo();
    void *dl = dlopen(libInfo->getLibPath().c_str(), RTLD_NOW);
    if (dl == nullptr) {
        logger.E("dlopen lib error : " + libInfo->getLibPath() + "  with name : " + libInfo->getLibName());
        flag = false;
    }
    loadLibInfo->setHander(dl);
    if (libInfo->getInitFunction().length() > 0) {
        // 打开初始化函数
        initFunctionForServere = (void (*)()) dlsym(dl, libInfo->getInitFunction().c_str());
        if (initFunctionForServere) {
            initFunctionForServere();
            logger.D("calling function : " + libInfo->getInitFunction() + " with module name : " +
                     libInfo->getLibName());
        } else {
            logger.E("dlsym error with check function, lib name : " + libInfo->getInitFunction() + " " +
                     string(dlerror()));
            flag = false;
        }
    }
    return flag;
}

bool ZServer::initLibFunc() {
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

bool ZServer::initConfig(string configPath) {

    struct stat statBuf;
    int flag = stat(configPath.c_str(), &statBuf);
    if (flag == -1) {
        logger.E("get config file stat error with flag : " + flag);
        return false;
    }
    if (!S_ISREG(statBuf.st_mode)) {
        logger.E(configPath + " file type is not a file");
        return false;
    }
    long fileSize = statBuf.st_size + 1;

    FILE *file = fopen(configPath.c_str(), "r");
    if (file == nullptr) {
        logger.E("open file error");
        return false;
    }
    char *jsonStr = (char *) malloc(static_cast<size_t>(fileSize));
    memset(jsonStr, 0, fileSize);

    size_t readSize = fread(jsonStr, 1, statBuf.st_size, file);
    if (readSize <= 0) {
        logger.E("read file error with read size : " + readSize);
        return false;
    }

    fclose(file);

    bool parseFlag = parseJson(jsonStr);
    if (!parseFlag) {
        logger.E("parse json error");
        return false;
    }

    return true;
}

bool ZServer::parseJson(const char *json) {
    // 该函数应该被放在一个类中进行解析
    cJSON *cjson = cJSON_Parse(json);
    this->port = cJSON_GetObjectItem(cjson, "port")->valueint;
    if (this->port <= 0) {
        logger.E("port set error , use default port 8899");
        // TODO 这里的具体数字应该被替换成全局变量
        this->port = 8899;
    }
    string logPath = cJSON_GetObjectItem(cjson, "logPath")->valuestring;
    if (logPath.length() <= 0) {
        logger.E("log path error , use default file ./zserver.log");
        logPath = "./zserver.log";
    }
    logger.setPath(logPath);
    cJSON *modules = cJSON_GetObjectItem(cjson, "modules");
    if (modules == nullptr) {
        logger.E("get modules size error");
        return false;
    }
    int moduleSize = cJSON_GetArraySize(modules);
    logger.D("module size is " + to_string(moduleSize));
    // TODO 现阶段权重不适用
    logger.D("==============ignore weight================");

    for (int x = 0; x < moduleSize; ++x) {
        cJSON *module = cJSON_GetArrayItem(modules, x);
        if (module != nullptr) {
            char *libName = cJSON_GetObjectItem(module, "libName")->valuestring;
            char *libPath = cJSON_GetObjectItem(module, "libPath")->valuestring;
            int libType = cJSON_GetObjectItem(module, "libType")->valueint;
            char *initFunction = cJSON_GetObjectItem(module, "initFunction")->valuestring;
            char *workFunction = cJSON_GetObjectItem(module, "workFunction")->valuestring;

            // 检查库的路径以及文件类型是否符合要求
            struct stat libStat;
            int statFlag = stat(libPath, &libStat);
            if (statFlag != 0 || !S_ISREG(libStat.st_mode)) {
                logger.E("lib stat check error with name : " + string(libName) + "  with path : " + string(libPath));
                continue;
            }

            if (strlen(initFunction) == 0 || strlen(workFunction) == 0) {
                logger.E("lib initFunction or workFunction length is 0");
                continue;
            }

            LibInfo *libInfo = new LibInfo(libName, libPath, libType, 0, initFunction, workFunction);
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
                serverLib.push_back(loadLibInfo);
            }
            logger.D("get module : " + string(libName));
        }
    }

    if (decodeLib.size() == 0 || encodeLib.size() == 0 || percolatorLib.size() == 0 || serverLib.size() == 0) {
        cout << decodeLib.size() << endl;
        logger.E("lib size error decodeLib size: " + to_string(decodeLib.size()));
        logger.E("lib size error encodeLib size: " + to_string(encodeLib.size()));
        logger.E("lib size error percolatorLib size: " + to_string(percolatorLib.size()));
        logger.E("lib size error serverLib size: " + to_string(serverLib.size()));
        return false;
    }
    return true;
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
