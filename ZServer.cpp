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

    // 设定为非阻塞模式,防止accept出阻塞
    fcntl(sock , F_SETFL , O_NONBLOCK);

    // 绑定
    struct sockaddr_in addr;
    // 设定绑定的端口
    addr.sin_port = htons(static_cast<uint16_t>(configManager->getPort()));
    // 设定绑定的网络标准
    addr.sin_family = AF_INET;
    // 设定绑定的地址
    addr.sin_addr.s_addr = inet_addr(configManager->getServerIp().c_str());

    int bindFlag = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    if (bindFlag != 0) {

        logger.E("bind failed with message : " + STRERR);
        return -1;
    }

    int listenFlag = listen(sock, this->configManager->getMaxListen());
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

void ZServer::sendServerSuccess(int fd, ServiceModuleInfo serviceModuleInfo) {
    sendServer(fd, move(serviceModuleInfo), NORMAL_MESSAGE);
}

void ZServer::sendServerError(int fd, ServiceModuleInfo serviceModuleInfo) {
    sendServer(fd, move(serviceModuleInfo), ERROR_MASSAGE);
}

void ZServer::sendServer(int fd, ServiceModuleInfo serviceModuleInfo, int type) {
    EncodeModuleInfo moduleInfo;
    if (type == NORMAL_MESSAGE) {
        configManager->getEncodeManager()->withEncode(serviceModuleInfo.getContent(), serviceModuleInfo.getObj(), true, moduleInfo);
    } else if (type == ERROR_MASSAGE) {
        configManager->getEncodeManager()->withEncode(serviceModuleInfo.getErrorMsg(), serviceModuleInfo.getObj(), false, moduleInfo);
    }
    sendStr(fd, moduleInfo);
}

void ZServer::sendStr(int fd, EncodeModuleInfo &moduleInfo) {
    cout << "sendStr " << fd << endl;
    pthread_mutex_lock(&serverMutex);
    cout << "sendStr in lock " << fd << endl;
    ssize_t sendFlag = send(fd, moduleInfo.getContent().c_str(), moduleInfo.getContent().length(), 0);
    if (sendFlag == -1) {
        logger.E("send error with message : " + STRERR);
    }

    close(fd);

    cout << "send length : " << sendFlag << endl;
    pthread_mutex_unlock(&serverMutex);
    cout << "sendStr unlock " << fd << endl;
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
        server->configManager->getDecodeModuleManager()->withDecode(fd, decodeModule);
        // 查看是否是命令
        if (decodeModule.getCmd() == ModuleInfo::CONNET_END) {
            server->logger.D("check cmd : connet_end");
            server->epollRmv(server->epollFd, fd);
            close(fd);
            return 0;

        } else if (decodeModule.getCmd() == ModuleInfo::ERROR_CONNET) {
            server->logger.D("check cmd : error_connet");
            // 这个连接是错误的,不需要在进行处理
            return 0;

        } else if (decodeModule.getCmd() == ModuleInfo::UNKNOWN_CMD) {
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
        PercolatorModuleInfo percolatorModule;
        server->configManager->getPercolatorManager()->withPercolator(decodeModule, percolatorModule);

        // 如果是管理员请求,不进行过滤器过滤,直接通过
        if (decodeModule.getCmd() != ModuleInfo::ADMIN_CONNET) {
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
        server->configManager->getServiceManager()->withServer(decodeModule, percolatorModule, serviceModuleInfo);

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
    EncodeModuleInfo encodeModuleInfo;
    configManager->getEncodeManager()->withEncode(moduleInfo->getErrorMsg(), nullptr, false ,encodeModuleInfo);
    sendStr(fd, encodeModuleInfo);
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

    // 这个值已经没有更多的意义了,理论上只需要大于0就可以了,但是为了一定的兼容性,这里还是赋值1024
    epollFd = epoll_create(1024);
    if (epollFd == -1) {
        return false;
    }

    // 添加服务器文件描述符到EPOLL中
    // 监听服务器的请求的时候,需要用到水平触发
    bool addFlag = epollAdd(epollFd, sock);
    if (!addFlag) {
        close(sock);
        return false;
    }

    // 准备线程池
    ZThreadPool &pool = ZThreadPool::getInstance(configManager->getThreadCount(), &logger);

    pool.startPool();

    cout << "epoll ready" << endl;

    struct epoll_event evs[configManager->getEpollMaxEvents()];

    for (;;) {
        int ret = epoll_wait(epollFd, evs, configManager->getEpollMaxEvents(), configManager->getEpollTimeOut());
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
                // 有人连接服务器的时候,需要使用边缘触发
                bool addFlag = epollAdd(epollFd, incomingFd, EPOLLET | EPOLLIN);
                if (!addFlag) {
                    logger.E("epoll add error : " + STRERR);
                    continue;
                }
                ++currentConn;
            } else {

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

    configManager = ConfigManager::getInstance();

    bool configFlag = configManager->loadConfig(configPath);
    if (!configFlag) {
        // 配置文件初始化出现问题
        logger.E("config init error");
        initFlag = false;
        return;
    }
    // 虽然不知道有没有用,但是会尝试调用每个动态库中设定的初始化函数,或许可以用来初始化数据库的链接操作吧
    bool flag = configManager->initLibFunction();
    if (initFlag) {
        initFlag = flag;
    }
}

ZServer::~ZServer() {

}
