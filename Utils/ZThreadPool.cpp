//
// Created by zkl on 18-5-6.
//

#include "ZThreadPool.h"

using namespace zkl_server;

ZThreadPool * ZThreadPool::pool = nullptr;

int ZThreadPool::threadCount = 10;
Logger * ZThreadPool::logger = nullptr;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


namespace zkl_server{
    void * threadFunc(void * arg){
        ZThreadPool * pool = (ZThreadPool *)arg;
        // 轮训查看队列中是否有内容需要操作
        for(;;){
            ThreadJob * job = nullptr;
            // 进入线程所状态,等待条件锁放开
            pthread_mutex_lock(&mutex);
            pthread_cond_wait(&cond , &mutex);
            if(pool->jobQueue->size() > 0){
                job = pool->jobQueue->front();
                pool->jobQueue->pop();
                -- pool->jobCount;
                cout << "job count " << pool->jobCount << "  queue size : " << pool->jobQueue->size() << endl;
            }
            pthread_mutex_unlock(&mutex);
            if(pool->stopFlag){
                break;
            }
            if(job == nullptr){
                break;
            }
            job->setThreadId(static_cast<int>(pthread_self()));
            int (*func)(ThreadJob * ) = (int (*)(ThreadJob * ))job->getFunc();
            int flag = func(job);
            if(flag < 0){
                pool->setJob(job);
            }else{
                delete job;
            }
            cout << "pool server over" << endl;
            if(pool->jobQueue->size() > 0){
                pthread_cond_signal(&cond);
            }
            cout << "pool server end" << endl;
        }
    }
}


ZThreadPool & ZThreadPool::getInstance(int count , Logger * log){
    if(pool != nullptr){
        return *pool;
    }
    pool = new ZThreadPool();
    if(count > 0){
        threadCount = count;
    }
    logger = log;
    return *pool;
}

//void timerHandler(int signo)
//{
//    cout << "timer" << endl;
//    if(signo == SIGALRM){
//        pthread_cond_signal(&cond);
//    }
//}

bool ZThreadPool::startPool(){
    jobCount = 0;
    startFlag = true;

//    signal(SIGALRM, timerHandler);
//
//    struct itimerval timer;
//    timer.it_value.tv_sec = 1;
//    timer.it_value.tv_usec = 0;
//    timer.it_interval.tv_sec = 1;
//    timer.it_interval.tv_usec = 0;
//
//    cout << setitimer(ITIMER_REAL , &timer , nullptr) << errno <<  endl;

    for(int x = 0 ; x < threadCount ; ++ x){
        pthread_t tid;
        int createFlag = pthread_create( &tid , nullptr , threadFunc , this);
        if(createFlag != 0){
            logger->E("create thread error");
        }else {
            threadList.push_back(tid);
        }
    }

    if(jobQueue == nullptr){
        jobQueue = new queue<ThreadJob *>;
    }

    if(!jobQueue->empty()){
        pthread_cond_signal(&cond);
        logger->D("set signal");
    }
}

bool ZThreadPool::stopPool(){
    stopFlag = true;
    int flag = pthread_cond_broadcast(&cond);

    for(pthread_t tid : threadList){
        // 阻塞在这里
        pthread_join(tid , nullptr);
    }

    return true;
}

bool ZThreadPool::setJob(ThreadJob * job){
    if(job == nullptr){
        logger->E("set job is null");
        return false;
    }

    if(jobQueue == nullptr){
        jobQueue = new queue<ThreadJob *>;
    }

    jobQueue->push(job);
    ++ jobCount;
    if(startFlag){
        pthread_cond_signal(&cond);
    }
    logger->D("set new job , current job count is " + to_string(jobCount));
    return true;
}