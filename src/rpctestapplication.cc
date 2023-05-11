#include"rpctestapplication.h"
#include<iostream>
#include<unistd.h>

RpctestConfig RpctestApplication::m_config;


// 错误提示
void showArgsHelp(){
    std::cout << "format : command -i <configfile>" << std::endl;
}

//初始化 provider -i config.conf
void RpctestApplication::Init(int argc,char** argv){
    if(argc < 2){
        showArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while((c = getopt(argc,argv,"i:")) != -1){
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            showArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            std::cout << "need <configfile>" << std::endl;
            showArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 加载配置文件
    m_config.LoadConfigFile(config_file.c_str());
}

RpctestApplication& RpctestApplication::GetInstance(){
    static RpctestApplication obj;
    return obj;
}

RpctestConfig& RpctestApplication::GetConfig(){
    return m_config;
}