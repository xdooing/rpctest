#pragma once
#include"rpctestconfig.h"





// rpctest框架--负责框架的初始化
class RpctestApplication{
public:
    static void Init(int argc,char** argv);
    static RpctestApplication& GetInstance();
    static RpctestConfig& GetConfig();

private:
    // 配置信息
    static RpctestConfig m_config;

    RpctestApplication() = default;
    RpctestApplication(const RpctestApplication& obj) = delete;
    RpctestApplication& operator=(const RpctestApplication& obj) = delete;
};