#include"logger.h"
#include"time.h"
#include<iostream>

// 获取日志单例
Logger& Logger::getInstance(){
    static Logger obj;
    return obj;
}

// 设置日志级别
void Logger::setLogLevel(LogLevel level){
    m_loglevel = level;
}

// 写日志
void Logger::Log(std::string msg){
    m_lockqueue.Push(msg);
}

Logger::Logger(){
    // 子线程 -- 从队列中取出日志写入磁盘
    std::thread writeLogTask([&](){
        while(true){
            // 获取当天的信息
            time_t now = time(nullptr);
            tm* nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            FILE* pf = fopen(file_name,"a+"); // 追加的方式打开
            if(pf == nullptr){
                std::cout << "logger file:" << file_name << "open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lockqueue.Pop();
            char time_buf[128] = {0};
            sprintf(time_buf,"%d:%d:%d=>[%s]",
                    nowtm->tm_hour,
                    nowtm->tm_min,
                    nowtm->tm_sec,
                    (m_loglevel == INFO ? "info" : "error"));

            msg.insert(0,time_buf);
            msg.append("\n");

            fputs(msg.c_str(),pf);
            fclose(pf);
        }
    });

    // 设置线程分离
    writeLogTask.detach();
}
