#pragma once
#include"lockqueue.h"
#include<string>

// 定义宏  LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger& logger = Logger::getInstance(); \
        logger.setLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...)\
    do \
    {  \
        Logger& logger = Logger::getInstance(); \
        logger.setLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \


enum LogLevel{
    INFO,  // 普通信息
    ERROR  // 错误信息
};

// 日志系统
class Logger{
public:
    // 获取日志单例
    static Logger& getInstance();
    // 设置日志级别
    void setLogLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    int m_loglevel;
    LockQueue<std::string> m_lockqueue;
};




