#pragma once
#include<string>
#include<map>


class RpctestConfig{
public:
    // 解析配置文件
    void LoadConfigFile(const char* confile_file);
    // 查询配置项信息
    std::string Load(const std::string& key);

private:
    // 配置信息映射表
    std::map<std::string,std::string> m_configMap;
    // 去除前后端空格函数
    std::string Trim(std::string src_buf);
};