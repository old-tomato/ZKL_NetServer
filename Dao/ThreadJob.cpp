//
// Created by zkl on 18-5-6.
//

#include "ThreadJob.h"

using namespace zkl_server;

void *ThreadJob::getJob() const {
    return job;
}

void ThreadJob::setJob(void *job) {
    ThreadJob::job = job;
}

int ThreadJob::getThreadId() const {
    return threadId;
}

void ThreadJob::setThreadId(int threadId){
    ThreadJob::threadId = threadId;
}

void ThreadJob::setType(int type){
    ThreadJob::type = type;
}

int ThreadJob::getType() const {
    return ThreadJob::type;
}

void * ThreadJob::getFunc() const{
    return (void *)func;
}

ThreadJob::ThreadJob(void *job, int type, int (* func)(ThreadJob *) ) : job(job), type(type),func(func) {}