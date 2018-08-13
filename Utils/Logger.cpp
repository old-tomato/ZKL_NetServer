//
// Created by zkl on 18-5-2.
//

#include "Logger.h"

using namespace zkl_server;

Logger * zkl_server::Logger::logger = nullptr;

Logger & Logger::getInstance() {

    if(logger == nullptr){
        logger = new Logger();
    }

    return *logger;
}

Logger::Logger(){
    ef = fopen("./error.log" , "a+");
    if(ef == nullptr){
        ef = fopen("./error.log" , "w+");
        if(ef == nullptr){
            cout << "logger init error" << endl;
            return;
        }
    }
}

Logger::~Logger() {
    // 需要在这里关闭之前打开的文件描述符
    if(this->file != nullptr){
        fclose(this->file);
    }
    if(this->ef != nullptr){
        fclose(ef);
    }
}

void Logger::crushWrite(string message){
    string privateLine = createLine(CRUSH , message , __FILE__ , __LINE__);
    writeLog(CRUSH , privateLine);
}

void Logger::setPath(string path) {
    this->logDirPath = path;
}

bool Logger::checkFile() {
    return file != nullptr;
}

bool Logger::clearFile() {
    return this->clearFlag;
}


void Logger::debug(string message, string file, int line) {
    string privateLine = createLine(DEBUG , message , file , line);
    writeLog(DEBUG , privateLine);
}

void Logger::info(string message , string file , int line) {
    string privateLine = createLine(INFO , message , file , line);
    writeLog(INFO , privateLine);
}

void Logger::warn(string message , string file , int line) {
    string privateLine = createLine(WARING , message , file , line);
    writeLog(WARING , privateLine);
}

void Logger::error(string message , string file , int line) {
    string privateLine = createLine(ERROR , message , file , line);
    writeLog(ERROR , privateLine);
}

void Logger::setDebugFlag(bool debugFlag) {
    this->debugFlag = debugFlag;
}

void Logger::setClearFlag(bool clearFlag) {
    if(this->clearFlag != clearFlag){
        if(file != nullptr){
            fclose(this->file);
        }
        if(clearFlag){
            file = fopen(logDirPath.c_str() , "w+");
        }else{
            file = fopen(logDirPath.c_str() , "a");
        }
    }
    bool checkFlag = checkFile();
    if(!checkFlag){

    }
    this->clearFlag = clearFlag;
}

string Logger::createLine(int level , string message , string file , int line){
    string levelFlag = "";
    switch (level){
        case DEBUG:
            levelFlag = "[DEBUG]";
            break;
        case INFO:
            levelFlag = "[INFO]";
            break;
        case WARING:
            levelFlag = "[WARING]";
            break;
        case ERROR:
            levelFlag = "[ERROR]";
            break;
        case CRUSH:
            levelFlag = "[CRUSH]";
            break;
        default:
            break;
    }
    string _line = levelFlag + " " + __DATE__ + " " + __TIME__ + " [" + file + " " + to_string(line) + "]" + " " + message + "\n";
    if(debugFlag){
        cout << _line << endl;
    }
    return _line;
}

void Logger::writeLog(int level , string message ){

    if(level == CRUSH){
        if(ef == nullptr){
            cout << "write crush error" << endl;
            return;
        }
        if(debugFlag){
            cout  << message << endl;
        }
        pthread_mutex_lock(&lock);
        fwrite(message.c_str() , 1 , message.length() , ef);
        fflush(ef);
        pthread_mutex_unlock(&lock);

    }else{
        changeFilePointer();
        if(!checkFile()){
            crushWrite("crush write : " + message);
            return;
        }
        pthread_mutex_lock(&lock);
        fwrite(message.c_str() , 1 , message.length() , file);
        fflush(file);
        pthread_mutex_unlock(&lock);
    }
}

string Logger::createFileNameWithTime(){
    time_t t;
    time(&t);
    tm * tmPoint = localtime(&t);
    int currentYear = 1900 + tmPoint->tm_year;
    int currentMonth = tmPoint->tm_mon;
    int currentDay = tmPoint->tm_mday;
    return "Log-" + to_string(currentYear) + "-" + to_string(currentMonth) + "-" + to_string(currentDay) + ".log";
}

bool Logger::changeFilePointer(){
    if(this->file == nullptr){
        fileFullName = this->logDirPath + createFileNameWithTime();
        // 打开文件
        FILE * f = fopen(fileFullName.c_str() , "a+");
        if(f != nullptr && clearFlag){
            fclose(f);
            f = nullptr;
        }
        if(f == nullptr){
            f = fopen(fileFullName.c_str() , "w+");
        }
        if(f != nullptr){
            this->file = f;
        }
    }else{
        if(lastFileFullName.length() == 0){
            lastFileFullName = fileFullName;
        }else if(lastFileFullName != fileFullName){
            // 如果之前写日志的文件和现在不同,需要关闭当前的文件描述符,重新创建新的文件
            if(this->file != nullptr){
                fclose(this->file);
            }
            FILE * f = fopen(this->fileFullName.c_str() , "a+");
            if(f != nullptr && clearFlag){
                fclose(f);
                f = nullptr;
            }
            if(f == nullptr){
                f = fopen(fileFullName.c_str() , "w+");
            }
            if(f != nullptr){
                this->file = f;
            }
        }
    }

    if(this->file == nullptr){
        cout << "open log file error : " << this->fileFullName << endl;
        return false;
    }

    return true;
}
