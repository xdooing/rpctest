#include"rpctestconfig.h"
#include<iostream>
#include<fcntl.h>
#include<unistd.h>

// 解析配置文件
void RpctestConfig::LoadConfigFile(const char* confile_file){
    FILE *pf = fopen(confile_file,"r");
    if(pf == nullptr){
        std::cout << confile_file << " is not exits!" << std::endl;
        exit(EXIT_FAILURE);
    }
    /*
    # rpc节点的ip地址
    rpcserverip = 127.0.0.1
    # rpc节点的port端口号
    rpcserverport = 8000
    */
   while(!feof(pf)){
    char buf[512] = {0};
    fgets(buf,512,pf);

    // 去掉前后的空格
    std::string read_buf(buf);
    read_buf = Trim(read_buf);
    if(read_buf[0] == '#' || read_buf.empty()){
        continue;
    }
    // 解析配置
    int index = read_buf.find('=');
    if(index == -1){
        continue;
    }
    std::string key;
    std::string value;
    key = Trim(read_buf.substr(0,index));
    value = Trim(read_buf.substr(index + 1,read_buf.size() - index - 1));
    m_configMap.insert({key,value});
   }
}

// 查询配置项信息
std::string RpctestConfig::Load(const std::string& key){
    // 安全期间，不要用[key]的方式访问value
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()){
        return std::string();
    }
    return it->second;
}

// 去除前后端空格函数
std::string RpctestConfig::Trim(std::string src_buf){
    int index = src_buf.find_first_not_of(' ');
    if(index != -1){
        // 前面有空格
        src_buf = src_buf.substr(index,src_buf.size() - index);
    }
    index = src_buf.find_last_not_of(' ');
    if(index != -1){
        src_buf = src_buf.substr(0,index + 1);
    }
    index = src_buf.find('\n');
    if(index != -1){
        src_buf = src_buf.substr(0,index);
    }
    return src_buf;
}