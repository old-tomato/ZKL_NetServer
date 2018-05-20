//
// Created by zkl on 18-5-6.
//

#ifndef ZKL_NETSERVER_ZTHREADPOOL_H
#define ZKL_NETSERVER_ZTHREADPOOL_H

#include <pthread.h>
#include <queue>
#include <list>
#include <atomic>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include "./Logger.h"
#include "../Dao/ThreadJob.h"

using namespace std;

namespace zkl_server{

    /**
     * 一个简单的线程池,用于处理任务
     */

    class ZThreadPool {

        friend void * threadFunc(void * arg);

    private:
        static int threadCount;
        atomic_int jobCount;

        static ZThreadPool * pool;

        static Logger * logger;

        bool stopFlag = false;

        bool startFlag = false;

        list<pthread_t> threadList;

        queue<ThreadJob *>  * jobQueue = nullptr;

        ZThreadPool(){};

    public:

        static ZThreadPool & getInstance(int threadCount , Logger * log);

        bool startPool();

        bool stopPool();

        bool setJob(ThreadJob * job);

    };
}

#endif //ZKL_NETSERVER_ZTHREADPOOL_H
