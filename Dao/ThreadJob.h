//
// Created by zkl on 18-5-6.
//

#ifndef ZKL_NETSERVER_THREADJOB_H
#define ZKL_NETSERVER_THREADJOB_H

namespace zkl_server {
    class ThreadJob {

    private:
        void *job = nullptr;
        int threadId = 0;
        int type = 0;
        int (*func)(ThreadJob * job) = nullptr;
    public:
        ThreadJob(void *job, int type , int (* func)(ThreadJob *) );

        void *getJob() const;

        void setJob(void *job);

        int getThreadId() const;

        void setThreadId(int threadId);

        void setType(int type);

        int getType() const;

        void * getFunc() const;

    };
}


#endif //ZKL_NETSERVER_THREADJOB_H
