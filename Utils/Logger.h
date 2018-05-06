//
// Created by zkl on 18-5-2.
//

#ifndef Z_SERVER_LOGGER_H
#define Z_SERVER_LOGGER_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <pthread.h>

using namespace std;

namespace zkl_server{

#define D(message) debug(message , __FILE__ , __LINE__)
#define I(message) info(message , __FILE__ , __LINE__)
#define W(message) waring(message , __FILE__ , __LINE__)
#define E(message) error(message , __FILE__ , __LINE__)

    /**
     * 只创建一个文件
     * TODO 未来需要实现文件按照日期创建
     */
    class Logger {
    private:

        enum {DEBUG,INFO,WARING,ERROR,CRUSH};

        pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

        static Logger * logger;
        // 文件位置
        string logPath = "";
        // 清理标志,如果设定为true,将会在下一次写入的时候将上次写入的内容完全清空
        bool clearFlag = false;
        // 调试标志,如果打开,所有写在文件中的语句都会在控制台中打印,默认打开
        bool debugFlag = true;

        FILE* file = nullptr;

        // 如果正常的日志文件不能够写入,将会写入在这个特殊的文件里面
        FILE* ef = nullptr;

        Logger();

        bool checkFile();

        bool clearFile();

        void crushWrite(string message);

        void writeLog(int level , string message);

        string createLine(int level , string message , string file , int line);

    public:
        static Logger & getInstance();

        void setDebugFlag(bool debugFlag);

        void setClearFlag(bool clearFlag);

        void setPath(string path);

        void debug(string message , string file , int line);

        void info(string message , string file , int line);

        void warn(string message , string file , int line);

        void error(string message , string file , int line);

        ~Logger();
    };

}

#endif //Z_SERVER_LOGGER_H
